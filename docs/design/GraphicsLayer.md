# Graphics層
*DirectX12をまとめる*<br>
<br>
DirectX12のデバイス・コマンド・ディスクリプタヒープ・スワップチェイン・シェーダー・PSOをまとめる層（RHI相当）です。<br>
<br>
## 場所
`project/Engine/Graphics/`<br>
<br>
## 構成
Graphics（Graphics.h/.cpp）が下記を所有します。DX12の型が外へ漏れる唯一の入口で、将来RHIとして抽象化する際は、ここのgetterを差し替えて上位層を守る想定です。<br>
<br>
Graphics::Initialize の初期化順:
- 1. DirectXDevice → 2. CommandManager → 3. DescriptorHeap ×3（RTV 4個 / SRV 128個・シェーダー可視 / DSV 1個）→ 4. SwapChain → 5. ShaderCompiler → 6. PipelineState → ShaderLibrary（既定シェーダー登録とPSO生成）→ 7. DepthBuffer → PerformanceProfiler

フォルダ:
- Device/ … DirectXDevice（Factory / Adapter / Device、デバッグレイヤー）、CommandManager（Queue / Allocator / CommandList / Fence。ExecuteAndWait、WaitForGPU）。
- Descriptor/ … DescriptorHeap。先頭を Reserved 名前空間で予約（ENABLE_EDITOR 時は SRV 0=ImGui、1=SceneRT、2=DebugSceneRT。RTV 0,1=SwapChain、2,3=エディタ用RT）。動的に確保する側は kSrvReservedCount から始める。
- RenderTarget/ … SwapChain（2枚）、DepthBuffer（D24_S8）、SceneRenderTarget（オフスクリーン描画先。遅延リサイズ対応）。
- Pipeline/ … PipelineState（ルートシグネチャ1つとPSOのマップ。BlendMode、VertexLayoutType、PSODesc）。
- Shader/ … ShaderCompiler（DXC。リフレクション取得）、ShaderDefinition（パス、パラメータスキーマ、PSO設定）、ShaderLibrary（定義の一元管理）、ShaderParam。
- Buffer/ … FrameConstantBuffer / FrameVertexBuffer（フレーム内使い捨てのバンプアロケータ）、GPUResourceUtility（CreateBufferResource）。
- Profiler/ … PerformanceProfiler（GPUタイムスタンプ、CPU時間、システムCPU/GPU使用率）。

既定シェーダー（ShaderLibrary::RegisterDefaultShaders）: `__error_shader__`、`Standard`（既定）、`Unlit/Color`、`Unlit/WireFrame`、`Skybox/Panoramic`、`Sprite`、`Debug/Gizmo`（エディタ非表示）。ゲーム固有のシェーダーは Game::RegisterGameShaders から Register する。<br>
<br>
## 依存関係
規則（Graphics.hのコメント）:
- 上位層（Asset / Render / Scene / Editor）を一切知らない。下は Platform と Foundation のみ。

注意: Overview.md では Graphics層が Asset層の上に記載されており、Asset.h のコメントは「AssetがGraphicsに依存」と書いています。実装は後者（Asset → Graphics）です。どちらを正とするか要判断（AssetLayer.md にも同じ記載があります）。

現状の実装:
- Foundation … DebugLog など。
- Platform … Profiler/PerformanceProfiler.h → Time/Time.h。
- 外部 … DirectX12、DXC、PDH、ImGui（PerformanceProfiler::DrawHud。USE_IMGUI 時のみ）。

規則との食い違い（現状の実装）:
- Application層 … RenderTarget/SwapChain.h が Application/BuildConfig.h をincludeしている（ENABLE_EDITOR の判定のため）。
- Asset層 … Buffer/GPUResourceUtility.cpp が Material.h をincludeしているが、ファイル内で使っていない（不要なinclude）。
- ENABLE_EDITOR 分岐が Graphics 内にある: DescriptorHeap.h の予約枠、SwapChain.h の RTV フォーマット（エディタは UNORM、リリースは UNORM_SRGB）。

## 規則
- 同期設計: Renderer::EndFrame が毎フレーム ExecuteAndWait で GPU を待ち切る。このため FrameConstantBuffer / FrameVertexBuffer / PerformanceProfiler は多重バッファリングをしない。これを非同期にする場合は上記3つを作り直すこと。
- FrameConstantBuffer は CBV 用に256バイト境界で確保する。枯渇時は offset を先頭へ戻して落とさない（描画は乱れる）。FrameVertexBuffer は枯渇時に SizeInBytes = 0 の VBV を返す。呼び出し側はそれを見て描画を諦める。
- リサイズ: Graphics::Resize / ResizeDepthBuffer は先に WaitForGPU する。Renderer::Resize も併せて呼ぶこと（ビューポートとシザーは Renderer の担当）。SceneRenderTarget は遅延リサイズ（RequestResize → 次フレーム頭で ConsumePendingResize）で、フレーム途中でRTを作り直さない。
- ルートシグネチャは1つを全シェーダーで共有する。0=PS b0（マテリアル）、1=VS b0（変換）、2=PS b1（ライト）、3〜6=PS テクスチャ t0〜t3（kMaxTextureSlots=4）、7=VS t0（インスタンシング用 StructuredBuffer）。
- ShaderDefinition::ComputeLayout でcbufferのHLSLパッキングを計算する。リフレクションが取れる場合は実レイアウトで上書きする（ApplyReflection）。
- requiresInstancing のシェーダーは Renderer::DrawModel では描かず、エラーマテリアルへ差し替える（Renderer 側の規則）。
- 頂点レイアウトは None / Mesh（POSITION, TEXCOORD, NORMAL）/ Line（POSITION, COLOR）の3種。
- HLSL側の構造体と C++ 側は1バイトも違ってはいけない（GizmoVertex、SpriteTransform など。static_assert で固定）。
