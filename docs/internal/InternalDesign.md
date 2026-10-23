# 内部設計

CakeEngine は、役割ごとに機能を 8 つの層に分けた C++ と DirectX 12 のゲームエンジンです。このページでは、層の並びと依存の規則、起動から 1 フレームが終わるまでの流れ、ビルド構成、層をまたいで守られている設計規則をまとめます。各層の中身は [layers/](layers/) の層ごとのページで説明します。

このページと各層のページに書かれている「守るべき規則」が、[docs/review/issue.md](../review/issue.md) で「設計規則」と呼んでいるものです。

## 層の一覧

上の層が下の層を使います。表は上位から下位の順に並べています。

| 層 | 何のための層か | 主な中身 | 詳細 |
| --- | --- | --- | --- |
| Application | エンジンを起動し、各層を所有してメインループを回す | `Application`、`WindowApp`、`ProjectSettings`、ゲームコードとの接点 | [ApplicationLayer.md](layers/ApplicationLayer.md) |
| Editor | ゲーム制作のための編集 UI を提供する（Debug 構成のみ） | `Editor`、Scene / Game ウィンドウ、Inspector、Play / Stop | [EditorLayer.md](layers/EditorLayer.md) |
| Scene | ゲーム世界を GameObject とコンポーネントで表現し、更新と保存を行う | `Scene`、`SceneManager`、各コンポーネント、System、`SceneSerializer` | [SceneLayer.md](layers/SceneLayer.md) |
| Render | シーンを 1 枚の絵にする工程を組み立て、描画命令を発行する | `Renderer`、`SceneRenderer`、描画パス、`CameraView` | [RenderLayer.md](layers/RenderLayer.md) |
| Asset | モデル・テクスチャ・マテリアルを読み込み、識別子とハンドルで管理する | 各 Manager、`AssetDatabase`、`.meta`、`AssetRef` | [AssetLayer.md](layers/AssetLayer.md) |
| Graphics | DirectX 12 のデバイス・コマンド・シェーダー・PSO をまとめる | `Graphics`、`DirectXDevice`、`PipelineState`、`ShaderLibrary` | [GraphicsLayer.md](layers/GraphicsLayer.md) |
| Platform | OS の機能（時間・入力・音・ファイルダイアログ）をエンジン向けに扱う | `Platform`、`Time`、`InputManager`、`SoundManager` | [PlatformLayer.md](layers/PlatformLayer.md) |
| Foundation | OS や GPU を知らない基礎部品を提供する | 数学、識別子、ログ、シリアライズ、リフレクション | [FoundationLayer.md](layers/FoundationLayer.md) |

層ごとのコードはすべて `project/Engine/<層名>/` にあり、名前空間は `Cake` です。ゲーム固有のコードは `project/Game/` に置き、Engine の外として扱います。

## 依存の規則と現状

### 規則

依存は「上の層から下の層へ」の一方向です。下の層が上の層を参照すると、層の境界が崩れて変更の影響範囲が広がるため、禁止しています。各層の入口となるクラス（`Platform.h`、`Graphics.h`、`Asset.h`）のヘッダコメントにも、同じ規則が書かれています。

### 実装上の依存

ソースの `#include` を集計した結果です。規則と一致しない箇所は右の列に示します。これらは [docs/review/issue.md](../review/issue.md) にも記録しています。

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
| Render → Scene | `OpaquePass` → `RenderSystem`、`LightSetupPass` → `LightSystem`、`GizmoPass` → `GizmoSystem`、`SpritePass` → `SpriteSystem`、`SceneRenderer` → `CameraSystem`、`ParticlePass` → `Scene` と各 Particle コンポーネント |
| Scene → Render | `CameraSystem` → `CameraView`、`LightSystem` → `Light.h`、`GizmoSystem` → `GizmoDrawList`、`SpriteSystem` → `SpriteDrawList`、`RenderSystem` → `Renderer` |

