# Scene層
*ゲームの内容が詰まってる*



GameObject とコンポーネントを所有するシーン、その更新・保存、コンポーネントに対する処理（System）を担当します。



## 場所
`project/Engine/Scene/`



## 構成
- Scene / SceneManager … Scene は GameObject と型ごとのコンポーネントプール、親子階層（roots_）、ParticleStorage を所有。SceneManager は現在のシーンを1つ所有し、生成・読込・保存・更新を管理する。編集中か実行中かは知らず、deltaTime が 0 ならコンポーネントを走らせない。
- Object/ … GameObject（識別子・名前・TransformComponent・ComponentRef の一覧を持つ器）、GameObjectId（index + generation）、ComponentPool\<T>（型ごとの連続配置プール。持ち主IDを並行配列で持つ）、ParticleStorage（エミッターごとの粒の状態と発生タイマー）。
- Component/ … データのみの素の構造体。Transform / Light / Camera / MeshRenderer / SphereCollider / BoxCollider / ParticleSystem / ParticleRenderer。TransformComponent だけは GameObject の組み込みフィールドで、プールにも登録簿にも入らない（インスペクタとシリアライザが個別に扱う）。
- Component/ComponentRegistry/ … TypeRegistry（型名・型IDから TypeInfo と ComponentOps（add / get / remove / copy / update）を引く登録簿）、RequireComponent（依存宣言。CAKE_REQUIRE_COMPONENTS）。
- Component/ComponentRegistration … RegisterAllComponents（エンジン側コンポーネントの登録。1行足す方式）。
- System/ … RenderSystem（DrawMeshRenderers、ResolveSceneAssets）、CameraSystem（CollectMainCamera）、LightSystem（CollectDirectionalLight）、GizmoSystem（CollectSceneGizmos）、CollisionSystem（RunCollisionDetection）、ParticleSystem（UpdateParticleSystems）、CloneSystem（DuplicateGameObject）、ComponentUpdateSystem（RunComponentUpdates）、UpdateContext（Update に渡す外部情報一式）。
- Serialize/SceneSerializer … シーンのJSON保存・復元（ファイル版と、Play/Stop のスナップショット用の文字列版）。

SceneManager::Update の順序:
- 1. deltaTime > 0 のとき RunComponentUpdates（優先度順）→ 2. UpdateTransforms（常に実行）→ 3. UpdateParticleSystems（常に実行。停止中は片付けのみで、生成・移動・消滅はしない）→ 4. deltaTime > 0 のとき RunCollisionDetection

## 依存関係
規則（Overview.mdの階層）:
- 自分以下の層にのみ依存する。Editor層・Application層はこの層の上。

現状の実装:
- Foundation … Math、Reflection、Debug、Serialize など。
- Asset … AssetRef / 各Handle（MeshRendererComponent など）、AssetDatabase（SceneManager、SceneSerializer、RenderSystem::ResolveSceneAssets）。
- Platform … SceneManager.cpp → Platform.h、System/UpdateContext.h → Input/KeyCode.h・KeyState.h、UpdateContext.cpp → InputManager.h。
- Render … System/CameraSystem.cpp → CameraView.h、GizmoSystem → Gizmo/GizmoDrawList.h、LightSystem.cpp → Light.h、RenderSystem → Renderer.h・CameraView.h。
- 外部 … nlohmann/json（SceneSerializer）、ImGui / ImGuizmo（System/GizmoSystem.h が GizmoSettings のために imgui.h と ImGuizmo.h を無条件にinclude）。

規則との食い違い（現状の実装）:
- Render ⇄ Scene が循環している（Render層の各パスが Scene の System を呼び、Scene の System が Render のヘッダを使う）。RenderLayer.md にも同じ記載がある。
- Scene層が ImGui に依存している（GizmoSystem.h。ENABLE_EDITOR / USE_IMGUI の分岐なし）。
- Foundation/Reflection/TypeInfo.h が GameObjectId をincludeしている（Foundationの逆依存）。

## 規則

### コンポーネント

