# Scene層
*ゲームの内容が詰まってる*<br>
<br>
GameObject とコンポーネントを所有するシーン、その更新・保存、コンポーネントに対する処理（System）を担当します。<br>
<br>
## 場所
`project/Engine/Scene/`<br>
<br>
## 構成
- Scene / SceneManager … Scene は GameObject と型ごとのコンポーネントプール、親子階層（roots_）、ParticleStorage を所有。SceneManager は現在のシーンを1つ所有し、生成・読込・保存・更新を管理する。編集中か実行中かは知らず、deltaTime が 0 ならコンポーネントを走らせない。
- Object/ … GameObject（識別子・名前・TransformComponent・ComponentRef の一覧を持つ器）、GameObjectId（index + generation）、ComponentPool\<T>（型ごとの連続配置プール。持ち主IDを並行配列で持つ）、ParticleStorage（エミッターごとの粒の状態と発生タイマー）。
- Component/ … データのみの素の構造体。Transform / Light / Camera / MeshRenderer / SphereCollider / BoxCollider / ParticleSystem / ParticleRenderer。TransformComponent だけは GameObject の組み込みフィールドで、プールにも登録簿にも入らない（インスペクタとシリアライザが個別に扱う）。
- Component/ComponentRegistry/ … TypeRegistry（型名・型IDから TypeInfo と ComponentOps（add / get / remove / copy / update）を引く登録簿）、RequireComponent（依存宣言。CAKE_REQUIRE_COMPONENTS）。
- Component/ComponentRegistration … RegisterAllComponents（エンジン側コンポーネントの登録。1行足す方式）。
- System/ … RenderSystem（DrawMeshRenderers、ResolveSceneAssets）、CameraSystem（CollectMainCamera）、LightSystem（CollectDirectionalLight）、GizmoSystem（CollectSceneGizmos）、CollisionSystem（RunCollisionDetection）、ParticleSystem（UpdateParticleSystems）、CloneSystem（DuplicateGameObject）、ComponentUpdateSystem（RunComponentUpdates）、UpdateContext（Update に渡す外部情報一式）。
- Serialize/SceneSerializer … シーンのJSON保存・復元（ファイル版と、Play/Stop のスナップショット用の文字列版）。

SceneManager::Update の順序:
- 1. deltaTime > 0 のとき RunComponentUpdates（優先度順）→ 2. UpdateTransforms（常に実行）→ 3. 3. UpdateParticleSystems（常に実行。停止中は片付けのみで、生成・移動・消滅はしない）→ 4. deltaTime > 0 のとき RunCollisionDetection

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
- コンポーネントは仮想関数を持たない素の構造体（standard-layout）を保つ。継承させない。offsetof によるリフレクションと memcpy スナップショットの前提のため。粒の配列のように増減するデータはコンポーネントに持たせず、ParticleStorage のように Scene 側へ置く。
- コンポーネントの追加手順: 構造体を作り、同じヘッダに CAKE_REFLECT ブロックを書き、RegisterAllComponents（ゲーム側は GameModule.cpp）に1行足す。書き忘れるとインスペクタに出ず、シーンにも保存されない。
- Update は継承やインターフェースではなく、`void Update(const UpdateContext& ctx, GameObjectId self)` を書いた型だけが自動で呼ばれる。実行順は Register の優先度（小さいほど先。kEarlyUpdatePriority = -100、kDefaultUpdatePriority = 0、kLateUpdatePriority = 100）。同値は登録順。
- Update の中でコンポーネントの追加・削除をしない（走査中のプールが再確保されて参照が壊れる）。
- 型IDは初回使用順で採番され、実行のたびに変わりうる。シリアライズしない。保存に使うのは型名。
- 参照の寿命: Find / GetComponent / ComponentPool::Get が返すポインタは次の追加までしか有効でない。フレームをまたいで持つのは GameObjectId。
- 同じ型のコンポーネントを1つの GameObject に重複して持たせない（AddComponent は既存を返す）。
- System はコンポーネントの処理を持つ場所で、Scene と Scene::UpdateTransforms を前提とする。行列を計算する System はなく、TransformComponent がキャッシュした world を使う。CollisionSystem・GizmoSystem・CameraSystem・LightSystem・RenderSystem は UpdateTransforms の後に呼ぶ。
- 依存宣言（CAKE_REQUIRE_COMPONENTS）が効くのは TypeRegistry::AddComponent / FindDependent 経由だけ。Scene::AddComponent / RemoveComponent を直接呼ぶと素通りするため、依存先を使う System は依存先が無くても落ちないように書く。
- 登録は明示的に行い、静的オブジェクトによる自己登録はしない。エンジン側とゲーム側の登録が終わったら Application が FinalizeRegistration を1回呼ぶ。以後の Register は拒否される。
- 衝突判定: 球（SphereCollider）は radius をワールド単位のまま使い、スケールが効かない。箱（BoxCollider）は OBB でスケールも回転も効く。結果は毎フレーム hits に上書きされ、押し戻しはしない。判定は移動の後に走るため、hits は1フレーム遅れる。SetActive(false) のオブジェクトは参加しない。総当たり（O(n²)）で、箱同士は包む球で早期棄却する。CollisionSystem.cpp の CollectBoxes / CollectSpheres はコライダーのメンバを直接読み書きするため、メンバを増減したら一緒に直す。
- Collider の hits と hitCount はリフレクションに載せない（保存もインスペクタ表示もされない）。
- CameraSystem / LightSystem は有効なもののうち priority が最小の1つだけを採用する（同値はプール順）。ライトは現状1灯のみ。
- CloneSystem は EntityRef を張り替えない。複製した部分木の中で閉じた参照も、元の相手を指したままになる。
- SceneSerializer は GameObjectId のうち index だけを保存し、読込時に新しいIDへ読み替える。階層はフラットで、親の index を各オブジェクトが持つ。ワールド行列・ダーティフラグ・AssetRef の handle は保存しない。PropertyType を増やしたら SceneSerializer と PropertyDrawer の両方の switch に追加する。
- Play/Stop はシーンを JSON 文字列でスナップショットして復元する。リフレクションに載っていない状態は復元されない。Stop 後は既存の GameObjectId がすべて無効になる。
- UpdateContext はそのフレームの Update の間だけ有効で、メンバに保持しない。ゲーム側の入力は GetKey 系（ゲーム入力経路）を使う。
