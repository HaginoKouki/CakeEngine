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
| 直下 | `Graphics` | 下記の部品をすべて所有し、`Initialize`、`Resize`、`BeginFrame`、`EndFrame` を提供する |
| `Device/` | `DirectXDevice` | Factory、Adapter（高性能優先でソフトウェアアダプタを除外）、Device の生成。Debug 構成ではデバッグレイヤーと GPU ベース検証を有効にし、エラーと警告で停止する |
| `Device/` | `CommandManager` | Queue、Allocator、CommandList、Fence。`ExecuteAndWait` と `WaitForGPU`。最後に送ったフェンス値と完了済みのフェンス値を返す |
| `Device/` | `DeferredReleaseQueue` | GPU が使い終わるまで解放を遅らせる。メインスレッド専用 |
| `Descriptor/` | `DescriptorHeap` | ディスクリプタヒープ 1 本と、RTV ヒープ先頭の予約枠（`Reserved` 名前空間） |
| `Descriptor/` | `SrvManager` | シェーダー可視の SRV ヒープ（4096 個）を 1 本持ち、`Allocate` で番号を払い出し、`Free` で返された番号を使い回す |
| `Descriptor/` | `IndexFreeList` | 番号の空きリストと、使用数・最大使用数。D3D12 を知らないので、Tests から GPU なしで確かめられる |
| `RenderTarget/` | `SwapChain` | バックバッファ 2 枚。フレーム先頭と末尾のバリア、Present、Resize |
| `RenderTarget/` | `DepthBuffer` | `D24_UNORM_S8_UINT` の深度バッファ 1 枚 |
| `RenderTarget/` | `SceneRenderTarget` | オフスクリーン描画先（RTV と SRV）。エディタの 2 ビューが使う。遅延リサイズに対応 |
| `Pipeline/` | `PipelineState`、`PSODesc`、`BlendMode`、`VertexLayoutType` | ルートシグネチャ 1 つと、名前をキーにした PSO のマップ |
| `Shader/` | `ShaderCompiler` | DXC による HLSL のコンパイル（`-Zi -Od -Zpr`）とリフレクションの取得 |
| `Shader/` | `ShaderDefinition`、`ShaderParamDesc`、`TextureSlotDesc` | シェーダー 1 つ分の定義（パス、cbuffer のパラメータ、テクスチャスロット、PSO 設定） |
| `Shader/` | `ShaderLibrary` | 定義の登録と一元管理。登録時にレイアウト計算と PSO 生成まで行う |
| `Buffer/` | `FrameConstantBuffer`、`FrameVertexBuffer` | フレーム内で使い捨てるバンプアロケータ（定数用 1 MiB、頂点用 4 MiB） |
| `Buffer/` | `CreateBufferResource` | Upload ヒープにバッファを 1 つ作る下請け |
| `Upload/` | `UploadQueue`、`UploadRequest` | GPU への転送を予約で受け付け、フレームの先頭で上限までコピーの命令を積む。完了処理は GPU が転送を終えた次のフレームの先頭で呼ぶ。メインスレッド専用 |
| `Upload/` | `kUploadBytesPerFrame`、`CountUploadsThisFrame` | 1 フレームの転送量の上限（仮の値 16 MiB）と、何件を転送するかの計算。D3D12 を知らないので、Tests から確かめられる |
| `Profiler/` | `PerformanceProfiler` | GPU タイムスタンプ、CPU 時間、システムの CPU と GPU 使用率（PDH）。`USE_IMGUI` 時は HUD を描く |

## 主要な仕組み

### 初期化の順序

`Graphics::Initialize` は次の順に部品を作ります。

1. `DirectXDevice`
2. `CommandManager`
3. `DescriptorHeap` を 2 本（RTV 4 個、DSV 1 個）と `SrvManager`（SRV 4096 個でシェーダー可視）
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

静的サンプラーは `s0` の 1 つ（線形フィルタ、WRAP）です。

