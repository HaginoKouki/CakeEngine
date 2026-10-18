# 全体設計

CakeEngine は、役割ごとに機能を 8 つの層に分けた C++ と DirectX 12 のゲームエンジンです。このページでは、層の並びと依存の規則、起動から 1 フレームが終わるまでの流れ、ビルド構成を説明します。各層の中身は、層ごとのページで説明します。

## 層の一覧

上の層が下の層を使います。表は上位から下位の順に並べています。

| 層 | 何のための層か | 主な中身 | 詳細 |
| --- | --- | --- | --- |
| Application | エンジンを起動し、各層を所有してメインループを回す | `Application`、`WindowApp`、`ProjectSettings`、ゲームコードとの接点 | [ApplicationLayer.md](ApplicationLayer.md) |
| Editor | ゲーム制作のための編集 UI を提供する（Debug 構成のみ） | `Editor`、Scene / Game ウィンドウ、Inspector、Play / Stop | [EditorLayer.md](EditorLayer.md) |
| Scene | ゲーム世界を GameObject とコンポーネントで表現し、更新と保存を行う | `Scene`、`SceneManager`、各コンポーネント、System、`SceneSerializer` | [SceneLayer.md](SceneLayer.md) |
| Render | シーンを 1 枚の絵にする工程を組み立て、描画命令を発行する | `Renderer`、`SceneRenderer`、描画パス、`CameraView` | [RenderLayer.md](RenderLayer.md) |
| Asset | モデル・テクスチャ・マテリアルを読み込み、識別子とハンドルで管理する | 各 Manager、`AssetDatabase`、`.meta`、`AssetRef` | [AssetLayer.md](AssetLayer.md) |
| Graphics | DirectX 12 のデバイス・コマンド・シェーダー・PSO をまとめる | `Graphics`、`DirectXDevice`、`PipelineState`、`ShaderLibrary` | [GraphicsLayer.md](GraphicsLayer.md) |
| Platform | OS の機能（時間・入力・音・ファイルダイアログ）をエンジン向けに扱う | `Platform`、`Time`、`InputManager`、`SoundManager` | [PlatformLayer.md](PlatformLayer.md) |
| Foundation | OS や GPU を知らない基礎部品を提供する | 数学、識別子、ログ、シリアライズ、リフレクション | [FoundationLayer.md](FoundationLayer.md) |

層ごとのコードはすべて `project/Engine/<層名>/` にあり、名前空間は `Cake` です。ゲーム固有のコードは `project/Game/` に置き、Engine の外として扱います。

## 依存の規則と現状

### 規則

依存は「上の層から下の層へ」の一方向です。下の層が上の層を参照すると、層の境界が崩れて変更の影響範囲が広がるため、禁止しています。各層の入口となるクラス（`Platform.h`、`Graphics.h`、`Asset.h`）のヘッダコメントにも、同じ規則が書かれています。

### 実装上の依存

ソースの `#include` を集計した結果です。規則と一致しない箇所は右の列に示します。

| 層 | 依存している層（実装） | 規則との食い違い |
| --- | --- | --- |
| Application | すべての層、`Game/` | なし |
| Editor | Scene、Asset、Graphics、Render、Platform、Foundation | Application 層の `BuildConfig.h` と `ProjectSettings.h` を参照している |
| Scene | Asset、Render、Platform、Foundation | Render 層と相互に参照している（下記）。`GizmoSystem.h` が ImGui と ImGuizmo を無条件に include している |
| Render | Scene、Graphics、Asset、Foundation | Scene 層の System を呼んでいる（下記） |
| Asset | Graphics、Foundation | なし |
| Graphics | Platform、Foundation | Application 層の `BuildConfig.h`（`SwapChain.h`）を参照している。`GPUResourceUtility.cpp` が Asset 層の `Material.h` を include しているが、使っていない |
| Platform | Foundation | なし |
| Foundation | なし（規則） | `TypeInfo.h` が Asset 層の `AssetRef.h` と各 Handle、Scene 層の `GameObjectId.h` を include している。`DebugLog.h` と `Convert.cpp` が `Windows.h` に依存している |

### Render 層と Scene 層の相互参照

Render 層の各パスは Scene 層の System を呼び出し、Scene 層の System は Render 層の型を使います。

