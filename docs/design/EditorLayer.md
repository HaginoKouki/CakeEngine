# Editor 層

ImGui によるエディタ UI を提供する層です。`ENABLE_EDITOR` が定義されたビルド（Debug 構成）でのみ存在し、ゲーム固有のクラスを知りません。

## この層が担当すること

Editor 層は、`SceneManager` が持つ現在のシーンを更新・描画し、その上に ImGui の UI を重ねます。シーンの中身を直接変更する処理（コンポーネントの追加や削除、保存）は、Scene 層と Asset 層の API を呼ぶ形で行います。

## 配置

`project/Engine/Editor/`

## 構成

`Editor`（`Editor.h` / `.cpp`）が統括します。ウィンドウをまたぐ判断だけを持ち、UI は各クラスへ委譲します。

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| 直下 | `Editor` | 選択中オブジェクト、再生状態、入力先（Game / DebugCamera / None）、フレーム境界（ImGui の Begin / End、DockSpace）、描画先の作り直し |
| 直下 | `EditorDrawContext` | 描画関数が共通で使う参照の束（device、`ShaderLibrary`、各 Manager、`AssetDatabase`、`allowAssetEditing`） |
| `ImGui/` | `ImGuiManager` | DirectX 12 版の初期化、フレーム管理、テーマ、UI スケール、DPI 追従、フォント |
| `ImGui/` | `ImGuiCustomWidget`、`Icons.h` | Drag、Slider、Vector、Transform、Color、AssetRef 選択などの共通ウィジェット |
| `Viewport/` | `SceneWindow` | エディタカメラ視点のウィンドウ。描画先、`EditorCamera`、`GizmoSettings`、ImGuizmo による Transform 操作 |
| `Viewport/` | `GameWindow` | ゲームカメラ視点のプレビュー。`ProjectSettings` のアスペクト比でレターボックスする |
| `Camera/` | `EditorCamera` | 右ドラッグ中の WASD と QE で移動し、マウス移動で視点を回す。シーンには属さない |
| `PlayMode/` | `PlayModeController`、`PlayModeState` | Edit / Play / Paused の状態、JSON スナップショットによる巻き戻し、コマ送り |
| `ToolBar/` | `ToolBar` | Play、Pause、Resume、Step、Stop のボタン |
| `MenuBar/` | `MenuBar` | File と Edit メニュー、確認ダイアログ、Ctrl+S、設定ウィンドウの起動 |
| `Inspector/` | `SceneInspector`（`DrawHierarchyWindow`、`DrawInspectorWindow`） | Hierarchy と Inspector。コンポーネントの型を知らず、`TypeRegistry` から引く |
| `Inspector/` | `PropertyDrawer` | `PropertyType` ごとの ImGui ウィジェット。型ごとの switch はここだけ |
| `Inspector/` | `MaterialEditor`、`ModelEditor`、`InspectorMaterialSection` | マテリアルとモデルの編集 UI |
| `Settings/` | `EditorPreferences` | 個人設定（テーマ、言語、UI スケール）。`UserSettings/EditorPreferences.json` |
| `Settings/` | `EditorCache` | 前回終了時の状態（ウィンドウ矩形、開いていたシーン、エディタカメラ）。`UserSettings/EditorCache.json` |
| `Settings/` | `SettingsWindow` | Project Settings と Preferences のウィンドウ |
| `Log/` | `LogWindow` | `DebugLog` の一覧をレベルで絞って表示する。ログ本体だけ等幅フォントで描く |
| `Input/` | `InputVisualizerWindow` | キーボード、パッド、マウスの入力を絵で可視化するデバッグウィンドウ |

## 主要な仕組み

### エディタの 1 フレーム

`Application::RunFrame` から呼ばれる順です。

| 順 | 関数 | 内容 |
| --- | --- | --- |
| 1 | `PrepareRenderTargets` | 2 枚の描画先の遅延リサイズ要求を取り出し、どちらかがあれば `WaitForGPU` を 1 回だけ行って作り直す。深度バッファは 2 ビューの最大サイズに合わせる |
| 2 | `BeginFrame` | ImGui と ImGuizmo の NewFrame、DockSpace、DPI の追従、プロファイラの開始 |
| 3 | `Update` | `PlayModeController::Tick` で `Time` の一時停止を更新 → 前フレームのフォーカスで決めた入力先を反映 → `SceneManager::Update` → エディタカメラの更新 → UI 一式の構築（`DrawEditorUI`） |
| 4 | `Draw` | Game ウィンドウの描画先へゲームカメラ視点、Scene ウィンドウの描画先へエディタカメラ視点でシーンを描く |
| 5 | `EndFrame` | バックバッファを ImGui 用に用意し、ImGui を描く。プロファイラの終了 |

UI の構築（`Update`）が描画（`Draw`）より前にあるのは、ImGui のウィンドウサイズを今フレームのうちに描画先のリサイズ要求へ変えるためです。要求は次フレームの `PrepareRenderTargets` で適用されます。

### 入力先の切り替え

入力先は前フレームのフォーカスで確定します（1 フレーム遅延）。Game ウィンドウにフォーカスがあるときだけ `InputManager::SetGameInputEnabled(true)` でゲームへ入力を通し、Scene ウィンドウにフォーカスがあるときだけエディタカメラを動かします。

### Play / Stop

`PlayModeController` は Play を押した時点のシーンを JSON 文字列で控え、Stop でそこへ戻します。Play 中の変更（インスペクタの編集もゲームロジックによる移動も）はすべて巻き戻ります。Paused 中に Step を押すと、そのフレームだけ `Time` の一時停止を解除して 1 フレーム進めます。