`b2` のレイアウトは `Renderer.cpp` の `CameraForGPU`（`float3 worldPosition` と `float` のパディングで 16 バイト）です。シェーダー側で `b2` を宣言するときは、この並びに合わせます。現状 `b2` を読んでいるのはゲーム側の `Object3D` シェーダーだけで、エンジンの `Standard` は宣言していません。

### シェーダー定義と PSO

`ShaderLibrary::Register` に `ShaderDefinition` を渡すと、次の処理をまとめて行います。

1. `PipelineState::AddPSO` で VS と PS をコンパイルし、`PSODesc` の設定（頂点レイアウト、ブレンド、深度、カリング、塗り）で PSO を作る。
2. PS のリフレクションが取れた場合は、`ApplyReflection` で `b0` の cbuffer の実際のレイアウト（ネストした構造体も展開）を `params` のオフセットへ焼き込む。取れない場合は `ComputeLayout` で HLSL のパッキング規則から手計算する。

`ShaderDefinition::name` は PSO の名前も兼ねます。マテリアルはこの定義を参照して cbuffer を確保し、`Material::Apply` で GPU へ反映します。`Find` で見つからない名前は `__error_shader__` の定義を返します。

エンジンが `RegisterDefaultShaders` で登録する既定シェーダーは次のとおりです。`EngineResources/shaders/` にあります。

| 名前 | 用途 | パラメータ | テクスチャ |
| --- | --- | --- | --- |
| `__error_shader__` | 読み込み失敗時のエラー表示（マゼンタ） | `color` | |
| `Standard` | 既定のモデル描画<br>`lightingType` が 1 で Half Lambert、2 で Lambert、それ以外で無灯 | `color`、`tiling`、`offset`、`lightingType` | `albedo`（t0） |
| `Unlit/Color` | ライティング無しの単色 | `color` | |
| `Unlit/WireFrame` | ワイヤーフレーム | `color` | |
| `Skybox/Panoramic` | パノラマ画像の Skybox。<br>頂点バッファ無し、深度書き込み無し | | `Spherical`（t0） |
| `Sprite` | 2D スプライト<br>頂点バッファなし、アルファブレンド、深度テスト・書き込みなし、カリングなし<br>色と切り出し範囲は、マテリアルではなく VS の `b0`（`SpriteForGPU`）で受け取る<br>エディタのマテリアル選択肢には出さない |  | `main`（t0） |
| `Debug/Gizmo` | ギズモの線<br>LINE トポロジ、深度書き込み無し<br>エディタのマテリアル選択肢には出さない | | |

ゲーム固有のシェーダー（`Object3D`、`Particle`）は `Game::RegisterGameShaders` から同じ `Register` で登録します。

### RTV の予約枠と SRV の払い出し

RTV ヒープの先頭は、エディタ固有の描画先のために予約しています（`Reserved` 名前空間）。

| 構成 | RTV |
| --- | --- |
| `ENABLE_EDITOR` あり | 0 と 1 = SwapChain、2 = SceneRT、3 = DebugSceneRT |
| `ENABLE_EDITOR` なし | 0 と 1 = SwapChain |

SRV は予約せず、`SrvManager::Allocate` が番号を払い出します。空きリスト（`IndexFreeList`）に返された番号があればそれを先に使い、なければまだ使っていない番号を出します。番号は初期化の順番で決まり、Debug と Release でも変わるので、各クラスは受け取った番号を保存して使います。作り直す SRV（リサイズする `SceneRenderTarget` など）は、最初に受け取った番号へ作り直します。

番号を返すときは `SrvManager::Free` を `DeferredReleaseQueue` に積み、GPU がその番号を使い終わってから空きリストへ戻します。ただし、まだ SRV を書いていない番号（GPU に渡していない番号）は、その場で `Free` してかまいません。

上限（`kMaxSrvCount` = 4096。手順9で計測して見直す仮の値）を使い切ると、`Allocate` は Release でも `kInvalidIndex` を返し、エラーのログを出します。二重解放と、払い出していない番号の解放も、エラーのログを出します。