ヘッダ単位では前方宣言で循環を避けていますが、`.cpp` 単位では相互に参照しています。どちらの層を上とみなすかは、実装からは決められません。

### Asset 層と Graphics 層の順序

`Asset.h` のコメントは「Graphics に依存する」と書いており、実装もそのとおりです（`TextureManager` が `DescriptorHeap` を、`Material` が `ShaderDefinition` を使います）。このページでは実装に合わせて Asset を Graphics の上に置いています。

## 起動の流れ

`main.cpp` の `WinMain` は `Application` を作り、`Initialize` と `Run` を呼ぶだけです。`Application::Initialize` は次の順に各層を初期化します。順序には理由があり、入れ替えると動きません。

| 順 | 処理 | 順序が決まっている理由 |
| --- | --- | --- |
| 1 | `DebugLog` の開始、DPI 対応（`USE_IMGUI` 時）、クラッシュダンプの設定 | 最初に `DebugLog::GetInstance` を呼んだスレッドがメインスレッドになる |
| 2 | `ProjectSettings` の読み込み | 読めなくても既定値で続行する。エディタ構成では初回にファイルを作る |
| 3 | （エディタ）`EditorPreferences` と `EditorCache` の読み込み | ウィンドウの位置と大きさを復元するため、ウィンドウ生成より前に読む |
| 4 | `WindowApp` でウィンドウを生成し、クライアント領域の大きさを確定する | 0 のままではスワップチェーンを作れないため、0 なら設定値で補う |
| 5 | `Platform` の初期化（タイマー分解能を 1 ms に上げ、`Time` と `InputManager` を初期化）と、`InputManager` を `WindowApp` へ登録 | WndProc が受けた入力を `InputManager` へ転送する。分解能はフレーム末尾の待機を正確にするために上げる |
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
| 9 | `Platform::EndFrame` で、フレーム開始から 1/60 秒になるまで待つ（60 FPS 固定） | | |

`Renderer::EndFrame` が毎フレーム GPU を待ち切る同期設計です。このため、フレーム内で使い捨てる定数バッファや頂点バッファ、プロファイラは多重バッファリングを持ちません。描画を非同期にする場合は、これら 3 つを作り直す必要があります。

フレームレートは `Time::WaitForTargetFrame` が 60 FPS に固定します。待つのは Present の後で、フレーム開始から 1/60 秒に満たない分だけです。待ち方は [PlatformLayer.md](layers/PlatformLayer.md) の「フレームレートの固定」にあります。

## シーンが絵になるまで

シーンの状態がどの層を通って画面に出るかを、上から下へ追います。

1. Scene 層の `SceneManager::Update` がコンポーネントの `Update` を呼び、`Scene::UpdateTransforms` でワールド行列を確定し、パーティクルの粒を進め、衝突判定を行います。
2. 描く側は視点（`CameraView`）を用意します。ゲームビューは `CameraSystem` がシーン内の `CameraComponent` から作り、エディタのシーンビューは `EditorCamera` が作ります。
3. Render 層の `SceneRenderer::Render` が視点を `Renderer::SetCamera` で GPU へ渡し、パス列を順に実行します。パスは LightSetup → Skybox → Opaque → Particle → Gizmo → Sprite の順です。
4. 各パスは Scene 層の System（`DrawMeshRenderers` など）を呼び、System はコンポーネントのプールを走査して `Renderer::DrawModel` などを呼びます。
5. `Renderer` は Asset 層のハンドルからメッシュとマテリアルを解決し、Graphics 層の PSO とルートシグネチャを使って DirectX 12 のコマンドを積みます。

ゲームビューとシーンビューは、同じフレームで同じパス列を 2 回実行します。そのため、パスはシーンの状態を変更しません。

## ビルド構成

構成は Debug と Release の 2 つで、プラットフォームは x64 のみです。切り替えに使うプリプロセッサ定義は `BuildConfig.h` に集約しています。

