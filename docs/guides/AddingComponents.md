# コンポーネントを追加する

新しいコンポーネントを作り、インスペクタで編集でき、シーンに保存され、毎フレーム更新されるようにするまでの手順です。エンジン側（`project/Engine/Scene/Component/`）とゲーム側（`project/Game/Components/`）で、登録する場所だけが違います。

仕組みの説明は [SceneLayer.md](../design/SceneLayer.md) にあります。

## 手順

### 1. 構造体を書く

コンポーネントは仮想関数を持たないデータだけの構造体です。毎フレーム動かしたい場合だけ、`Update` を書きます。

`project/Game/Components/RotatorComponent.h` が最小の例です。

```cpp
#pragma once
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Foundation/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
class Scene;
struct UpdateContext;
} // namespace Cake

struct RotatorComponent {
	// 各軸の回転速度（ラジアン/秒）.
	Cake::Vector3 speed = {0.0f, 1.0f, 0.0f};

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);
};

namespace Cake {
CAKE_REFLECT(RotatorComponent)
CAKE_PROPERTY(speed, "Speed")
CAKE_REFLECT_END()
} // namespace Cake
```

ゲーム側のコンポーネントはグローバル名前空間に置き、`CAKE_REFLECT` ブロックだけを `Cake` 名前空間の中に書きます。エンジン側のコンポーネントは構造体も `Cake` の中に置きます。

### 2. リフレクションを書く

構造体の直後に `CAKE_REFLECT` ブロックを書き、インスペクタに出したい・保存したいメンバを 1 行ずつ並べます。別ファイルに離すと、メンバを増やしたときに書き足し忘れるため、同じヘッダに置きます。

| マクロ | 用途 |
| --- | --- |
| `CAKE_PROPERTY(member, "Label")` | 型を自動判別して登録する。通常はこれを使う |
| `CAKE_PROPERTY_MIN(member, "Label", min)` | 下限だけを縛る |
| `CAKE_PROPERTY_MAX(member, "Label", max)` | 上限だけを縛る |
| `CAKE_PROPERTY_CLAMP(member, "Label", min, max)` | 両側を縛る Drag |
| `CAKE_PROPERTY_RANGE(member, "Label", min, max)` | Slider。範囲が狭く端から端まで動かしたい値に使う |
| `CAKE_PROPERTY_DRAG(member, "Label", min, max, speed)` | ドラッグ速度まで指定する。桁が大きい値に使う |
| `CAKE_PROPERTY_COLOR(member, "Label")` | `Vector4` をカラーピッカーとして扱う |

第 1 引数のメンバ名はシリアライズのキーになります。変更すると既存のシーンファイルが読めなくなります。第 2 引数の表示名はいつ変えても安全です。

使える型は `bool`、`int`、`float`、`std::string`、`Vector2/3/4`、`Transform`、`Gradient`、`GameObjectId`、`AssetRef<ModelHandle>`、`AssetRef<TextureHandle>`、`AssetRef<MaterialHandle>` です。対応していない型を渡すとコンパイルエラーになります。

### 3. 登録する

登録しないと、インスペクタに出ず、シーンにも保存されません。

| 置き場所 | 登録する関数 | ファイル |
| --- | --- | --- |
| エンジン側 | `RegisterAllComponents` | `project/Engine/Scene/Component/ComponentRegistration.cpp` |
| ゲーム側 | `Game::RegisterGameComponents` | `project/Game/GameModule.cpp` |

```cpp
registry.Register<RotatorComponent>(Cake::kDefaultUpdatePriority);
```

第 1 引数は `Update` の実行順で、小さいほど先に走ります。`kEarlyUpdatePriority`（-100）、`kDefaultUpdatePriority`（0）、`kLateUpdatePriority`（100）が目安です。`Update` を持たない型では意味を持たないため、省略できます。

### 4. Update を書く（動かす場合）

`Update` は `UpdateContext` と自分の `GameObjectId` を受け取ります。自分の Transform へは `ctx.scene.Find(self)` で辿ります。

| `UpdateContext` のメンバ | 内容 |
| --- | --- |
| `scene` | 現在のシーン。`Find` や `GetComponent` に使う |
| `input` | `InputManager`。キー判定は `ctx.GetKey`、`IsKeyHeld`、`IsKeyDown`、`IsKeyUp` の短縮形を使う |
| `time`、`sound` | `Time` と `SoundManager` |
| `deltaTime` | タイムスケール適用済みの経過時間 |
| `unscaledDeltaTime` | 一時停止中も進む経過時間 |

キー判定はゲーム用の経路を通るため、エディタで Game ウィンドウにフォーカスが無い間は自動的に `None` が返ります。

## 守ること

- `Update` の中でコンポーネントを追加・削除しない。プールの再確保で走査中の参照が無効になる。
- `Find` や `GetComponent` が返すポインタをフレームをまたいで持たない。持ち回るのは `GameObjectId`。
- `UpdateContext` をメンバに保持しない。そのフレームの `Update` の間だけ有効。
- 粒の配列のように増減するデータはコンポーネントに置かず、`Scene` 側に置く。
- 同じ型を 1 つの GameObject に 2 つ付けることはできない。`AddComponent` は既存のものを返す。

## 他のコンポーネントを必要とする場合

`CAKE_REFLECT_END()` の直後に `CAKE_REQUIRE_COMPONENTS` を書くと、インスペクタの Add Component で依存先も一緒に付き、依存先だけを削除できなくなります。

```cpp
CAKE_REFLECT(ParticleSystemComponent)
// ...
CAKE_REFLECT_END()
CAKE_REQUIRE_COMPONENTS(ParticleSystemComponent, ParticleRendererComponent)
```

この依存は `TypeRegistry::AddComponent` 経由の追加にだけ効きます。コードから `Scene::AddComponent` を直接呼ぶ場合は素通りするため、依存先を使う System は依存先が無くても落ちないように書きます。

## 新しい型のプロパティが必要な場合

`PropertyType` に無い型（列挙など）をインスペクタに出すには、次の 3 箇所を揃えて追加します。

1. `Foundation/Reflection/TypeInfo.h` の `PropertyType` と `PropertyTypeOf`
2. `Editor/Inspector/PropertyDrawer.cpp` の switch（描画）
3. `Scene/Serialize/SceneSerializer.cpp` の `WriteProperty` と `ReadProperty` の switch（保存と復元）

## 確認

- Debug 構成で起動し、Inspector の Add Component に型名が出る。
- 値を変えて Ctrl+S で保存し、`.scene` ファイルに `"type": "型名"` とプロパティが書かれている。
- Play を押すと `Update` が動き、Stop で元に戻る。
