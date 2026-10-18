# Render 層

描画命令の発行（`Renderer`）と、シーンを 1 枚の絵にする工程（`SceneRenderer` とパス列）を担当する層です。

## この層が担当すること

Render 層は「視点と描画先が決まった状態で、シーンの中身をどの順にどう描くか」を担当します。GPU リソースの生成は Graphics 層、描くもの（メッシュやマテリアル）の所有は Asset 層、描く対象の状態（コンポーネント）は Scene 層が持ちます。

## 配置

`project/Engine/Render/`

## 構成

| ファイル | 主な型 | 役割 |
| --- | --- | --- |
| `Renderer.h` / `.cpp` | `Renderer`、`RendererInitDesc`、`InstanceData` | GPU 描画命令の発行と描画状態の管理。フレームとシーンの開始・終了、バックバッファの準備、`DrawModel` などの描画関数、ライトとカメラのバインド |
| `SceneRenderer.h` / `.cpp` | `SceneRenderer` | パス列を保持して順に実行する。シーン内のメインカメラから視点を作る `TryMakeMainCameraView` も持つ |
| `CameraView.h` / `.cpp` | `CameraView`、`MakeCameraView` | 1 回の描画で使う視点（view、projection、回転のみの行列、位置）。ゲームカメラもエディタカメラも同じ形になる |
| `Light.h`、`LightManager.h` / `.cpp` | `DirectionalLight`、`LightManager` | 平行光源の値と定数バッファ。値を書くだけで、バインドは `Renderer` が行う |
| `Pass/RenderPass.h` | `RenderPass`、`RenderContext`、`RenderViewOptions` | パスの基底。`RenderContext` は scene、view、renderer、options を束ねる |
| `Pass/` | `LightSetupPass`、`SkyboxPass`、`OpaquePass`、`ParticlePass`、`GizmoPass` | 既定のパス 5 つ |
| `Gizmo/GizmoDrawList.h` / `.cpp` | `GizmoDrawList`、`GizmoVertex`、`GizmoColor`、`GizmoGrid` | ギズモを線分へ分解して 1 本の LINELIST 頂点列に貯める入れ物 |

## 主要な仕組み

### Renderer の関数と役割

| 分類 | 関数 | 内容 |
| --- | --- | --- |
| フレーム | `BeginFrame` / `EndFrame` | フレーム内バッファの `Reset`、コマンドリストの取得とバックバッファのバリア。`EndFrame` で `ExecuteAndWait` と Present を行う |
| 描画先 | `BeginScene` / `EndScene` | オフスクリーンの `SceneRenderTarget` を描画先にし、終わったら ImGui が読める状態へ戻す。エディタの 2 ビューが使う |
| 描画先 | `PrepareBackbufferToEditor` | ImGui だけを描くためにバックバッファを用意する（深度無し） |
| 描画先 | `PrepareBackbufferToGame` | ゲームをバックバッファへ直接描く。黒でクリアし、設計アスペクト比を保つビューポートを置く（レターボックス） |
| 描画 | `DrawModel` | 共有モデルを world 行列と視点で描く。`materialOverride` が有効なら全サブメッシュをそれ 1 枚で描く |
| 描画 | `DrawModelInstanced` | 同じモデルをインスタンシングで描く（パーティクル用）。マテリアルのシェーダーが `requiresInstancing` でなければ 1 個ずつ `DrawModel` で描く |
| 描画 | `DrawSkybox`、`DrawSprite`、`DrawGizmos` | 専用の描き方 |
| バインド | `SetLight` | `b1` に平行光源をバインドする |
| バインド | `SetCamera` | `CameraView::position` を `CameraForGPU` に詰めてフレーム定数バッファへ書き、`b2` にバインドする。ビューごとに別アドレスへ書くので、ゲームビューとシーンビューで上書きし合わない |

`Renderer` は起動時に `__Skybox` という名前の `Runtime` マテリアル（`Skybox/Panoramic` シェーダー）を作り、`EngineResources/textures/Skybox.png` を貼ります。実ファイル名は小文字の `skybox.png` で、Windows では動きますが大文字小文字が一致していません。

