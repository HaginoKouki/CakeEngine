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
| `Component/` | `CameraComponent`、`LightComponent`、`MeshRendererComponent` | データのみの構造体 |
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
| `System/Collision/` | `CollisionSystem`（`RunCollisionDetection`） | 球と箱の総当たり判定 |
| `System/Particle/` | `ParticleSystem`（`UpdateParticleSystems`） | 粒の生成、移動、消滅、Field の適用、色の計算 |
| `Serialize/` | `SceneSerializer` | シーンの JSON 保存と復元。ファイル版と、Play / Stop のスナップショット用の文字列版 |

## 主要な仕組み

### GameObject とコンポーネントの持ち方

`GameObject` はコンポーネントの実体を持ちません。実体は型ごとの `ComponentPool<T>` が 1 本の `std::vector` に並べて持ち、`GameObject` はプール内の添字（`ComponentRef`）だけを持ちます。System が「全 MeshRenderer を処理する」ときはメモリを順方向に読むだけで済みます。

`TransformComponent` だけは例外で、`GameObject` の組み込みフィールドとして必ず 1 つ存在します。親子はシーングラフの構造そのものなので、付け外しできるコンポーネントの枠には入れていません。親子はどちらも `GameObjectId` で参照し、張り替えは `Scene::SetParent` 経由でのみ行います。

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

`CollisionSystem` は球と箱を共通の形へ潰してから総当たり（O(n²)）で判定します。箱同士は分離軸を 15 本調べる必要があるため、その前に外接球で早期棄却します。

| 形 | スケールの扱い |
| --- | --- |
| `SphereCollider` | `radius` をワールド単位でそのまま使う。`Transform` を拡大しても判定は太らない |
| `BoxCollider` | 向き付きの箱（OBB）。ワールド行列から軸と半長を取り出すため、回転とスケールが効く |

結果は各コライダーの `hits` に毎フレーム上書きします。押し戻しはせず、読んだ側が反応します。判定は移動後なので結果は 1 フレーム遅れます。`layer` と `mask` のビット演算で当たる相手を絞ります。

### パーティクル

設定は `ParticleSystemComponent`（出し方・動き方）と `ParticleRendererComponent`（mesh と material）に分け、実行時の状態（粒の配列、発生タイマー）は `ParticleStorage` に置きます。`ParticleSystemComponent` を `TypeRegistry::AddComponent` で付けると、`ParticleRendererComponent` も付きます。

`UpdateParticleSystems` はエミッターごとに次の順で処理します。

1. 有効な Field をフレーム先頭で 1 回だけ集め、逆行列を控える。
2. 粒ごとにワールド位置を各 Field のローカルへ戻して箱の内外を判定し、合算した加速度をエミッターのローカルへ変換して `velocity` に足す。
3. `currentTime` と `translate` を進める。
4. 寿命に達した粒を消す。
5. `isEnableEmission` が true かつ `rateOverTime` が 0 より大きいとき、発生タイマーを進め、`max(burstCount, 1) / rateOverTime` 秒（下限 `kMinEmitInterval` = 0.01 秒）ごとに `burstCount` 個をまとめて出す。
6. `drawColor` を `startColor` から計算し直す。`isEnableColorOverLifetime` のときは年齢（`currentTime / lifeTime`）で `colorOverLifetime.Sorted().Evaluate` を引いて成分ごとに乗算する。

粒の位置と速度はエミッターからの相対です。生成時の位置は各軸 ±1（`kSpawnRange`）の一様乱数、速度は各軸 ±1 の一様乱数に `startSpeed` を掛けたものです。色と寿命は生成時に粒へコピーするので、後から設定を変えても出ている粒には影響しません。粒数の上限はありません。

`ParticleForceFieldComponent` は範囲内の粒に加速度を与えます。範囲は `BoxColliderComponent` と同じ読み方（Transform から見たローカルの箱）で、`acceleration` はワールド空間です。すべてのエミッターの粒に効き、絞り込みはありません。

### シリアライズと Play / Stop

`SceneSerializer` はコンポーネントの型を知りません。`TypeRegistry` から `TypeInfo` を引き、プロパティ一覧を回して読み書きします。

