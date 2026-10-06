# Render層
*画面を描画するための機能*



描画命令の発行（Renderer）と、シーンを1枚の絵にする工程（SceneRenderer とパス列）を担当します。



## 場所
`project/Engine/Render/`



## 構成
- Renderer … GPU描画命令の発行と描画状態の管理。BeginFrame / EndFrame（EndFrameで ExecuteAndWait と Present）、BeginScene / EndScene（オフスクリーンRT）、PrepareBackbufferToEditor / PrepareBackbufferToGame（16:9レターボックス）、DrawModel、DrawModelInstanced（Particle用）、DrawSprite、DrawSkybox、DrawGizmos。PSOとマテリアルの直前値をキャッシュして切替を減らす。
- LightManager / Light.h … DirectionalLight の定数バッファを持つ。値を書くだけで、バインドは Renderer が行う。
- CameraView … 1回の描画で使う視点（view / projection / rotation / position）。MakeCameraView で world 行列と射影パラメータと出力解像度から作る。ゲームカメラもエディタカメラも同じ経路。
- SceneRenderer … パス列を保持して順に Execute する。Initialize で LightSetup → Skybox → Opaque → Particle → Gizmo の順に組み立てる。
- Pass/ … RenderPass（基底。RenderContext = scene / view / renderer / options を受け取る）、LightSetupPass、SkyboxPass、OpaquePass、ParticlePass、GizmoPass。
- Gizmo/GizmoDrawList … ギズモを線分へ分解して1本の LINELIST 頂点列に貯める入れ物。形は Add～ で足し、色は GizmoColor に集約。

## 依存関係
規則（Overview.mdの階層）:
- 自分以下の層にのみ依存する。Scene層はこの層の上。

現状の実装:
- Graphics … FrameConstantBuffer、FrameVertexBuffer、SwapChain、DepthBuffer、DescriptorHeap、PipelineState、ShaderDefinition、SceneRenderTarget、CommandManager。
- Asset … MaterialHandle、ModelHandle、TextureManager、MaterialManager、ModelManager、Material、Mesh、Sprite。
- Foundation … Math、DebugLog。

規則との食い違い（現状の実装）:
- Scene層への依存: パスが Scene の System を呼んでいる。OpaquePass → Scene/System/RenderSystem.h（DrawMeshRenderers）、LightSetupPass → LightSystem.h（CollectDirectionalLight）、GizmoPass → GizmoSystem.h（CollectSceneGizmos）、SceneRenderer.cpp → CameraSystem.h（CollectMainCamera）、ParticlePass → Scene.h / GameObject.h / ParticleStorage.h / ParticleSystemComponent.h / ParticleRendererComponent.h。
- 逆方向にも依存があり、Scene層も Render層をincludeしている（Render/CameraView.h、Render/Light.h、Render/Gizmo/GizmoDrawList.h、Render/Renderer.h）。つまり Render ⇄ Scene が循環している。
- RenderPass.h は Scene と GizmoSettings を前方宣言で受けるため、ヘッダ単位の循環は避けているが、.cpp 単位では相互に参照している。

## 規則

### 責務の境界

- `Renderer` は視点として `CameraView` だけを受け取る。`CameraComponent` や `GameObject` には依存しない。
- `SceneRenderer` は視点や描画先を選ばない。呼び出し側が視点を選び、`Renderer::BeginScene` などで描画先を決める。
- 各パスは現在バインドされている描画先へ描き、自分では描画先を保持しない。
- `RenderContext` に DirectX 12 の型（例: `ID3D12GraphicsCommandList`）を持ち込まない。低レベル描画も必ず `Renderer` 経由で行う。

### パスの追加と実行順

`SceneRenderer` はパス列を順番に実行します。新しい描画機能は `RenderPass` を作って列に追加します。通常、機能追加のために `SceneRenderer` 本体を変更する必要はありません。

実行順には次の制約があります。

- `LightSetup` は先頭に置く。後続のパスが使うライトを準備する。
- `Skybox` は `Opaque` より前に置く。
- `Particle` は半透明描画を想定し、`Opaque` の後に置く。
- `Gizmo` は実体の上に表示するため、最後に置く。

### シーン状態と描画データ

- `OpaquePass` の前に `Scene::UpdateTransforms` を実行し、ワールド行列を確定させる。これは毎フレームの `SceneManager::Update` が担当する。
- `ParticlePass` は粒の状態を読むだけで、変更しない。同じフレームにゲームビューとシーンビューから実行されるため、更新は `UpdateParticleSystems` が担当する。
- `DrawModel` の `materialOverride` はその描画だけに適用する。モデルの `materialSlots` は変更しない。

### ギズモ

- `options->gizmos` が `nullptr` なら `GizmoPass` は何もしない。ゲームビューでは常に `nullptr` のため、ギズモを表示しない。
- `DrawList` はメンバとして保持し、毎フレーム `Clear` して再利用する。
- 形状はコライダーの判定形状に合わせる。箱には回転とスケールを適用し、球は中心だけを移動して半径はそのまま使う。
- 頂点はワールド空間で作る。`Gizmo.VS.hlsl` の入力と、32バイトの `GizmoVertex` のレイアウトを一致させる。
