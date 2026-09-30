# Application層
*アプリをアプリたらしめるところ*<br>
<br>
最上位の層。各層を生成・初期化・破棄し、メインループを回します。ゲーム固有のコード（Game/）との接点もここです。<br>
<br>
## 場所
`project/Engine/Application/`（エントリポイントは `project/main.cpp`、ゲーム側は `project/Game/`）<br>
<br>
## 構成
- Application … 全層の所有者。COM初期化、D3Dリソースリークチェック、クラッシュダンプ（`./Dumps/`）、初期化、メインループ（Run / RunFrame）、リサイズ検知。
- WindowApp … Win32ウィンドウの生成、メッセージ処理、WndProc から InputManager への入力転送、ボーダレス全画面、ウィンドウ矩形の控え（WM_DESTROY 後も終了時に使える）。
- ProjectSettings … ゲーム1本ぶんの設定（設計解像度、起動ウィンドウサイズ、全画面、タイトル、起動シーン）。`ProjectSettings/ProjectSettings.json` に保存し、Git へコミットする前提。
- BuildConfig.h … ビルド構成フラグ（USE_IMGUI / ENABLE_EDITOR）の定義と整合チェック。
- main.cpp … WinMain。`Application app; app.Initialize(hInstance); app.Run();` のみ。

Game/（ユーザーのゲームコード。Engine の外）:
- GameModule … `Game::RegisterGameComponents`（RotatorComponent、PlayerComponent を登録）と `Game::RegisterGameShaders`（Object3D、Particle を登録）。
- Components/ … ゲーム固有のコンポーネント（グローバル名前空間。CAKE_REFLECT は Cake 名前空間で特殊化）。
- GameLayer.h … コライダーの layer / mask に入れるビット値の一覧（`GameLayer` 名前空間）。

## 初期化順（Application::Initialize）
- 1. DebugLog開始 → DPI対応（USE_IMGUI 時）→ クラッシュダンプ設定
- 2. ProjectSettings 読込（失敗しても既定値で続行。エディタ時は初回にファイルを作る）
- 3. （エディタ）EditorPreferences・EditorCache 読込 → WindowApp 生成（前回のウィンドウ矩形を復元）
- 4. クライアント領域サイズを確定（0なら ProjectSettings の値）
- 5. Platform → InputManager を WindowApp へ登録 → Graphics
- 6. Game::RegisterGameShaders（Asset初期化より前。マテリアル読込でシェーダー名を解決するため）
- 7. Asset → Renderer
- 8. RegisterAllComponents → Game::RegisterGameComponents → TypeRegistry::FinalizeRegistration（シーン読込より前）
- 9. SceneRenderer → SceneManager → 初期シーンの読込（エディタは前回のシーン優先）
- 10. （エディタ）Editor → Time::Reset（初期化にかかった時間を1フレーム目へ流さない）

## フレームの流れ（RunFrame）
- 共通: DebugLog::PumpMainThread → Platform::BeginFrame → リサイズ検知 → Renderer::BeginFrame → …（下記）… → Renderer::EndFrame（ExecuteAndWait と Present）
- エディタビルド: 先頭で Editor::PrepareRenderTargets。中間で Editor の BeginFrame / Update / Draw / EndFrame。最後に PerformanceProfiler::Resolve。
- リリースビルド: 中間で PrepareBackbufferToGame（ProjectSettings のアスペクト比でレターボックス）→ SceneManager::Update → メインカメラの視点を作って SceneRenderer::Render。カメラが無ければ描かない。

## 依存関係
規則（Overview.mdの階層）:
- 最上位。すべての下位層に依存してよい。

現状の実装:
- Foundation、Platform、Graphics、Asset、Render、Scene、Editor（ENABLE_EDITOR 時のみ）、および Game/GameModule.h。
- 外部 … ImGui（DPI対応の1関数）。

上位から見た逆方向の依存（現状の実装）:
- Graphics層の SwapChain.h と Editor層の各ヘッダが BuildConfig.h をincludeしている。Editor層は ProjectSettings.h も使っている。これらは Application フォルダにあるが、実質は下位層と共有する設定・ビルド定義。

## 規則
- メンバの宣言順が初期化順で、破棄は逆順（Application.h に明記）。Editor は graphics_ より後に宣言する。
- 実際に開いたクライアント領域を、Application が唯一の基準サイズとして確定する。0 のままではスワップチェーンを作れない。
- フルスクリーンはゲーム本番（Release）のみ。エディタビルドでは無視される。
- 各層への依存は Desc 構造体（AssetInitializeDesc、RendererInitDesc）で渡す。必須項目は IsValid で初期化時に検査する。
- ユーザー定義のシェーダーは Asset 初期化より前、ユーザー定義のコンポーネントはシーン読込より前に登録する。締め切り（FinalizeRegistration）の後は Register が拒否される。
- ProjectSettings は共有データなので、個人ごとの値（テーマ、前回の状態）を混ぜない。描画実装の都合（PSO、バッファサイズなど）も入れない。
- 終了時は GPU の完了を待ってから、エディタの状態（EditorCache）と個人設定（EditorPreferences）を書き出す。
- Game/ のコンポーネントを追加する手順: 構造体と CAKE_REFLECT を書き、GameModule.cpp の RegisterGameComponents に1行足す（Update の優先度は第1引数）。
