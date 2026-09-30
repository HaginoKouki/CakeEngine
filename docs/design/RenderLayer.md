# Render層
*画面を描画するための機能*<br>
<br>
描画命令の発行（Renderer）と、シーンを1枚の絵にする工程（SceneRenderer とパス列）を担当します。<br>
<br>
## 場所
`project/Engine/Render/`<br>
<br>
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
- Renderer は視点として CameraView だけを受け取る。CameraComponent も GameObject も知らない。
- SceneRenderer は視点も描画先も持たない。視点の選択と描画先の決定（Renderer::BeginScene など）は呼び出し側が行う。パス列の並びがそのまま絵の順序になり、描画機能の追加は「RenderPass を1つ作って列へ挿す」だけで済ませる（SceneRenderer 自体は書き換えない）。
- パスの順序: LightSetup は必ず先頭（後続が使うライトを用意する）。Skybox は Opaque より前。Gizmo は実体の上に描くため最後。Particle は半透明想定のため Opaque の後。
- パスは「今バインドされている描画先」へ描くだけで、描画先を持たない。
- パスは DX12 の型（ID3D12GraphicsCommandList など）を RenderContext へ持ち込まない。低レベルな描画は必ず Renderer 経由で行う（将来RHIを挟むときの書き換えを増やさないため）。
- ParticlePass は粒の状態を読むだけで変えない。1フレームでゲームビューとシーンビューの2回実行されるため。更新は UpdateParticleSystems が行う。
- GizmoPass は options->gizmos が nullptr のとき何もしない。ゲームビューは常に nullptr なのでギズモは出ない。DrawList はメンバで持ち、毎フレーム Clear して確保を避ける。
- OpaquePass の実行前に Scene::UpdateTransforms でワールド行列が確定していること（SceneManager::Update が毎フレーム呼ぶ）。
- DrawModel の materialOverride は描画時だけの上書きで、モデルの materialSlots を書き換えない。
- ギズモの形はコライダーの判定と揃える。箱は回転もスケールも効かせ、球は中心だけ動かして半径は素のまま使う。
- ギズモの頂点はワールド空間で積む。Gizmo.VS.hlsl の入力と GizmoVertex（32バイト）は一致させる。
