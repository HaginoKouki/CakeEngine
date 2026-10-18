# Application 層

最上位の層です。各層を生成・初期化・破棄し、メインループを回します。ゲーム固有のコード（`project/Game/`）との接点もここです。

## この層が担当すること

Application 層は「どの層を、どの順に、何を渡して初期化するか」と「1 フレームに何をどの順で呼ぶか」を決めます。起動の順序と 1 フレームの流れは [Overview.md](Overview.md) に表としてまとめています。このページでは、各クラスの役割とゲームコードとの接点を説明します。

## 配置

| 場所 | 内容 |
| --- | --- |
| `project/Engine/Application/` | `Application`、`WindowApp`、`ProjectSettings`、`BuildConfig.h` |
| `project/main.cpp` | エントリポイント（`WinMain`） |
| `project/Game/` | ゲーム固有のコード。Engine の外 |

## 構成

| ファイル | 主な型 | 役割 |
| --- | --- | --- |
| `Application.h` / `.cpp` | `Application` | 全層の所有者。COM の初期化、D3D リソースのリークチェック、クラッシュダンプ（`./Dumps/`）、初期化、メインループ（`Run`、`RunFrame`）、リサイズ検知 |
| `WindowApp.h` / `.cpp` | `WindowApp`、`WindowPlacement` | Win32 ウィンドウの生成、メッセージ処理、WndProc から `InputManager` への入力転送、ボーダレス全画面、ウィンドウ矩形の控え |
| `ProjectSettings.h` / `.cpp` | `ProjectSettings` | ゲーム 1 本ぶんの設定。`ProjectSettings/ProjectSettings.json` に保存し、コミットする |
| `BuildConfig.h` | | ビルド構成フラグ（`USE_IMGUI`、`ENABLE_EDITOR`）の説明と整合チェック |
| `main.cpp` | `WinMain` | `Application app; app.Initialize(hInstance); app.Run();` のみ |

### ProjectSettings の項目

| 項目 | 既定値 | 意味 |
| --- | --- | --- |
| `referenceWidth`、`referenceHeight` | 1920、1080 | ゲーム画面の設計解像度。アスペクト比の基準 |
| `windowWidth`、`windowHeight` | 1280、720 | 起動時のウィンドウサイズ |
| `fullscreen` | false | ボーダレス全画面で起動するか。エディタ構成では無視される |
| `windowTitle` | "CakeEngine" | ウィンドウタイトル |
| `startupScene` | "Assets/GameScene.scene" | 起動時に読み込むシーン |
| `version` | 1 | ファイル形式の版数 |

### Application が持つメンバの順序

`Application` のメンバは宣言順に初期化され、逆順に破棄されます。この順序には意味があります。

| 順 | メンバ | 理由 |
| --- | --- | --- |
| 1 | `ComScope`、`D3DResourceLeakChecker` | COM を最初に初期化し、最後に D3D のリークを報告する |
| 2 | `ProjectSettings`、（エディタ）`EditorPreferences`、`EditorCache` | ウィンドウ生成より前に読む |
| 3 | `WindowApp` | ウィンドウ |
| 4 | `Platform`、`Graphics`、`Asset`、`Renderer` | 下の層から順に |
| 5 | `SceneRenderer`、`SceneManager` | |
| 6 | （エディタ）`Editor` | ImGui のシャットダウンと描画先の解放を、デバイス破棄より先に走らせるため `Graphics` より後 |

## ゲームコードとの接点

`project/Game/` はエンジンが知らない「ユーザーのゲームコード」です。エンジンへは 2 つの関数で接続します。

| 関数 | 呼ばれる場所 | 内容 |
| --- | --- | --- |
| `Game::RegisterGameShaders(ShaderLibrary&)` | `Graphics` 初期化の直後、`Asset` 初期化の前 | ゲーム固有のシェーダー定義を登録する。現状は `Object3D` と `Particle` |
| `Game::RegisterGameComponents()` | `RegisterAllComponents` の直後、`FinalizeRegistration` の前 | ゲーム固有のコンポーネントを登録する。現状は `RotatorComponent` と `PlayerComponent` |

`Game/` の中身は次のとおりです。

| 場所 | 内容 |
| --- | --- |
| `GameModule.h` / `.cpp` | 上記 2 関数の実装 |
| `Components/` | ゲーム固有のコンポーネント。グローバル名前空間に置き、`CAKE_REFLECT` は `Cake` 名前空間の中で書く |
| `GameLayer.h` | コライダーの `layer` と `mask` に入れるビット値の一覧（`GameLayer` 名前空間） |
| `Assets/Shaders/` | ゲーム固有の HLSL（`Object3D`、`Particle`） |

追加の手順は [guides/AddingComponents.md](../guides/AddingComponents.md) と [guides/AddingShaders.md](../guides/AddingShaders.md) にあります。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | 最上位。すべての下位層に依存してよい |
| 実装上の依存 | Foundation、Platform、Graphics、Asset、Render、Scene、Editor（`ENABLE_EDITOR` 時のみ）、`Game/GameModule.h` |
| 外部 | ImGui（DPI 対応の 1 関数。`USE_IMGUI` 時のみ） |
| 逆方向の参照 | Graphics 層の `SwapChain.h` と Editor 層の各ヘッダが `BuildConfig.h` を include している。Editor 層は `ProjectSettings.h` も使う。これらは Application フォルダにあるが、実質は下位層と共有する設定とビルド定義になっている |

## 守るべき規則

### 初期化と依存

- メンバの宣言順が初期化順で、破棄は逆順。`Editor` は `graphics_` より後に宣言する。
- 実際に開いたクライアント領域を、`Application` が唯一の基準サイズとして確定する。0 のままではスワップチェーンを作れない。
- フルスクリーンはゲーム本番（Release）のみ。エディタ構成では無視される。
- 各層への依存は Desc 構造体（`AssetInitializeDesc`、`RendererInitDesc`）で渡す。必須項目は `IsValid` で初期化時に検査する。
- ユーザー定義のシェーダーは `Asset` 初期化より前、ユーザー定義のコンポーネントはシーン読み込みより前に登録する。`FinalizeRegistration` の後は `Register` が拒否される。

### 設定と終了

- `ProjectSettings` は共有データなので、個人ごとの値（テーマ、前回の状態）を混ぜない。描画実装の都合（PSO、バッファサイズなど）も入れない。
- 終了時は GPU の完了を待ってから、エディタの状態（`EditorCache`）と個人設定（`EditorPreferences`）を書き出す。

## 未実装・要確認

- リポジトリの `ProjectSettings.json` は `fullscreen` が `true`、`windowTitle` が "CakeEngine_Game" です。Release 構成で起動するとボーダレス全画面になります。
- `.github/workflows/Development.yml` は、対応する Development 構成が削除された後もファイルとして残っています。