### パス列

`SceneRenderer::Initialize` は次の順にパスを組み立てます。この並びがそのまま描画順です。

| 順 | パス | 何をするか | 呼び出す System |
| --- | --- | --- | --- |
| 1 | `LightSetupPass` | シーン内の平行光源を集めて `Renderer::SetLight` で `b1` にバインドする。自身は何も描かない | `CollectDirectionalLight` |
| 2 | `SkyboxPass` | 背景を描く。最背面を埋めるため不透明より前に置く | |
| 3 | `OpaquePass` | 可視な `MeshRendererComponent` をすべて描く | `DrawMeshRenderers` |
| 4 | `ParticlePass` | 粒をカメラ正面向きの板（ビルボード）としてインスタンシング描画する。半透明を想定し不透明の後に置く | `Scene::GetParticleStore` を直接読む |
| 5 | `GizmoPass` | コライダーやカメラの視錐台など、通常は絵に出ない情報を線で重ねる。`options->gizmos` が `nullptr` なら何もしない | `CollectSceneGizmos` |

`SceneRenderer::Render` は先頭で `Renderer::SetCamera(view)` を呼び、その後パス列を実行します。パスは `SetCamera` を呼びません。

### 視点の作り方

`CameraView` は world 行列、画角、near と far、出力先の解像度から `MakeCameraView` で作ります。world からスケールを打ち消すために各軸を正規化し、`viewMatrix` は回転と平行移動の合成の逆行列です。アスペクト比は出力先の解像度から毎回計算するため、どこにも保存しません。

| 視点 | 作る場所 | 描画先 |
| --- | --- | --- |
| ゲームカメラ | `CameraSystem::CollectMainCamera`（Scene 層）。有効な `CameraComponent` のうち `priority` が最小のものを採用する | リリースではバックバッファ、エディタでは Game ウィンドウの描画先 |
| エディタカメラ | `EditorCamera::GetCameraView`（Editor 層） | Scene ウィンドウの描画先 |

### Skybox

`DrawSkybox` は頂点バッファ無しの 1 枚の大きな三角形で画面全体を覆い、PS で「平行移動を除いたビュー射影の逆行列」からピクセルごとのワールド方向を復元してパノラマ画像をサンプリングします。逆行列は `b0`（ルートパラメータ 0）に流します。

### ギズモ

`GizmoDrawList` はワールド空間の頂点を積みます。シェーダーへ渡すのは viewProjection だけで済み、形ごとにワールド行列を積み直す必要がありません。形は `AddLine`、`AddRay`、`AddGrid`、`AddWireBox`、`AddWireSphere`、`AddFrustum`、`AddAxes` で足し、色は `GizmoColor` にまとめています。

| 色 | 用途 |
| --- | --- |
| `kCollider`（緑） | コライダーの範囲 |
| `kCamera`（白） | カメラの視錐台 |
| `kLight`（黄） | ライトの向き |
| `kSelected`（青） | 選択中のオブジェクト |
| `kForceField`（水色） | パーティクルの Field の範囲と加速度方向 |
| `kGrid`（灰） | 地面のグリッド |

`DrawGizmos` は `FrameVertexBuffer` に頂点を書き、`Debug/Gizmo` の PSO と LINELIST で 1 回描きます。描いた後はトポロジを三角形へ戻します。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | Asset 層以下にのみ依存する。Scene 層はこの層の上 |
| 実装上の依存 | Graphics（`FrameConstantBuffer`、`FrameVertexBuffer`、`SwapChain`、`DepthBuffer`、`DescriptorHeap`、`PipelineState`、`ShaderDefinition`、`SceneRenderTarget`、`CommandManager`）、Asset（各 Handle と Manager、`Material`、`Mesh`、`Sprite`）、Foundation（Math、`DebugLog`） |
| 規則との食い違い | 各パスと `SceneRenderer` が Scene 層の System を呼ぶ。`ParticlePass` は `Scene`、`GameObject`、`ParticleStorage`、Particle 系コンポーネントを直接 include する |
| 相互参照 | Scene 層の System も Render 層のヘッダ（`CameraView`、`Light.h`、`GizmoDrawList`、`Renderer`）を使う。ヘッダ単位では前方宣言で循環を避けているが、`.cpp` 単位では相互に参照している。詳細は [InternalDesign.md](../InternalDesign.md) を参照 |