| 使う側 | 確保する数 | 確保する時点 |
| --- | --- | --- |
| `TextureManager` | テクスチャ 1 枚につき 2（描画用と、`ENABLE_EDITOR` のときだけプレビュー用）。取れなければ Error テクスチャで代替する | テクスチャの読み込み時 |
| `ImGuiManager` | 予備の 1 と、バックエンドがテクスチャを作るたびに 1 | 初期化時と、ImGui のテクスチャの作成時 |
| `SceneRenderTarget` | 1 枚につき 1 | 初期化時（リサイズでは確保しない） |

ImGui 1.92 以降のバックエンドは、テクスチャごとに SRV を確保します。`ImGuiManager` は確保のコールバック（`SrvDescriptorAllocFn`）から `SrvManager::Allocate` を直接呼び、解放のコールバック（`SrvDescriptorFreeFn`）では CPU ハンドルを `SrvManager::GetIndexFromCPUHandle` で番号に戻して、`Free` を `DeferredReleaseQueue` に積みます。積むラムダは、`SrvManager` のポインタと番号を値でキャプチャします（終了時は `ImGuiManager` が先に破棄され、そのあとで `~Graphics` がラムダを呼ぶため）。番号が尽きたときは、初期化時に確保した予備の番号を渡します。予備の番号は `Free` しません。

### 遅延解放

`DeferredReleaseQueue` は、GPU が使い終わるまで解放を遅らせます。そのフレームに積まれた予約を、`Graphics::EndFrame`（`Renderer::EndFrame` の `ExecuteAndWait` の後）で、最後に送ったフェンス値で締めます。次のフレームの `Graphics::BeginFrame` で、完了済みのフェンス値までの予約を解放します。

フェンス値は 1 フレームに 1 回進むとは限らない前提で書いています（以前は `TextureManager` がフレームの途中で `ExecuteAndWait` していました。今は `UploadQueue` に移りましたが、ほかの部品が同じことをしても壊れないようにしています）。フレームの途中の値で締めると早すぎる解放になるので、必ずフレームの終わりで締めます。終了時は、`~Graphics` が GPU の完了を待ってから、残りをすべて解放します。`deferredReleaseQueue_` は `srvManager_` より後に宣言しているので、`srvManager_` より先に破棄されます。

### GPU への転送

`UploadQueue` は、「このリソースへこのデータを転送する」という予約（`UploadRequest`）を受け付けます。予約には、転送先のリソース（`COPY_DEST` の状態で作っておく）、転送するデータの場所（`D3D12_SUBRESOURCE_DATA` の並び）、データの持ち主（`std::shared_ptr<const void>`）、転送後の状態、完了処理を入れます。

1 フレームの流れは次のとおりです。

1. `Graphics::BeginFrame`：`ProcessCompleted` で、GPU が転送を終えた予約の完了処理を呼ぶ。次に `Record` で、予約を前から取り出し、上限（`kUploadBytesPerFrame`）まで、一時バッファの作成、`UpdateSubresources`、転送後の状態へのバリアをコマンドリストの先頭に積む
2. `Renderer::EndFrame` の `ExecuteAndWait` で、GPU がコピーを実行する
3. `Graphics::EndFrame`：このフレームで積んだ完了処理に、最後のフェンス値を付ける

上限は、1 件目だけは超えても取ります（0 件だと先へ進めないため）。予約の順番は入れ替えません。同じリソースへの転送が 2 回あったとき、古いデータが後から書かれないようにするためです。

データの持ち主は、`Record` で `UpdateSubresources` が一時バッファへ写した時点で手放します。一時バッファは、GPU がコピーを終えるまで `DeferredReleaseQueue` に預けます。一時バッファは予約 1 件ごとに作っています。作るのが重いとわかったら、使い回す形に中身だけを差し替えます。

終了時は、残った予約と完了処理を実行せずに捨てます。完了処理は `TextureManager` などの中を触りますが、終了時には Asset が `Graphics` より先に破棄されているためです。

### フレーム内の使い捨てバッファ

