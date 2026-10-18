# Graphics 層

DirectX 12 のデバイス、コマンド、ディスクリプタヒープ、スワップチェーン、シェーダー、PSO をまとめる層です。RHI（Rendering Hardware Interface。描画 API を抽象化する層）に相当します。

## この層が担当すること

Graphics 層は GPU リソースの生成と、シェーダーから PSO を作るまでを担当します。「何を描くか」は知らず、描画命令を発行するのは Render 層の `Renderer` です。

`Graphics.h` は DirectX 12 の型が外へ漏れる唯一の入口です。将来 RHI として抽象化する際は、ここの getter を差し替えて上位層を守る想定です。

## 配置

`project/Engine/Graphics/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| 直下 | `Graphics` | 下記の部品をすべて所有し、`Initialize` と `Resize` を提供する |
| `Device/` | `DirectXDevice` | Factory、Adapter、Device の生成。Debug 構成ではデバッグレイヤーを有効にする |
| `Device/` | `CommandManager` | Queue、Allocator、CommandList、Fence。`ExecuteAndWait` と `WaitForGPU` |
| `Descriptor/` | `DescriptorHeap` | ディスクリプタヒープ 1 本と、先頭の予約枠（`Reserved` 名前空間） |
| `RenderTarget/` | `SwapChain` | バックバッファ 2 枚。フレーム先頭と末尾のバリア、Present、Resize |
| `RenderTarget/` | `DepthBuffer` | `D24_UNORM_S8_UINT` の深度バッファ 1 枚 |
| `RenderTarget/` | `SceneRenderTarget` | オフスクリーン描画先（RTV と SRV）。エディタの 2 ビューが使う。遅延リサイズに対応 |
| `Pipeline/` | `PipelineState`、`PSODesc`、`BlendMode`、`VertexLayoutType` | ルートシグネチャ 1 つと、名前をキーにした PSO のマップ |
| `Shader/` | `ShaderCompiler` | DXC による HLSL のコンパイルとリフレクションの取得 |
| `Shader/` | `ShaderDefinition`、`ShaderParamDesc`、`TextureSlotDesc` | シェーダー 1 つ分の定義（パス、cbuffer のパラメータ、テクスチャスロット、PSO 設定） |
| `Shader/` | `ShaderLibrary` | 定義の登録と一元管理。登録時にレイアウト計算と PSO 生成まで行う |
| `Buffer/` | `FrameConstantBuffer`、`FrameVertexBuffer` | フレーム内で使い捨てるバンプアロケータ（定数用 1 MiB、頂点用 4 MiB） |
| `Buffer/` | `CreateBufferResource` | Upload ヒープにバッファを 1 つ作る下請け |
| `Profiler/` | `PerformanceProfiler` | GPU タイムスタンプ、CPU 時間、システムの CPU と GPU 使用率。`USE_IMGUI` 時は HUD を描く |

## 主要な仕組み

### 初期化の順序

`Graphics::Initialize` は次の順に部品を作ります。

1. `DirectXDevice`
2. `CommandManager`
3. `DescriptorHeap` を 3 本（RTV 4 個、SRV 128 個でシェーダー可視、DSV 1 個）
4. `SwapChain`
5. `ShaderCompiler`
6. `PipelineState`（ルートシグネチャの構築）と `ShaderLibrary`（既定シェーダーの登録と PSO 生成）
7. `DepthBuffer`
8. `PerformanceProfiler`

### ルートシグネチャ

ルートシグネチャは 1 つだけで、すべてのシェーダーが共有します。番号は `PipelineState` の定数で参照します。

| 番号 | 内容 | 定数 |
| --- | --- | --- |
| 0 | PS `b0`。マテリアルの定数バッファ | |
| 1 | VS `b0`。変換行列（`TransformationMatrix`） | |
| 2 | PS `b1`。平行光源（`DirectionalLight`） | |
| 3 から 6 | PS `t0` から `t3`。テクスチャスロット（1 枚ごとに独立したテーブル） | `kTextureRootParamStart`、`kMaxTextureSlots` = 4 |
| 7 | VS `t0`。インスタンシング用の `StructuredBuffer`（ルート SRV） | `kInstancingRootParam` |
| 8 | PS `b2`。カメラ（`CameraForGPU`） | `kCameraRootParam` |

`b2` のレイアウトは `Renderer.cpp` の `CameraForGPU`（`float3 worldPosition` と `float` のパディングで 16 バイト）です。シェーダー側で `b2` を宣言するときは、この並びに合わせます。現状 `b2` を読んでいるのはゲーム側の `Object3D` シェーダーだけで、エンジンの `Standard` は宣言していません。

### シェーダー定義と PSO

`ShaderLibrary::Register` に `ShaderDefinition` を渡すと、次の処理をまとめて行います。

1. `ComputeLayout` で cbuffer のパラメータのオフセットを HLSL のパッキング規則で計算する。
2. `PipelineState::AddPSO` で VS と PS をコンパイルし、`PSODesc` の設定（頂点レイアウト、ブレンド、深度、カリング、塗り）で PSO を作る。
3. PS のリフレクションが取れた場合は、`ApplyReflection` で実際のレイアウトに上書きする。

`ShaderDefinition::name` は PSO の名前も兼ねます。マテリアルはこの定義を参照して cbuffer を確保し、`Material::Apply` で GPU へ反映します。

エンジンが `RegisterDefaultShaders` で登録する既定シェーダーは次のとおりです。`EngineResources/shaders/` にあります。

| 名前 | 用途 | パラメータ | テクスチャ |
| --- | --- | --- | --- |
| `__error_shader__` | 読み込み失敗時のエラー表示（マゼンタ） | `color` | |
| `Standard` | 既定のモデル描画。`lightingType` が 1 で Half Lambert、2 で Lambert、それ以外で無灯 | `color`、`tiling`、`offset`、`lightingType` | `albedo`（t0） |
| `Unlit/Color` | ライティング無しの単色 | `color` | |
| `Unlit/WireFrame` | ワイヤーフレーム | `color` | |
| `Skybox/Panoramic` | パノラマ画像の Skybox | | `Spherical`（t0） |
| `Sprite` | 2D スプライト | `color`、`uvRect` | `main`（t0） |
| `Debug/Gizmo` | ギズモの線。エディタのマテリアル選択肢には出さない | | |

ゲーム固有のシェーダー（`Object3D`、`Particle`）は `Game::RegisterGameShaders` から同じ `Register` で登録します。手順は [guides/AddingShaders.md](../guides/AddingShaders.md) にあります。

### ディスクリプタヒープの予約枠

SRV ヒープと RTV ヒープの先頭は、エディタ固有の描画先のために予約しています（`Reserved` 名前空間）。

| 構成 | SRV | RTV |
| --- | --- | --- |
| `ENABLE_EDITOR` あり | 0 = ImGui フォント、1 = SceneRT、2 = DebugSceneRT。`kSrvReservedCount` = 3 | 0 と 1 = SwapChain、2 = SceneRT、3 = DebugSceneRT |
| `ENABLE_EDITOR` なし | `kSrvReservedCount` = 0 | 0 と 1 = SwapChain |

`TextureManager` のように動的に確保する側は、必ず `kSrvReservedCount` から始めます。

### フレーム内の使い捨てバッファ

`FrameConstantBuffer` と `FrameVertexBuffer` は、フレーム先頭に `Reset` するだけで再利用できる線形アロケータです。`Renderer::EndFrame` が毎フレーム `ExecuteAndWait` で GPU を待ち切るため、多重バッファリングを持ちません。

| バッファ | 用途 | 枯渇時の動き |
| --- | --- | --- |
| `FrameConstantBuffer`（1 MiB） | モデルごとの変換行列、パーティクルのインスタンスデータ、カメラ | `offset` を先頭へ戻して続行する。描画は乱れるが落ちない。ただし 1 回の `Allocate` が容量を超える場合は検出しない |
| `FrameVertexBuffer`（4 MiB） | ギズモの線分 | `SizeInBytes` が 0 の VBV を返す。呼び出し側はそれを見て描画を諦める |

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | 下は Platform と Foundation のみ。上位層（Asset、Render、Scene、Editor）を知らない |
| 実装上の依存 | Foundation（`DebugLog` など）、Platform（`PerformanceProfiler` → `Time`） |
| 外部 | DirectX 12、DXC、PDH、ImGui（`PerformanceProfiler::DrawHud`。`USE_IMGUI` 時のみ） |
| 規則との食い違い | `RenderTarget/SwapChain.h` が Application 層の `BuildConfig.h` を include している（`ENABLE_EDITOR` で RTV フォーマットを切り替えるため。エディタは `UNORM`、リリースは `UNORM_SRGB`） |
| 規則との食い違い | `Buffer/GPUResourceUtility.cpp` が Asset 層の `Material.h` を include しているが、使っていない |
| 規則との食い違い | `DescriptorHeap.h` の予約枠と `SwapChain.h` のフォーマットに `ENABLE_EDITOR` の分岐がある |

## 守るべき規則

### GPU 同期と一時バッファ

- `Renderer::EndFrame` が毎フレーム `ExecuteAndWait` で GPU を待ち切る。この前提で `FrameConstantBuffer`、`FrameVertexBuffer`、`PerformanceProfiler` は多重バッファリングをしない。非同期にする場合は 3 つを作り直す。
- `FrameConstantBuffer` は CBV 用に 256 バイト境界で確保する。

### リサイズ

- `Graphics::Resize` と `ResizeDepthBuffer` は先に `WaitForGPU` する。`Renderer::Resize` も併せて呼ぶ（ビューポートとシザーは `Renderer` の担当）。
- `SceneRenderTarget` は遅延リサイズ（`RequestResize` → 次フレーム先頭で `ConsumePendingResize`）で、フレーム途中に描画先を作り直さない。

### シェーダーと頂点データ

- ルートシグネチャは 1 つを全シェーダーで共有する。番号の割り当ては上の表のとおり。
- `requiresInstancing` のシェーダーは `Renderer::DrawModel` では描かず、エラーマテリアルへ差し替える。VS が未設定のルート SRV を読んでデバイス削除になるのを防ぐため。
- 頂点レイアウトは `None`（頂点バッファ無し）、`Mesh`（POSITION、TEXCOORD、NORMAL）、`Line`（POSITION、COLOR）の 3 種。
- ブレンド（`PipelineState::GetBlendDesc`）は、色の式はモードごとに異なり、透明度の列は全モード共通で「重ね塗り」（`ONE` と `INV_SRC_ALPHA`）にする。`kBlendModeNone` だけはブレンド自体を無効にする。
- HLSL 側の構造体と C++ 側の構造体は 1 バイトも違ってはいけない。`GizmoVertex`（32 バイト）、`SpriteTransform`（32 バイト）、`CameraForGPU`（16 バイト）は `static_assert` で固定している。

## 未実装・要確認

- `GetBlendDesc` の `kBlendModeMultiply` に `break` が無く、`kBlendModeScreen` へフォールスルーします。現状 Multiply を使うシェーダーは無いため見えていません。意図的かどうかは要確認です（[レビューの運用](../review/README.md) を参照）。
- `ShaderCompiler::Compile` はコンパイル失敗時に Debug 構成では `assert` で止まり、Release 構成では空のブロブを返します。HLSL は実行時にコンパイルされるため、CI ではシェーダーの誤りを検出できません。