## 守るべき規則

### 責務の境界

- `Renderer` は視点として `CameraView` だけを受け取る。`CameraComponent` や `GameObject` には依存しない。
- `SceneRenderer` は視点も描画先も選ばない。呼び出し側が視点を作り、`Renderer::BeginScene` などで描画先を決める。
- 各パスは現在バインドされている描画先へ描き、自分では描画先を保持しない。
- `RenderContext` に DirectX 12 の型（`ID3D12GraphicsCommandList` など）を持ち込まない。低レベル描画は必ず `Renderer` 経由で行う。
- ビュー単位の定数（ライト `b1`、カメラ `b2`）はパス列の前に `Renderer` のバインド関数で設定する。`SceneRenderer::Render` が `SetCamera` を、`LightSetupPass` が `SetLight` を担当する。

### パスの追加と実行順

新しい描画機能は `RenderPass` を作って列に追加します。通常、`SceneRenderer` 本体を変更する必要はありません。実行順には次の制約があります。

- `LightSetup` は先頭に置く。後続のパスが使うライトを準備する。
- `Skybox` は `Opaque` より前に置く。
- `Particle` は半透明描画を想定し、`Opaque` の後に置く。
- `Gizmo` は実体の上に表示するため、最後に置く。

### シーンの状態と描画データ

- `OpaquePass` の前に `Scene::UpdateTransforms` でワールド行列を確定させる。毎フレームの `SceneManager::Update` が担当する。
- パスはシーンの状態を変更しない。同じフレームにゲームビューとシーンビューから実行されるため、粒の更新は `UpdateParticleSystems` が担当する。
- `DrawModel` の `materialOverride` はその描画だけに適用し、モデルの `materialSlots` は変更しない。
- `ParticlePass` のビルボードは `CameraView::rotationMatrix` から作る。粒のワールド行列は「スケール → 画面内回転 → カメラ回転 → 平行移動」の順で、位置だけをエミッターのワールド行列で変換する。インスタンスの色には `Particle::drawColor` を渡す。
- 粒のマテリアルは `requiresInstancing` のシェーダー（ゲーム側の `Particle`）を想定する。非対応シェーダーだと 1 個ずつ `DrawModel` で描かれ、粒ごとの色は反映されない。

### ギズモ

- `options->gizmos` が `nullptr` なら `GizmoPass` は何もしない。ゲームビューでは常に `nullptr` のため、ギズモを表示しない。
- `GizmoDrawList` はメンバとして保持し、毎フレーム `Clear` して再利用する。
- 形状はコライダーの判定形状に合わせる。箱には回転とスケールを適用し、球は中心だけを移動して半径はそのまま使う。
- 頂点はワールド空間で作る。`Gizmo.VS.hlsl` の入力と 32 バイトの `GizmoVertex` のレイアウトを一致させる。

## 未実装・要確認

- `ParticlePass` は不透明の後にエミッター順で描くだけで、粒やエミッター間のソートはありません。
- `Renderer.cpp` の `CameraForGPU` のコメントは「Standard.PS.hlsl の Camera と同じ並びにすること」と書いていますが、`Standard.PS.hlsl` に `Camera` は無く、`b2` を読むのはゲーム側の `Object3d.PS.hlsl` だけです。
- `DrawSprite` と `SetPSO` を呼んでいる箇所はエンジン内にありません。
- `Renderer` の `lastPSO_` と `lastMat_` は PSO キャッシュ用のメンバですが、`DrawGizmos` で `nullptr` に戻す以外に使われていません。