`FrameConstantBuffer` と `FrameVertexBuffer` は、フレーム先頭に `Reset` するだけで再利用できる線形アロケータです。`Renderer::EndFrame` が毎フレーム `ExecuteAndWait` で GPU を待ち切るため、多重バッファリングを持ちません。

| バッファ | 用途 | 枯渇時の動き |
| --- | --- | --- |
| `FrameConstantBuffer`（1 MiB） | モデルごとの変換行列、パーティクルのインスタンスデータ、カメラ、スプライト 1 枚ごとの `SpriteForGPU` | `offset` を先頭へ戻して続行する。描画は乱れるが落ちない。ただし 1 回の `Allocate` が容量を超える場合は検出しない |
| `FrameVertexBuffer`（4 MiB） | ギズモの線分 | `SizeInBytes` が 0 の VBV を返す。呼び出し側はそれを見て描画を諦める |

### プロファイラ

`PerformanceProfiler` は `BeginFrame` と `EndFrame` で GPU タイムスタンプを 2 点打ち、`ExecuteAndWait` の後に `Resolve` で読み戻します。同期設計なので同じフレーム内で確実に結果が取れ、多重バッファリングが要りません。システムの CPU 使用率は `GetSystemTimes` の差分、GPU 使用率は PDH の `GPU Engine(*)\Utilization Percentage` の最大値で、1 秒おきに更新します。

## シェーダーを追加する

新しいシェーダーを登録し、マテリアルから選べるようにするまでの手順です。

### 1. HLSL を書く

| 置き場所 | 用途 |
| --- | --- |
| `project/EngineResources/shaders/<名前>/` | エンジンの既定シェーダー |
| `project/Assets/Shaders/<名前>/` | ゲーム固有のシェーダー |

ルートシグネチャは 1 つを全シェーダーで共有するため、レジスタの割り当ては固定です。

| レジスタ | ステージ | 内容 |
| --- | --- | --- |
| `b0` | PS | マテリアルのパラメータ<br>`ShaderDefinition::params` の並びと一致させる |
| `b0` | VS | 変換行列<br>`TransformationMatrix`（`WVP`、`World`、`WorldInverseTranspose` の `float4x4` が 3 つ）。エンジンの `Standard` は先頭の 2 つだけを宣言し、3 つ目はゲーム側の `Object3D` が法線行列として読む<br>`Sprite` は `SpriteForGPU` |
| `b1` | PS | 平行光源。`DirectionalLight`（`float4 color`、`float3 direction`、`float intensity`） |
| `b2` | PS | カメラ。`float3 worldPosition` と `float` のパディング |
| `t0` から `t3` | PS | テクスチャスロット。`TextureSlotDesc::registerIndex` と一致させる |
| `t0` | VS | インスタンシング用の `StructuredBuffer<ParticleForGPU>`。`requiresInstancing` のシェーダーのみ |
| `s0` | PS | サンプラー |

頂点入力は `VertexLayoutType` に合わせます。`Mesh` は `POSITION0`（`float4`）、`TEXCOORD0`（`float2`）、`NORMAL0`（`float3`）、`Line` は `POSITION0`（`float4`）、`COLOR0`（`float4`）です。

HLSL ファイルには日本語を書きません。HLSL は起動時に DXC でコンパイルされ、CI では検査されません。コンパイルエラーがあると Debug 構成では `ShaderCompiler.cpp` の `assert` で止まります。

### 2. 定義を登録する

ゲーム側は `project/Game/GameModule.cpp` の `Game::RegisterGameShaders` に書きます。`Object3D` の登録が例です。

```cpp
Cake::ShaderDefinition object3d;
object3d.name = "Object3D";
object3d.vsPath = L"Assets/Shaders/Object3D/Object3d.VS.hlsl";
object3d.psPath = L"Assets/Shaders/Object3D/Object3d.PS.hlsl";
object3d.params = {
	Cake::ShaderParamDesc{
		.name = "color",
		.type = Cake::ShaderParamType::Color,
		.defaultValue = {1, 1, 1, 1},
		.uiMin = 0.0f,
		.uiMax = 1.0f
	},
	// ...
};
object3d.textures = {
	Cake::TextureSlotDesc{
		.name = "albedo",
		.registerIndex = 0
	}, // t0.
};
shaderLibrary.Register(object3d);
```