| 構成 | 定義 | 含まれるもの |
| --- | --- | --- |
| Debug | `_DEBUG`、`_WINDOWS`、`ENABLE_EDITOR`、`USE_IMGUI` | エディタ、ImGui、プロファイラの HUD。警告はエラー扱い |
| Release | `NDEBUG`、`_WINDOWS` | ゲームのみ。`externals/imgui` と `externals/imgradient` の `.cpp` はビルドから除外される |

`USE_IMGUI` は ImGui ライブラリを使うかどうか、`ENABLE_EDITOR` はエディタ機能を有効にするかどうかを表します。エディタは ImGui に依存するため、`ENABLE_EDITOR` を定義して `USE_IMGUI` を定義しない構成はコンパイルエラーになります。

| フラグ | 意味 | 使う場所 |
| --- | --- | --- |
| `USE_IMGUI` | ImGui ライブラリを使うか（include、リンク、API 呼び出しの可否） | 純粋に ImGui API を叩くだけの箇所 |
| `ENABLE_EDITOR` | エディタ機能を有効にするか | `Editor` クラス、ビューポート、インスペクタなど、エディタ機能そのもの |

ソリューションには `CakeEngine` と `externals/DirectXTex/DirectXTex_Desktop_2022_Win10.vcxproj` の 2 プロジェクトが入り、DirectXTex は静的ライブラリとして一緒にビルドされます。ビルド後イベントで Windows SDK の `dxcompiler.dll` と `dxil.dll` を出力フォルダへコピーします。HLSL は起動時に DXC で実行時コンパイルされるため、この 2 つが無いと起動できません。ビルドの手順と実行時の前提は [README.md](../../README.md) にあります。

### CI

`.github/workflows/` に、`main` への push で走るワークフローがあります。

| ワークフロー | ランナー | 内容 |
| --- | --- | --- |
| `DebugBuild.yml` | `windows-2025-vs2026` | Debug 構成を msbuild でビルドする |
| `ReleaseBuild.yml` | `windows-2025-vs2026` | Release 構成を msbuild でビルドする |
| `CheckUnwantedFiles.yml` | `ubuntu-latest` | `.pdb`、`.dmp`、`generated/`、`.vs/` などの混入を検索し、見つかれば失敗する |
| `Development.yml` | `windows-2025-vs2026` | Development 構成をビルドする。対応する構成はプロジェクトから削除済みで、このファイルだけが残っている |

CI がビルドするのは C++ だけです。HLSL は実行時にコンパイルされるため、シェーダーの誤りは CI では検出されません。

## 設計の共通の考え方

層をまたいで繰り返し出てくる考え方を挙げます。

| 考え方 | 内容 | 主な登場箇所 |
| --- | --- | --- |
| ハンドルで参照する | 実体は Manager の連続配置プールが所有し、外へは index と generation の組を払い出す。返されたポインタは次の追加まで有効 | `MaterialHandle`、`ModelHandle`、`TextureHandle`、`GameObjectId`、`ComponentRef` |
| 保存には識別子だけを使う | ファイルに書くのは GUID と LocalId、または型名だけ。ハンドルや型 ID は実行時にだけ意味を持つ | `AssetRef`、`SceneSerializer`、`.meta` |
| コンポーネントはデータのみ | コンポーネントは仮想関数を持たない素の構造体で、処理は System が担当する | Scene 層全体 |
| 読み込み失敗で落とさない | 失敗時はエラー用のテクスチャやマテリアルを返し、欠損が見える状態で動き続ける | `TextureManager`、`MaterialManager`、`JsonFile` |
| 設定の置き場所を分ける | チームで共有する値は `ProjectSettings`、個人の設定は `EditorPreferences`、前回の状態は `EditorCache` | Application 層、Editor 層 |

## コーディング規約

整形ルールは `project/.clang-format` と `project/.editorconfig` が正で、ここではその要点と、ツールでは検査できない約束を説明します。

### 言語とツール