- コンポーネントは仮想関数を持たない素の構造体（standard-layout）にする。継承は使わない。`offsetof` によるリフレクションと `memcpy` スナップショットが前提。
- 粒の配列のように増減するデータはコンポーネントに置かず、`ParticleStorage` のように `Scene` 側で管理する。
- 追加時は、構造体と同じヘッダに `CAKE_REFLECT` ブロックを書き、`RegisterAllComponents`（ゲーム側は `GameModule.cpp`）に登録する。登録しないとインスペクタ表示もシーン保存もされない。
- 同じ型のコンポーネントを1つの `GameObject` に重複して追加しない。`AddComponent` は既存のものを返す。
- `CAKE_REQUIRE_COMPONENTS` の依存宣言は `TypeRegistry::AddComponent` / `FindDependent` 経由でのみ適用される。`Scene::AddComponent` / `RemoveComponent` の直接呼び出しでは適用されない。依存コンポーネントを使うSystemは、依存先がなくても安全に動くようにする。

### 更新と登録

- `void Update(const UpdateContext& ctx, GameObjectId self)` を持つ型だけが自動更新される。継承やインターフェースは使わない。
- 更新順は登録優先度の小さい順。標準値は早期 `-100`、通常 `0`、後期 `100`。同じ優先度なら登録順。
- `Update` 中にコンポーネントを追加・削除しない。プールの再確保で走査中の参照が無効になる。
- 型IDは初回使用順で採番され、実行ごとに変わる可能性があるため保存しない。保存には型名を使う。
- 登録は明示的に行い、静的オブジェクトによる自己登録はしない。エンジンとゲームの登録後に `Application` が `FinalizeRegistration` を1回呼ぶ。それ以降の登録は拒否される。

### 参照とSystem

- `Find` / `GetComponent` / `ComponentPool::Get` が返すポインタは、次の追加まで有効。フレームをまたいで保持する識別子は `GameObjectId`。
- System はコンポーネントを処理する場所。`Scene` と `Scene::UpdateTransforms` を前提にする。
- 行列を計算するSystemは作らず、`TransformComponent` がキャッシュした `world` を使う。
- `CollisionSystem`、`GizmoSystem`、`CameraSystem`、`LightSystem`、`RenderSystem` は `UpdateTransforms` の後に呼ぶ。
- `UpdateContext` はそのフレームの `Update` 中だけ有効。メンバに保持しない。ゲーム入力にはゲーム入力経路の `GetKey` 系を使う。
- `CameraSystem` / `LightSystem` は有効なものから最小priorityの1つを選ぶ。同値ならプール順。ライトは現状1灯のみ。

### 衝突と複製

- 球（`SphereCollider`）は `radius` をワールド単位のまま使い、スケールを反映しない。箱（`BoxCollider`）は OBB とし、回転とスケールを反映する。
- 衝突結果は毎フレーム `hits` に上書きし、押し戻しはしない。判定は移動後なので結果は1フレーム遅れる。`SetActive(false)` のオブジェクトは対象外。
- 判定は総当たり（O(n²)）。箱同士は外接球で早期棄却する。`CollisionSystem.cpp` の `CollectBoxes` / `CollectSpheres` はメンバを直接扱うため、コライダーのメンバ変更時に合わせて更新する。
- `Collider` の `hits` / `hitCount` はリフレクションに載せない。保存もインスペクタ表示もしない。
- `CloneSystem` は `EntityRef` を張り替えない。複製した部分木の内部参照も元の相手を指したままになる。

### シリアライズとPlay/Stop

- `SceneSerializer` は `GameObjectId` のindexだけを保存し、読込時に新しいIDへ置き換える。階層はフラットで、各オブジェクトが親のindexを持つ。
- ワールド行列、ダーティフラグ、`AssetRef` のhandleは保存しない。`PropertyType` を追加したら `SceneSerializer` と `PropertyDrawer` の両方のswitchを更新する。
- Play/Stop はシーンをJSON文字列でスナップショットして復元する。リフレクションに載っていない状態は復元されない。Stop後は既存の `GameObjectId` がすべて無効になる。