| 項目 | 内容 |
| --- | --- |
| `name` | シェーダー名。PSO の名前とマテリアルの選択肢になる。`Unlit/Color` のようにスラッシュで階層を付けられる |
| `vsPath`、`psPath` | `project/` からの相対パス（ワイド文字列） |
| `params` | cbuffer `b0` に並ぶパラメータ。`name` は HLSL の変数名と一致させる。型は `Float`、`Float2`、`Float3`、`Float4`、`Color`、`Int`。`uiMin` と `uiMax` が等しいと UI から触れなくなる。`Int` で 0 から 1 ならチェックボックスになる |
| `textures` | テクスチャスロット。`name` はマテリアル側で指定する名前、`registerIndex` は `t` のレジスタ番号（0 から 3） |
| `blendMode`、`depthTest`、`depthWrite`、`cullMode`、`fillMode` | PSO の設定。既定はブレンド無し、深度テストと書き込みあり、背面カリング、塗りつぶし |
| `vertexLayout`、`topologyType` | 頂点入力とトポロジ。既定は `Mesh` と三角形 |
| `requiresInstancing` | true にすると `Renderer::DrawModelInstanced` 専用になる。通常の `DrawModel` ではエラーマテリアルに差し替えられる |
| `hiddenInEditor` | true にするとマテリアルの選択肢に出さない（`Debug/Gizmo` など） |

エンジンの既定シェーダーを足す場合は `project/Engine/Graphics/Shader/ShaderLibrary.cpp` の `RegisterDefaultShaders` に同じ形で書きます。

### 3. マテリアルを作る

Debug 構成で起動し、MaterialList ウィンドウの Create ボタンから新しい `.mat` を作ります。作った `.mat` は `Assets/Materials/` に保存され、`AssetDatabase` の索引に登録されます。Inspector でシェーダーを切り替えると、cbuffer が作り直され、`params` の既定値で初期化されます。`.mat` の形式は [AssetLayer.md](AssetLayer.md) を参照してください。

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
- 登録は `Asset` の初期化より前に行う。`Application::Initialize` が `Game::RegisterGameShaders` をその位置で呼んでいる。
- HLSL 側の cbuffer と C++ 側の `params` の並びを一致させる。`b1` と `b2` の構造体はエンジン側で固定されている。既存の `name` を変えると、その名前を保存している `.mat` が読めなくなる。
- `requiresInstancing` のシェーダーは `Renderer::DrawModel` では描かず、エラーマテリアルへ差し替える。VS が未設定のルート SRV を読んでデバイス削除になるのを防ぐため。
- 頂点レイアウトは `None`（頂点バッファ無し）、`Mesh`（POSITION、TEXCOORD、NORMAL）、`Line`（POSITION、COLOR）の 3 種。
- ブレンド（`PipelineState::GetBlendDesc`）は、色の式はモードごとに異なり、透明度の列は全モード共通で「重ね塗り」（`ONE` と `INV_SRC_ALPHA`）にする。`kBlendModeNone` だけはブレンド自体を無効にする。
- HLSL 側の構造体と C++ 側の構造体は 1 バイトも違ってはいけない。`GizmoVertex`（32 バイト）、`SpriteForGPU`（128 バイト）、`CameraForGPU`（16 バイト）は `static_assert` で固定している。

## 未実装・要確認

- `ShaderCompiler::Compile` はコンパイル失敗時に Debug 構成では `assert` で止まり、Release 構成では空のブロブを返します。HLSL は実行時にコンパイルされるため、CI ではシェーダーの誤りを検出できません。
- PSO の RTV フォーマットは `R8G8B8A8_UNORM_SRGB` で固定ですが、エディタ構成のスワップチェーンの RTV は `UNORM` です。エディタ構成ではバックバッファへ描くのは ImGui だけなので実害はありません。