Stop、New、Open でシーンが作り直されるため、既存の `GameObjectId` はすべて無効になり、`Editor` は選択を解除します。Play 中はファイル操作を止めます。

### 設定とキャッシュ

| ファイル | 中身 | 保存のタイミング | Git |
| --- | --- | --- | --- |
| `ProjectSettings/ProjectSettings.json` | ゲーム 1 本ぶんの設定（Application 層） | Save ボタンを押したときだけ | コミットする |
| `UserSettings/EditorPreferences.json` | テーマ、言語、UI スケール | 変更した瞬間 | コミットしない |
| `UserSettings/EditorCache.json` | ウィンドウ矩形、開いていたシーン、エディタカメラ | 終了時に 1 回 | コミットしない |

`EditorPreferences` と `EditorCache` はウィンドウ生成より前に読む必要があるため、`Editor` ではなく `Application` が所有します。

### 操作

| 操作 | 場所 | 動き |
| --- | --- | --- |
| 右ドラッグ + WASD、QE | Scene ウィンドウ | エディタカメラの移動と回転 |
| W、E、R | Scene ウィンドウ | ImGuizmo の操作を移動、回転、スケールに切り替える |
| Gizmos ボタン | Scene ウィンドウ左下 | Grid、Colliders、Cameras、Lights、Force Fields の表示を切り替える |
| Ctrl+S | どこでも（Edit 中のみ） | シーンと未保存のマテリアルをまとめて保存する。保存先が未確定なら先にダイアログで決める |
| Add Component | Inspector | `TypeRegistry::AddComponent` 経由で追加する。依存先も一緒に付く |
| コンポーネントの削除 | Inspector | `FindDependent` で依存されていないことを確かめてから削除する |

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | Scene 層以下にのみ依存する。Application 層はこの層の上 |
| 実装上の依存 | Platform、Graphics、Asset、Render、Scene、Foundation の各層と、ImGui、ImGuizmo、IconFontCppHeaders |
| 規則との食い違い | ほぼすべてのヘッダが Application 層の `BuildConfig.h` を include している。`Editor.h`、`MenuBar.cpp`、`SettingsWindow.cpp`、`GameWindow.cpp`、`SceneWindow.cpp` が `ProjectSettings.h` を使う。`BuildConfig` と `ProjectSettings` は Application フォルダにあるが、実質は下位層と共有する設定の置き場になっている |
| 外部ライブラリ | `externals/imgradient`（グラデーション編集ウィジェット）は vcxproj に登録されているが、Engine と Game からは使われていない。ImGuizmo 同梱の `externals/imgui/ImGradient.*` とは別物 |
| ゲームコード | `Game/` は知らない |

## 守るべき規則

### 所有と状態

- `Editor` は `Application` が所有し、`graphics_` より後に宣言する（ImGui のシャットダウンと描画先の解放をデバイス破棄より先に走らせるため）。
- 各ウィンドウが持つのはそのウィンドウ固有の状態だけ。選択中オブジェクトと再生状態のようなウィンドウ横断の状態は `Editor` が持ち、引数で渡す。

### 描画と入力

- 描画先の作り直しは `Editor` が順番を握る。2 枚の遅延リサイズ要求を取り出してから `WaitForGPU` を 1 回だけ行う。
- 入力先は前フレームのフォーカスで確定する。
- Scene ウィンドウのギズモは、ビューポート画像の直後に `ImGuizmo::SetDrawlist` を呼ぶ（呼ばないと別ウィンドウの上に描かれる）。

### シーンと保存

- Play 中の状態は Stop で巻き戻る。Stop、New、Open の後は選択を解除する。Play 中はファイル操作を止める。
- `ProjectSettings` は Save ボタンでのみ書く。`EditorPreferences` は変更した瞬間に書く。`EditorCache` は終了時に 1 回だけ書く。
- ウィンドウ矩形は枠を含むウィンドウ矩形で保存する（クライアント矩形ではない）。
- マテリアル編集は `MarkDirty` を立てるだけで、書き戻しは Ctrl+S（`MaterialManager::SaveDirtyMaterials`）が行う。保存先は `Material` の `assetPath_` で、UI 側でパスを組み立てない。

### Inspector

- Inspector はコンポーネントの型を知らない。コンポーネントを追加しても `SceneInspector` と `PropertyDrawer` は触らない。`PropertyType` を増やしたときだけ `PropertyDrawer` と `SceneSerializer` の switch に追加する。
- ImGui API を叩くだけの箇所は `#ifdef USE_IMGUI`、エディタ機能そのものは `#ifdef ENABLE_EDITOR` で囲む。

## 未実装・要確認

- `InputVisualizerWindow` と `DrawModelList` は定義されていますが、`Editor::DrawEditorUI` から呼ばれていません。
- `EditorDrawContext::allowAssetEditing` のコメントは「毎フレーム Editor が更新する」と書いていますが、`Editor` 側で書き換えている箇所はなく、常に `true` のままです。
- `Editor::DrawEditorUI` で `ImGui::ShowDemoWindow()` を毎フレーム呼んでいます。
- `ImGuiManager::GetMainFont` は保持していますが、呼び出し元がありません。
- Inspector 系のヘッダは `#ifdef USE_IMGUI`、`EditorDrawContext.h` は `#ifdef ENABLE_EDITOR` で囲まれており、ガードの基準が揺れています。現状 `USE_IMGUI` 単独の構成は無いため、ビルドには影響していません。
- `EditorPreferences::language` は値を保存するだけで、UI 文字列の差し替えは実装されていません。