| 項目 | 内容 |
| --- | --- |
| 言語 | C++20（`stdcpp20`）。`concept`、指示付き初期化、`std::span`、`std::format` を使う |
| コンパイラ | Visual Studio 2026 の MSVC（プラットフォームツールセット v145） |
| 名前空間 | エンジンはすべて `Cake`。ゲーム固有のコンポーネントはグローバル名前空間、`GameLayer` のような補助は専用の名前空間 |
| 文字コード | UTF-8（`/utf-8`）。コメントは日本語で書く。HLSL ファイルには日本語を書かない（先頭に英語で注記がある） |
| 警告 | Debug 構成では警告をエラー扱いにする。外部ライブラリの警告は抑制する |
| スペルチェック | `.editorconfig` で識別子とコメントの英単語を検査する。固有の単語は `project/exclusion.dic` に追加する |

### 整形

`.clang-format` は LLVM を土台に次の点を上書きしています。

| 項目 | 規則 |
| --- | --- |
| インデント | タブ（表示幅 4）。行内の桁揃えだけスペース |
| 波括弧 | K&R。開き波括弧は宣言と同じ行、`} else {` の形 |
| 1 行の長さ | 上限なし（`ColumnLimit: 0`）。`.editorconfig` の `max_line_length` は 120 |
| ポインタと参照 | 型側に寄せる（`int* p`、`int& r`） |
| include | 自動で並べ替えない。書いた順を保つ |
| 短い関数 | クラス内の短いインライン関数は 1 行にまとめてよい。`if` やループは必ず改行する |
| アクセス指定子 | クラス本体より 1 段左（`AccessModifierOffset: -4`） |

### 命名

| 対象 | 規則 | 例 |
| --- | --- | --- |
| 型、関数 | パスカルケース | `SceneManager`、`UpdateTransforms` |
| メンバ変数 | 末尾にアンダースコア | `scene_`、`currentPath_` |
| ローカル変数、引数、公開メンバ | キャメルケース | `deltaTime`、`startColor` |
| 定数 | `k` 接頭辞 | `kMaxDeltaTime`、`kSrvReservedCount` |
| 列挙子 | パスカルケース。`BlendMode` のように `k` 接頭辞のものもある | `KeyState::Pressed`、`BlendMode::kBlendModeNormal` |
| シェーダー名 | スラッシュ区切りの階層 | `Unlit/Color`、`Skybox/Panoramic` |
| HLSL のリソース | `g` 接頭辞 | `gMaterial`、`gTexture` |
| ファイル | 型名と同じ。1 ファイル 1 型を基本とする | `Renderer.h` / `Renderer.cpp` |

### ファイルの配置

| 置き場所 | 内容 |
| --- | --- |
| `project/Engine/<層>/<機能>/` | 層ごとにフォルダを分け、その下を機能で分ける。層の入口（`Platform.h`、`Graphics.h`、`Asset.h`）は層フォルダの直下 |
| `project/Game/` | ゲーム固有のコード。Engine からは `GameModule.h` だけを参照する |
| `project/EngineResources/` | エンジンが使うシェーダー、フォント、テクスチャ。`AssetDatabase` の走査対象外 |
| `project/Assets/` | ゲームのアセット。`AssetDatabase` が走査し、`.meta` を発行する |
| `project/externals/` | 外部ライブラリ。整形とスペルチェックの対象外 |

include は `project/` を基準にした絶対パスで書きます（`#include "Engine/Scene/Scene.h"`）。同じフォルダのヘッダも同じ形で書き、`.cpp` が対応する `.h` を include するときだけ `#include "Renderer.h"` のように書きます。

### ヘッダコメントの様式

主要なヘッダは、先頭に次の形の説明を置いています。初めて読む人がファイルの目的と注意点を最初に読めるようにするためです。

```cpp
#pragma once
/*====================================
 *
 * このファイルが何をするかの 1 から 3 行の説明。
 *
 * 【見出し】
 * 設計上の判断と、その理由。「なぜこうなっているか」を書く。
 *
 * ====================================*/
```