| 項目 | 内容 |
| --- | --- |
| 階層 | フラットな配列で、各オブジェクトが親の index を持つ。親を張り替えても JSON の構造が動かない |
| ID | `GameObjectId` の index だけを保存し、読み込み時に新しい ID へ読み替える（親子と `EntityRef` も同様） |
| 保存しないもの | ワールド行列、ダーティフラグ、`AssetRef` の `handle`、リフレクションに載っていないメンバ（コライダーの `hits` など） |
| 版数 | `kSceneVersion` = 1 |

Play / Stop は `PlayModeController`（Editor 層）がシーンを JSON 文字列でスナップショットして復元します。リフレクションに載っていない状態は復元されません。Stop 後は既存の `GameObjectId` がすべて無効になります。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | Render 層以下にのみ依存する。Editor 層と Application 層はこの層の上 |
| 実装上の依存 | Foundation（Math、Reflection、Debug、Serialize）、Asset（`AssetRef`、各 Handle、`AssetDatabase`）、Platform（`SceneManager` → `Platform.h`、`UpdateContext` → Input）、Render（`CameraView`、`Light.h`、`GizmoDrawList`、`Renderer`） |
| 外部 | nlohmann/json（`SceneSerializer`）、ImGui と ImGuizmo（`GizmoSystem.h` が `GizmoSettings` のために無条件に include） |
| 規則との食い違い | Render 層と相互に参照している（Render 層の各パスが Scene の System を呼び、Scene の System が Render のヘッダを使う）。詳細は [Overview.md](Overview.md) |
| 規則との食い違い | Scene 層が ImGui に依存している（`GizmoSystem.h`。`ENABLE_EDITOR` や `USE_IMGUI` の分岐なし） |
| 逆方向の参照 | Foundation 層の `TypeInfo.h` が `GameObjectId.h` を include している |

## 守るべき規則

### コンポーネント

- コンポーネントは仮想関数を持たない素の構造体（standard-layout）にする。継承は使わない。`offsetof` によるリフレクションと `memcpy` によるスナップショットが前提。
- 粒の配列のように増減するデータはコンポーネントに置かず、`ParticleStorage` のように `Scene` 側で管理する。
- 追加時は、構造体と同じヘッダに `CAKE_REFLECT` ブロックを書き、`RegisterAllComponents`（ゲーム側は `GameModule.cpp`）に登録する。登録しないとインスペクタ表示もシーン保存もされない。手順は [guides/AddingComponents.md](../guides/AddingComponents.md) を参照。
- 同じ型のコンポーネントを 1 つの GameObject に重複して追加しない。`AddComponent` は既存のものを返す。
- 依存コンポーネントを使う System は、依存先がなくても安全に動くようにする。

### 更新と登録

- `Update` 中にコンポーネントを追加・削除しない。プールの再確保で走査中の参照が無効になる。
- 型 ID は初回使用順で採番され、実行ごとに変わる可能性があるため保存しない。保存には型名を使う。
- 登録は明示的に行い、静的オブジェクトによる自己登録はしない。

### 参照と System

- `Find`、`GetComponent`、`ComponentPool::Get` が返すポインタは次の追加まで有効。フレームをまたいで保持する識別子は `GameObjectId`。
- 行列を計算する System は作らず、`TransformComponent` がキャッシュした world を使う。
- `CollisionSystem`、`GizmoSystem`、`CameraSystem`、`LightSystem`、`RenderSystem` は `UpdateTransforms` の後に呼ぶ。
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

### シリアライズ

- `PropertyType` を追加したら `SceneSerializer` と `PropertyDrawer` の両方の switch を更新する。
- Play 中にだけ変わる値は、必ず `CAKE_PROPERTY` で登録されたメンバに置く。

## 未実装・要確認

- 粒の `rotate` と `scale` は System が変更しないため、常に 0 と (1, 1) のまま描かれます。将来の Size / Rotation over Lifetime 用の予約枠と読めます。
- `ParticleSystemComponent.h` と `ParticleSystem.h` のヘッダコメントは旧仕様（`color`、`emitInterval`、`lifeTime`）のままで、現在のメンバ名と一致していません。
- `System/` 直下と専用フォルダの使い分けは途中です（コミットメッセージに「後でもっと分ける」とあります）。
- `startSpeed` は各軸 ±1 の乱数ベクトルに掛けるため、実際の速さは 0 から √3 × `startSpeed` の範囲でばらつきます。コメントの「m/s」とは一致しません。
