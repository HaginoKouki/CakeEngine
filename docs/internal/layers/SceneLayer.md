# Scene 層

GameObject とコンポーネントを所有するシーン、その更新と保存、コンポーネントに対する処理（System）を担当する層です。

## この層が担当すること

Scene 層はゲーム世界の状態を持ち、毎フレーム更新します。「コンポーネントはデータだけを持ち、処理は System が行う」という方針で作られています。描画命令そのものは Render 層、アセットの実体は Asset 層が担当します。

## 配置

`project/Engine/Scene/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| 直下 | `Scene` | GameObject のプール、型ごとのコンポーネントプール、親子階層（`roots_`）、`ParticleStorage` を所有する |
| 直下 | `SceneManager` | 現在のシーンを 1 つ所有し、生成、読み込み、保存、毎フレームの更新を行う。編集中か実行中かは知らず、`deltaTime` が 0 ならコンポーネントを走らせない |
| `Object/` | `GameObject` | 識別子、名前、`TransformComponent`、`ComponentRef` の一覧、active フラグを持つ器 |
| `Object/` | `GameObjectId` | index と generation の識別子。シーン内でのみ一意 |
| `Object/` | `ComponentPool<T>`、`IComponentPool`、`ComponentRef`、`ComponentTypeId` | 型ごとの連続配置プール。持ち主の `GameObjectId` を並行配列で持つ |
| `Object/` | `ParticleStorage`、`EmitterState`、`Particle` | パーティクルの粒の状態と発生タイマー。エミッターごとに持つ |
| `Component/` | `TransformComponent` | 姿勢と親子関係。GameObject の組み込みフィールドで、プールには入らない |
| `Component/` | `CameraComponent`、`LightComponent`、`MeshRendererComponent`、`SpriteRendererComponent` | データのみの構造体 |
| `Component/Collider/` | `SphereColliderComponent`、`BoxColliderComponent` | 当たり判定の設定と、今フレームの衝突結果 |
| `Component/Particle/` | `ParticleSystemComponent`、`ParticleRendererComponent`、`ParticleForceFieldComponent` | パーティクルの出し方、描き方、Field |
| `Component/ComponentRegistry/` | `TypeRegistry`、`ComponentOps`、`ComponentTypeInfo` | 型名や型 ID から `TypeInfo` と操作関数（add、get、remove、copy、update）を引く登録簿 |
| `Component/ComponentRegistry/` | `RequireComponent.h`（`CAKE_REQUIRE_COMPONENTS`） | コンポーネント間の依存宣言 |
| `Component/` | `ComponentRegistration` | `RegisterAllComponents`。エンジン側コンポーネントを 1 行ずつ登録する |
| `System/` | `UpdateContext` | コンポーネントの `Update` に渡す、そのフレームの外部情報一式（scene、input、time、sound、deltaTime） |
| `System/` | `ComponentUpdateSystem`（`RunComponentUpdates`） | `Update` を持つ全コンポーネントを優先度順に呼ぶ |
| `System/` | `CameraSystem`（`CollectMainCamera`）、`LightSystem`（`CollectDirectionalLight`） | 有効なもののうち `priority` が最小の 1 つを採用する |
| `System/` | `GizmoSystem`（`CollectSceneGizmos`、`GizmoSettings`） | グリッド、コライダー、カメラの視錐台、ライト、Field、選択中の軸を `GizmoDrawList` へ積む |
| `System/` | `CloneSystem`（`DuplicateGameObject`） | GameObject を子孫ごと複製する |
| `System/Render/` | `RenderSystem`（`DrawMeshRenderers`、`ResolveSceneAssets`） | MeshRenderer の描画と、シーン読み込み直後のアセット参照の解決 |
| `System/Render/` | `SpriteSystem`（`CollectScreenSprites`） | 表示中の SpriteRenderer を `SpriteDrawList` へ積む。描画はしない |
| `System/Collision/` | `CollisionSystem`（`RunCollisionDetection`） | 球と箱の総当たり判定 |
| `System/Particle/` | `ParticleSystem`（`UpdateParticleSystems`） | 粒の生成、移動、消滅、Field の適用、色の計算 |
| `Serialize/` | `SceneSerializer` | シーンの JSON 保存と復元。ファイル版と、Play / Stop のスナップショット用の文字列版 |

## 主要な仕組み

### GameObject とコンポーネントの持ち方

`GameObject` はコンポーネントの実体を持ちません。実体は型ごとの `ComponentPool<T>` が 1 本の `std::vector` に並べて持ち、`GameObject` はプール内の添字（`ComponentRef`）だけを持ちます。System が「全 MeshRenderer を処理する」ときはメモリを順方向に読むだけで済みます。

`TransformComponent` だけは例外で、`GameObject` の組み込みフィールドとして必ず 1 つ存在します。親子はシーングラフの構造そのものなので、付け外しできるコンポーネントの枠には入れていません。親子はどちらも `GameObjectId` で参照し、張り替えは `Scene::SetParent` 経由でのみ行います。`SetParent` は循環を検出して拒否し、`worldPositionStays` が true なら張り替え前後で見た目の位置を保つように local を `Math::DecomposeAffine` で作り直します。

ワールド行列は `local_.world` にキャッシュし、`Scene::UpdateTransforms` がルートから再帰で確定します。合成順は「子 × 親」（行ベクトル規約）で、自分か祖先がダーティなときだけ再計算します。`GetLocalMutable` を呼んだ時点でダーティになります。

`Find` や `GetComponent` が返すポインタは、次にオブジェクトやコンポーネントを追加するまでの間だけ有効です。フレームをまたいで持ち回るのは `GameObjectId` です。

### 型の登録と Update

`TypeRegistry` は登録されたコンポーネント型の一覧です。インスペクタの Add Component メニューも、シーンの読み込みも、毎フレームの更新も、「型名や型 ID しか知らない」状態からここを引いて実体を操作します。

コンポーネントに次のメンバ関数があれば、自動的に毎フレーム呼ばれます。継承もインターフェースも不要です。

```cpp
void Update(const UpdateContext& ctx, GameObjectId self);
```

`Register<T>(priority)` の第 1 引数が実行順で、小さいほど先に走ります。目安として `kEarlyUpdatePriority`（-100）、`kDefaultUpdatePriority`（0）、`kLateUpdatePriority`（100）があります。同じ優先度なら登録順です。非アクティブな GameObject のコンポーネントは呼ばれません。

登録はエンジン側（`RegisterAllComponents`）とゲーム側（`Game::RegisterGameComponents`）の両方が終わった後、`Application` が `FinalizeRegistration` を 1 回呼んで締め切ります。ここで依存宣言の未登録と循環を検査し、以後の `Register` は拒否されます。

`CAKE_REQUIRE_COMPONENTS` で宣言した依存は、`TypeRegistry::AddComponent` 経由の追加では依存先を先に自動で付け、`FindDependent` で削除してはいけない型を教えます。`Scene::AddComponent` や `RemoveComponent` を直接呼ぶコードには効きません。

### 毎フレームの更新順

`SceneManager::Update(deltaTime, unscaledDeltaTime)` は次の順に処理します。

| 順 | 処理 | 実行条件 |
| --- | --- | --- |
| 1 | `RunComponentUpdates`（優先度順にコンポーネントの `Update`） | `deltaTime` が 0 より大きいとき |
| 2 | `Scene::UpdateTransforms`（ルートから再帰でワールド行列を確定） | 常に。インスペクタでの編集を反映するため |
| 3 | `UpdateParticleSystems` | 常に。停止中は外した・破棄した分の片付けだけを行い、生成・移動・消滅はしない |
| 4 | `RunCollisionDetection` | `deltaTime` が 0 より大きいとき |

エディタの Stop 中や一時停止中は `Time` が `deltaTime` を 0 にするため、コンポーネントと衝突判定は走りません。`SceneManager` は Play 状態を知らず、`deltaTime` だけを見ます。

### 衝突判定

`CollisionSystem` は球と箱を共通の形（`ColliderEntry`）へ潰してから総当たり（O(n²)）で判定します。箱同士は分離軸を 15 本調べる必要があるため、その前に外接球で早期棄却します。

| 形 | スケールの扱い |
| --- | --- |
| `SphereCollider` | `radius` をワールド単位でそのまま使う。`Transform` を拡大しても判定は太らない |
| `BoxCollider` | 向き付きの箱（OBB）。ワールド行列から軸と半長を取り出すため、回転とスケールが効く |

結果は各コライダーの `hits` に毎フレーム上書きします（最大 `kMaxHits` = 8 件）。押し戻しはせず、読んだ側が反応します。判定は移動後なので結果は 1 フレーム遅れます。`layer` と `mask` のビット演算で当たる相手を絞り、片方だけが相手を見ている組は見ている側にだけ記録します。

### パーティクル

設定は `ParticleSystemComponent`（出し方・動き方）と `ParticleRendererComponent`（mesh と material）に分け、実行時の状態（粒の配列、発生タイマー）は `ParticleStorage` に置きます。`ParticleSystemComponent` を `TypeRegistry::AddComponent` で付けると、`ParticleRendererComponent` も付きます。

`UpdateParticleSystems` はエミッターごとに次の順で処理します。

1. 有効な Field をフレーム先頭で 1 回だけ集め、逆行列を控える。
2. 粒ごとにワールド位置を各 Field のローカルへ戻して箱の内外を判定し、合算した加速度をエミッターのローカルへ変換して `velocity` に足す。
3. `currentTime` と `translate` を進める。
4. 寿命に達した粒を消す。
5. `isEnableEmission` が true かつ `rateOverTime` が 0 より大きいとき、発生タイマーを進め、`max(burstCount, 1) / rateOverTime` 秒（下限 `kMinEmitInterval` = 0.01 秒）ごとに `burstCount` 個をまとめて出す。無効中はタイマーを進めない。
6. `drawColor` を `startColor` から計算し直す。`isEnableColorOverLifetime` のときは年齢（`currentTime / lifeTime`）で `colorOverLifetime.Sorted().Evaluate` を引いて成分ごとに乗算する。

粒の位置と速度はエミッターからの相対です。生成時の位置は各軸 ±1（`kSpawnRange`）の一様乱数、速度は各軸 ±1 の一様乱数に `startSpeed` を掛けたものです。色と寿命は生成時に粒へコピーするので、後から設定を変えても出ている粒には影響しません。粒数の上限はありません。

`ParticleForceFieldComponent` は範囲内の粒に加速度を与えます。範囲は `BoxColliderComponent` と同じ読み方（Transform から見たローカルの箱）で、`acceleration` はワールド空間です。すべてのエミッターの粒に効き、絞り込みはありません。

### シリアライズと Play / Stop

`SceneSerializer` はコンポーネントの型を知りません。`TypeRegistry` から `TypeInfo` を引き、プロパティ一覧を回して読み書きします。

| 項目 | 内容 |
| --- | --- |
| 階層 | フラットな配列で、各オブジェクトが親の index を持つ。親を張り替えても JSON の構造が動かない |
| ID | `GameObjectId` の index だけを保存し、読み込み時に新しい ID へ読み替える（親子と `EntityRef` も同様）。2 周で読み、1 周目で全オブジェクトを作って対応表を作る |
| コンポーネント | `type` に型名、以降にプロパティ名をキーとした値。読み込みは `TypeRegistry::AddComponent` 経由なので依存先が JSON に無くても補われる |
| 保存しないもの | ワールド行列、ダーティフラグ、`AssetRef` の `handle`、リフレクションに載っていないメンバ（コライダーの `hits` など） |
| 版数 | `kSceneVersion` = 1 |

Play / Stop は `PlayModeController`（Editor 層）がシーンを JSON 文字列でスナップショットして復元します。リフレクションに載っていない状態は復元されません。Stop 後は既存の `GameObjectId` がすべて無効になります。

## コンポーネントを追加する

新しいコンポーネントを作り、インスペクタで編集でき、シーンに保存され、毎フレーム更新されるようにするまでの手順です。エンジン側（`project/Engine/Scene/Component/`）とゲーム側（`project/Game/Components/`）で、登録する場所だけが違います。

### 1. 構造体を書く

コンポーネントは仮想関数を持たないデータだけの構造体です。毎フレーム動かしたい場合だけ、`Update` を書きます。`project/Game/Components/RotatorComponent.h` が最小の例です。

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

構造体の直後に `CAKE_REFLECT` ブロックを書き、インスペクタに出したい・保存したいメンバを 1 行ずつ並べます。別ファイルに離すと、メンバを増やしたときに書き足し忘れるため、同じヘッダに置きます。マクロの一覧は [FoundationLayer.md](FoundationLayer.md) にあります。

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

第 1 引数は `Update` の実行順で、小さいほど先に走ります。`Update` を持たない型では意味を持たないため、省略できます。

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

### 他のコンポーネントを必要とする場合

`CAKE_REFLECT_END()` の直後に `CAKE_REQUIRE_COMPONENTS` を書くと、インスペクタの Add Component で依存先も一緒に付き、依存先だけを削除できなくなります。

```cpp
CAKE_REFLECT(ParticleSystemComponent)
// ...
CAKE_REFLECT_END()
CAKE_REQUIRE_COMPONENTS(ParticleSystemComponent, ParticleRendererComponent)
```

### 新しい型のプロパティが必要な場合

`PropertyType` に無い型（列挙など）をインスペクタに出すには、次の 3 箇所を揃えて追加します。

1. `Foundation/Reflection/TypeInfo.h` の `PropertyType` と `PropertyTypeOf`
2. `Editor/Inspector/PropertyDrawer.cpp` の switch（描画）
3. `Scene/Serialize/SceneSerializer.cpp` の `WriteProperty` と `ReadProperty` の switch（保存と復元）

### 確認

- Debug 構成で起動し、Inspector の Add Component に型名が出る。
- 値を変えて Ctrl+S で保存し、`.scene` ファイルに `"type": "型名"` とプロパティが書かれている。
- Play を押すと `Update` が動き、Stop で元に戻る。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | Render 層以下にのみ依存する。Editor 層と Application 層はこの層の上 |
| 実装上の依存 | Foundation（Math、Reflection、Debug、Serialize）、Asset（`AssetRef`、各 Handle、`AssetDatabase`）、Platform（`SceneManager` → `Platform.h`、`UpdateContext` → Input）、Render（`CameraView`、`Light.h`、`GizmoDrawList`、`SpriteDrawList`、`Renderer`） |
| 外部 | nlohmann/json（`SceneSerializer`）、ImGui と ImGuizmo（`GizmoSystem.h` が `GizmoSettings` のために無条件に include） |
| 規則との食い違い | Render 層と相互に参照している（Render 層の各パスが Scene の System を呼び、Scene の System が Render のヘッダを使う）。詳細は [InternalDesign.md](../InternalDesign.md) |
| 規則との食い違い | Scene 層が ImGui に依存している（`GizmoSystem.h`。`ENABLE_EDITOR` や `USE_IMGUI` の分岐なし） |
| 逆方向の参照 | Foundation 層の `TypeInfo.h` が `GameObjectId.h` を include している |

## 守るべき規則

### コンポーネント

- コンポーネントは仮想関数を持たない素の構造体（standard-layout）にする。継承は使わない。`offsetof` によるリフレクションと `memcpy` によるスナップショットが前提。
- 粒の配列のように増減するデータはコンポーネントに置かず、`ParticleStorage` のように `Scene` 側で管理する。
- 追加時は、構造体と同じヘッダに `CAKE_REFLECT` ブロックを書き、`RegisterAllComponents`（ゲーム側は `GameModule.cpp`）に登録する。登録しないとインスペクタ表示もシーン保存もされない。
- 同じ型のコンポーネントを 1 つの GameObject に重複して追加しない。`AddComponent` は既存のものを返す。
- 依存コンポーネントを使う System は、依存先がなくても安全に動くようにする。

### 更新と登録

- `Update` 中にコンポーネントを追加・削除しない。プールの再確保で走査中の参照が無効になる。
- 型 ID は初回使用順で採番され、実行ごとに変わる可能性があるため保存しない。保存には型名を使う。
- 登録は明示的に行い、静的オブジェクトによる自己登録はしない。

### 参照と System

- `Find`、`GetComponent`、`ComponentPool::Get` が返すポインタは次の追加まで有効。フレームをまたいで保持する識別子は `GameObjectId`。
- 行列を計算する System は作らず、`TransformComponent` がキャッシュした world を使う。
- `CollisionSystem`、`GizmoSystem`、`CameraSystem`、`LightSystem`、`RenderSystem`、`SpriteSystem` は `UpdateTransforms` の後に呼ぶ。
- `UpdateContext` はそのフレームの `Update` 中だけ有効。メンバに保持しない。ゲーム入力には `GetKey` 系を使う。
- `CameraSystem` と `LightSystem` は有効なものから最小 `priority` の 1 つを選ぶ。同値ならプール順。ライトは現状 1 灯のみ。

### 衝突と複製

- 衝突結果は毎フレーム `hits` に上書きし、押し戻しはしない。`SetActive(false)` のオブジェクトは対象外。
- `CollisionSystem.cpp` の `CollectBoxes` と `CollectSpheres` はコライダーのメンバを直接扱うため、メンバ変更時に合わせて更新する。
- コライダーの `hits` と `hitCount` はリフレクションに載せない。
- `CloneSystem` は `EntityRef` を張り替えない。複製した部分木の内部参照も元の相手を指したままになる。

### パーティクル

- `ParticlePass` は粒の状態を読むだけで変更しない。更新は `UpdateParticleSystems` だけが行う。
- Field が「潰れている」（ワールド行列のいずれかの軸の長さが 1e-6 以下）ならその Field を集めない。エミッター側が潰れていればそのエミッターには Field を効かせない。

### スプライト

- `SpriteRendererComponent` を持つ GameObject の Transform は、基準解像度のピクセル座標として扱う。使うのは translate.xy、rotate.z、scale.xy で、左上が原点、y は下向き。
- 表示サイズは `size`（ピクセル）で、Transform の scale.xy がさらに掛かる。`color` はテクスチャの色に掛ける色で、w（不透明度）を下げると半透明になる。
- スプライトを 3D のオブジェクトの子にしない。親のワールド行列（メートル単位）が掛かり、画面上の位置が崩れる。
- `sortingOrder` が同じスプライトどうしは、プールに並んでいる順に描かれる。プールは削除で空いた枠を再利用するため、作った順になるとは限らない。

### シリアライズ

- `PropertyType` を追加したら `SceneSerializer` と `PropertyDrawer` の両方の switch を更新する。
- Play 中にだけ変わる値は、必ず `CAKE_PROPERTY` で登録されたメンバに置く。

## 未実装・要確認

- 粒の `rotate` と `scale` は System が変更しないため、常に 0 と (1, 1) のまま描かれます。将来の Size / Rotation over Lifetime 用の予約枠と読めます。
- `ParticleSystemComponent.h`、`ParticleSystem.h`、`ParticleStorage.h` のヘッダコメントは旧仕様（`color`、`emitInterval`、`lifeTime`）のままで、現在のメンバ名と一致していません。
- `System/` 直下と専用フォルダの使い分けは途中です。
- `startSpeed` は各軸 ±1 の乱数ベクトルに掛けるため、実際の速さは 0 から √3 × `startSpeed` の範囲でばらつきます。コメントの「m/s」とは一致しません。
- `Game/Components/Player/PlayerComponent.h` のヘッダコメントは `RotatorComponent` のものがそのまま残っています（「毎フレーム自分を回転させる」）。