関数やメンバのコメントは `// 説明.` のように末尾にピリオドを付けます。

## 設計規則

層をまたいで守られている約束です。各層のページにも、その層に固有の規則があります。これらに反する実装は [docs/review/issue.md](../review/issue.md) の「設計規則の違反」に記録します。

### 依存の向き

- 上の層から下の層へだけ依存する。層の入口クラスのヘッダコメントに、知ってよい層と知ってはいけない層を書く。
- 下の層が上の層の型を必要とするときは、前方宣言と引数渡しで受け取る。`RenderContext` や `UpdateContext` のように「そのフレームに使ってよいもの」を束ねた構造体で渡す。
- 層の初期化に必要な依存は Desc 構造体（`AssetInitializeDesc`、`RendererInitDesc`）で渡し、`IsValid` で検査する。

### 所有と参照

- 実体は 1 箇所（Manager、`Scene`）が所有し、ほかはハンドル（index と generation）か `GameObjectId` で参照する。生ポインタは「次の追加まで有効」で、保持しない。
- 非所有のポインタはコメントに「非所有」と書く。
- `Application` のメンバ宣言順が初期化順で、破棄は逆順。順序に意味がある場合はコメントに理由を書く。

### 保存形式

- ファイルに書くのは、実行をまたいでも変わらない値だけ。GUID、LocalId、型名、プロパティ名。
- ハンドル、型 ID、ワールド行列、ダーティフラグは保存しない。
- 保存形式を変えたら版数（`kVersion`、`kSceneVersion`、`kCurrentMetaVersion`）を上げる。
- 設定ファイルは「チームで共有する値」（`ProjectSettings`）と「個人の値」（`EditorPreferences`、`EditorCache`）を分け、後者は `UserSettings/` に置いてコミットしない。

### 失敗時の振る舞い

- 読み込み失敗でエンジンを落とさない。エラー用のテクスチャやマテリアルを返し、欠損が見える状態で動き続ける。
- 設定ファイルが壊れていても既定値で起動する。ログを出して `false` を返す。
- 初期化時の抜けは `assert` と `IsValid` で早く検出する。

### コンポーネントと System

- コンポーネントは仮想関数を持たないデータだけの構造体にする。処理は System が担当する。
- 構造体の直後に `CAKE_REFLECT` ブロックを書く。メンバを増やしたら `CAKE_PROPERTY` も足す。
- `Update` を持つ型だけが自動更新される。継承やインターフェースは使わない。
- 可変長のデータはコンポーネントに持たせず、`ParticleStorage` のように `Scene` 側に置く。

### GPU との契約

- HLSL 側の構造体と C++ 側の構造体は 1 バイトも違ってはいけない。`static_assert` でサイズを固定する。
- ルートシグネチャは 1 つを全シェーダーで共有する。番号の割り当ては [GraphicsLayer.md](layers/GraphicsLayer.md) を参照。
- 毎フレーム GPU を待ち切る同期設計を前提にしている。これを変えるときは `FrameConstantBuffer`、`FrameVertexBuffer`、`PerformanceProfiler` を作り直す。

## ドキュメントの書き方

ドキュメントを更新するときは、次の点を揃えます。

- 設計書には日付や変更履歴を書かない。履歴はレビュー記録（`docs/review/history/`）と Git に任せる。
- 実装されている事実、設計上の意図、未確定の事項を混ぜない。未確定の事項は各層のページの「未実装・要確認」の節に書く。
- クラス名、関数名、ファイル名は実装と一致させる。
- 一覧性が必要な情報は表にし、説明文は地の文で書く。
- 同じ内容を複数のページに書かず、参照リンクで済ませる。
- 各層のページは「この層が担当すること」「配置」「構成」「主要な仕組み」「他の層との関係」「守るべき規則」「未実装・要確認」の順で書く。