| 方向 | 具体例 |
| --- | --- |
| Render → Scene | `OpaquePass` → `RenderSystem`、`LightSetupPass` → `LightSystem`、`GizmoPass` → `GizmoSystem`、`SceneRenderer` → `CameraSystem`、`ParticlePass` → `Scene` と各 Particle コンポーネント |
| Scene → Render | `CameraSystem` → `CameraView`、`LightSystem` → `Light.h`、`GizmoSystem` → `GizmoDrawList`、`RenderSystem` → `Renderer` |

ヘッダ単位では前方宣言で循環を避けていますが、`.cpp` 単位では相互に参照しています。どちらの層を上とみなすかは、実装からは決められません。この点は要確認事項として [レビューの運用](../review/README.md) に記録しています。

### Asset 層と Graphics 層の順序

以前の設計書では Asset 層を Graphics 層より下に置いていました。一方、`Asset.h` のコメントは「Graphics に依存する」と書いており、実装もそのとおりです（`TextureManager` が `DescriptorHeap` を、`Material` が `ShaderDefinition` を使う）。このページでは実装に合わせて Asset を Graphics の上に置いています。どちらを正とするかは要確認事項です。

## 起動の流れ

`main.cpp` の `WinMain` は `Application` を作り、`Initialize` と `Run` を呼ぶだけです。`Application::Initialize` は次の順に各層を初期化します。順序には理由があり、入れ替えると動きません。

| 順 | 処理 | 順序が決まっている理由 |
| --- | --- | --- |
| 1 | `DebugLog` の開始、DPI 対応（`USE_IMGUI` 時）、クラッシュダンプの設定 | 最初に `DebugLog::GetInstance` を呼んだスレッドがメインスレッドになる |
| 2 | `ProjectSettings` の読み込み | 読めなくても既定値で続行する。エディタ構成では初回にファイルを作る |
| 3 | （エディタ）`EditorPreferences` と `EditorCache` の読み込み | ウィンドウの位置と大きさを復元するため、ウィンドウ生成より前に読む |
| 4 | `WindowApp` でウィンドウを生成し、クライアント領域の大きさを確定する | 0 のままではスワップチェーンを作れないため、0 なら設定値で補う |
| 5 | `Platform` の初期化と、`InputManager` を `WindowApp` へ登録 | WndProc が受けた入力を `InputManager` へ転送する |
| 6 | `Graphics` の初期化 | デバイス、コマンド、ヒープ、スワップチェーン、シェーダー、PSO を作る |
| 7 | `Game::RegisterGameShaders` | マテリアルの読み込みでシェーダー名を解決するため、Asset より前に登録する |
| 8 | `Asset` の初期化 | `Assets/` 以下を走査して `.meta` と索引を作る |
| 9 | `Renderer` の初期化 | Skybox 用マテリアルの生成を含む |
| 10 | `RegisterAllComponents`、`Game::RegisterGameComponents`、`TypeRegistry::FinalizeRegistration` | シーン読み込みで型名からコンポーネントを復元するため、読み込みより前に登録を締め切る |
| 11 | `SceneRenderer` と `SceneManager` の初期化、初期シーンの読み込み | エディタ構成では前回開いていたシーンを優先する |
| 12 | （エディタ）`Editor` の初期化 | ImGui の初期化と 2 枚のオフスクリーン描画先の生成 |
| 13 | `Time::Reset` | 初期化にかかった時間を 1 フレーム目の経過時間に混ぜない |

各層へ渡す依存は、`AssetInitializeDesc` や `RendererInitDesc` のような構造体にまとめて渡します。必須項目が埋まっているかは `IsValid` で初期化時に検査します。

## 1 フレームの流れ

`Application::Run` は `WindowApp::ProcessMessage` が false を返すまで `RunFrame` を繰り返します。`RunFrame` の流れは構成によって中間部分が変わります。

| 段階 | 共通 | エディタ構成（Debug） | リリース構成（Release） |
| --- | --- | --- | --- |
| 1 | `DebugLog::PumpMainThread` でワーカーのログを表示用一覧へ移す | | |
| 2 | `Platform::BeginFrame` で時間を測り、入力のスナップショットを取る | | |
| 3 | | `Editor::PrepareRenderTargets` で 2 枚の描画先の遅延リサイズを適用する | |
| 4 | `CheckResize` でウィンドウサイズの変化を調べ、変わっていればスワップチェーンを作り直す | | 深度バッファもクライアント領域に合わせて作り直す |
| 5 | `Renderer::BeginFrame` でコマンドリストを取り、バックバッファへのバリアを張る | | |
| 6 | | `Editor` の `BeginFrame` → `Update` → `Draw` → `EndFrame`。シーンの更新、2 視点の描画、ImGui の描画をここで行う | `PrepareBackbufferToGame` でレターボックスを設定し、`SceneManager::Update` の後にメインカメラの視点で `SceneRenderer::Render` を呼ぶ |
| 7 | `Renderer::EndFrame` でコマンドを実行し、GPU の完了を待ってから Present する | | |
| 8 | | `PerformanceProfiler::Resolve` で GPU 時間を読み戻す | |

