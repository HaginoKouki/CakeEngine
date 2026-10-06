# Editor層
*ゲーム制作するための層*



ImGuiによるエディタUIを提供する層です。ENABLE_EDITOR が定義されたビルド（Debug / Development）でのみ存在し、ゲーム固有のクラスを知りません。



## 場所
`project/Engine/Editor/`



## ビルド構成
- Debug と Development: `ENABLE_EDITOR` + `USE_IMGUI`（Debugは加えて `_DEBUG`）。Release: `_WINDOWS` と NDEBUG のみで、Editor は含まれない。
- BuildConfig.h（Application層）が定義の関係を検査する。ENABLE_EDITOR には USE_IMGUI が必須で、崩れるとコンパイルエラー。
- ImGui API を叩くだけの箇所は `#ifdef USE_IMGUI`、エディタ機能そのものは `#ifdef ENABLE_EDITOR`。

## 構成
Editor（Editor.h/.cpp）が統括します。ウィンドウをまたぐ判断だけを持ち、UIは各クラスへ委譲します。



- Editor … 選択中オブジェクト、再生状態、入力先（Game / DebugCamera / None）、フレーム境界（ImGuiのBegin/End、DockSpace）、RTの作り直し。
- ImGui/ … ImGuiManager（D3D12版の初期化、フレーム管理、テーマ、UIスケール、DPI追従）、ImGuiCustomWidget（Drag / Slider / Vector / Transform / Color / AssetRefCombo など）、Icons。
- Viewport/ … SceneWindow（エディタカメラ視点。RT、EditorCamera、GizmoSettings、ImGuizmo）、GameWindow（ゲームカメラ視点。ProjectSettings のアスペクト比でレターボックス）。
- Camera/ … EditorCamera（右ドラッグ中のWASD+QEで移動。シーンには属さない）。
- PlayMode/ … PlayModeController（Edit / Play / Paused、JSONスナップショット、コマ送り）。
- ToolBar/ MenuBar/ … 再生操作、File / Edit メニュー、確認ダイアログ、Ctrl+S、設定ウィンドウの起動。
- Inspector/ … SceneInspector（Hierarchy と Inspector。型を知らず、TypeRegistry から引く）、PropertyDrawer（PropertyType ごとの描画switchはここだけ）、MaterialEditor、ModelEditor、InspectorMaterialSection。
- Settings/ … EditorPreferences（個人設定。UserSettings/EditorPreferences.json）、EditorCache（前回終了時の状態。UserSettings/EditorCache.json）、SettingsWindow（ProjectSettings と Preferences のウィンドウ）。
- Log/ … LogWindow。Input/ … InputVisualizerWindow。
- EditorDrawContext … 描画関数が共通で使う参照の束（device、shaderLibrary、各マネージャ、assetDatabase、allowAssetEditing）。

## 依存関係
規則（Overview.mdの階層）:
- 自分以下の層にのみ依存する。Application層はこの層の上。

現状の実装:
- Platform、Graphics、Asset、Render、Scene、Foundation の各層と、ImGui / ImGuizmo / IconFontCppHeaders（externals）。
- ゲーム固有のクラス（Game/）は知らない。

規則との食い違い（現状の実装）:
- Application層への依存: ほぼすべてのヘッダが Application/BuildConfig.h をincludeしている（ビルド構成フラグ）。加えて Editor.h、MenuBar.cpp、SettingsWindow.cpp、GameWindow.cpp、SceneWindow.cpp が Application/ProjectSettings.h を使う。BuildConfig と ProjectSettings は Application フォルダにあるが、実質は共有設定の置き場になっている。

## 規則

### 所有と状態

- Editor は Application が所有する。破棄は宣言と逆順なので、graphics_ より後に宣言する（ImGui のシャットダウンと RT の解放をデバイス破棄より先に走らせるため）。
- EditorPreferences と EditorCache は、ウィンドウ生成より前に読む必要があるため Application が持つ。
- 各ウィンドウが持つのはそのウィンドウ固有の状態だけ。選択中オブジェクトと再生状態のようなウィンドウ横断の状態は Editor が持ち、引数（参照）で渡す。

### 描画と入力

- RT の作り直しは Editor が順番を握る。2枚のRTの遅延リサイズ要求を取り出してから、WaitForGPU を1回だけ行い、深度バッファは2ビューの最大サイズに合わせる。
- 入力先は前フレームのフォーカスで確定する（1フレーム遅延）。Game 選択中だけゲームへ入力を通し、DebugCamera 選択中だけデバッグカメラを動かす。

### シーンと保存

- Play 中の状態は Stop で巻き戻る。Stop / New / Open でシーンが作り直されるため、既存の GameObjectId が全て無効になり、選択を解除する。Play 中はファイル操作を止める。
- 保存の方針: ProjectSettings は Save ボタンでのみ書く（共有データ）。EditorPreferences は変更した瞬間に書く。EditorCache は終了時に1回だけ書く。UserSettings/ は gitignore 対象。
- ウィンドウ矩形は枠を含むウィンドウ矩形で保存する（クライアント矩形ではない）。
- マテリアル編集は MarkDirty を立てるだけで、書き戻しは Ctrl+S（MaterialManager::SaveDirtyMaterials）が行う。保存先は Material の assetPath_ で、パスをUI側で組み立てない。Embedded は読み取り専用。

### Inspector

- Inspector はコンポーネントの型を知らない。コンポーネントを追加しても SceneInspector / PropertyDrawer は触らない。PropertyType を増やしたときだけ PropertyDrawer とシリアライザの switch に追加する。
- SceneWindow のギズモは、ビューポート画像の直後に ImGuizmo::SetDrawlist を呼ぶ（呼ばないと別ウィンドウの上に描かれる）。

## 未使用・要確認
- InputVisualizerWindow と DrawModelList は、Editor.cpp の DrawEditorUI から呼ばれていない（ウィンドウは定義のみ）。
- EditorDrawContext のコメントは「allowAssetEditing を毎フレーム Editor が更新する」と書いているが、Editor 側でこのフラグを書き換えている箇所は見当たらない（常に true のまま）。意図した実装が未完成か、要確認。
- Editor.cpp の DrawEditorUI で ImGui::ShowDemoWindow() を毎フレーム呼んでいる（開発用の残りの可能性）。