`Renderer::EndFrame` が毎フレーム GPU を待ち切る同期設計です。このため、フレーム内で使い捨てる定数バッファや頂点バッファ、プロファイラは多重バッファリングを持ちません。描画を非同期にする場合は、これら 3 つを作り直す必要があります。

## シーンが絵になるまで

シーンの状態がどの層を通って画面に出るかを、上から下へ追います。

1. Scene 層の `SceneManager::Update` がコンポーネントの `Update` を呼び、`Scene::UpdateTransforms` でワールド行列を確定し、パーティクルの粒を進め、衝突判定を行います。
2. 描く側は視点（`CameraView`）を用意します。ゲームビューは `CameraSystem` がシーン内の `CameraComponent` から作り、エディタのシーンビューは `EditorCamera` が作ります。
3. Render 層の `SceneRenderer::Render` が視点を `Renderer::SetCamera` で GPU へ渡し、パス列を順に実行します。パスは LightSetup → Skybox → Opaque → Particle → Gizmo の順です。
4. 各パスは Scene 層の System（`DrawMeshRenderers` など）を呼び、System はコンポーネントのプールを走査して `Renderer::DrawModel` などを呼びます。
5. `Renderer` は Asset 層のハンドルからメッシュとマテリアルを解決し、Graphics 層の PSO とルートシグネチャを使って DirectX 12 のコマンドを積みます。

ゲームビューとシーンビューは、同じフレームで同じパス列を 2 回実行します。そのため、パスはシーンの状態を変更しません。

## ビルド構成

構成は Debug と Release の 2 つで、プラットフォームは x64 のみです。切り替えに使うプリプロセッサ定義は `BuildConfig.h` に集約しています。

| 構成 | 定義 | 含まれるもの |
| --- | --- | --- |
| Debug | `_DEBUG`、`_WINDOWS`、`ENABLE_EDITOR`、`USE_IMGUI` | エディタ、ImGui、プロファイラの HUD |
| Release | `NDEBUG`、`_WINDOWS` | ゲームのみ。`externals/imgui` と `externals/imgradient` の `.cpp` はビルドから除外される |

`USE_IMGUI` は ImGui ライブラリを使うかどうか、`ENABLE_EDITOR` はエディタ機能を有効にするかどうかを表します。エディタは ImGui に依存するため、`ENABLE_EDITOR` を定義して `USE_IMGUI` を定義しない構成はコンパイルエラーになります。使い分けは [Conventions.md](Conventions.md) を参照してください。

ビルド手順と実行時の前提は [guides/BuildAndRun.md](../guides/BuildAndRun.md) にまとめています。

## 設計の共通の考え方

層をまたいで繰り返し出てくる考え方を挙げます。詳しい規則は [Conventions.md](Conventions.md) にあります。

| 考え方 | 内容 | 主な登場箇所 |
| --- | --- | --- |
| ハンドルで参照する | 実体は Manager の連続配置プールが所有し、外へは index と generation の組を払い出す。返されたポインタは次の追加まで有効 | `MaterialHandle`、`ModelHandle`、`TextureHandle`、`GameObjectId`、`ComponentRef` |
| 保存には識別子だけを使う | ファイルに書くのは GUID と LocalId、または型名だけ。ハンドルや型 ID は実行時にだけ意味を持つ | `AssetRef`、`SceneSerializer`、`.meta` |
| コンポーネントはデータのみ | コンポーネントは仮想関数を持たない素の構造体で、処理は System が担当する | Scene 層全体 |
| 読み込み失敗で落とさない | 失敗時はエラー用のテクスチャやマテリアルを返し、欠損が見える状態で動き続ける | `TextureManager`、`MaterialManager`、`JsonFile` |
| 設定の置き場所を分ける | チームで共有する値は `ProjectSettings`、個人の設定は `EditorPreferences`、前回の状態は `EditorCache` | Application 層、Editor 層 |
