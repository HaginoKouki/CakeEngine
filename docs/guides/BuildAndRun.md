# ビルドと実行

CakeEngine をローカルでビルドして動かすまでの手順と、実行時の前提をまとめます。手順はリポジトリ内のプロジェクト設定と CI の設定から確認できる範囲で書いています。

## 必要なもの

| 項目 | 内容 | 根拠 |
| --- | --- | --- |
| OS | Windows 10 以降（x64） | Win32、DirectX 12、XInput、XAudio2 を使う |
| IDE | Visual Studio 2026 | `CakeEngine.vcxproj` のプラットフォームツールセットが v145。CI は `windows-2025-vs2026` ランナーを使う |
| Windows SDK | 10.0（インストール済みの最新） | `WindowsTargetPlatformVersion` が 10.0。DXC の DLL を SDK からコピーする |
| ワークロード | C++ によるデスクトップ開発 | MSBuild と MSVC |

ソリューションは `.slnx` 形式です。古い Visual Studio では開けません。

## ビルド

Visual Studio で `project/CakeEngine.slnx` を開き、構成とプラットフォームを選んでビルドします。コマンドラインからビルドする場合は、CI と同じコマンドを使います。

```bash
msbuild project/CakeEngine.slnx /p:Platform=x64,Configuration=Debug
```

| 構成 | 用途 | 定義 |
| --- | --- | --- |
| Debug | エディタ付きで開発する | `_DEBUG`、`_WINDOWS`、`ENABLE_EDITOR`、`USE_IMGUI` |
| Release | ゲームだけを動かす | `NDEBUG`、`_WINDOWS` |

ソリューションには `CakeEngine` と `externals/DirectXTex/DirectXTex_Desktop_2022_Win10.vcxproj` の 2 プロジェクトが入っています。DirectXTex は静的ライブラリとして一緒にビルドされます。

ビルド後イベントで、Windows SDK の `dxcompiler.dll` と `dxil.dll` を出力フォルダへコピーします。HLSL は起動時に DXC で実行時コンパイルされるため、この 2 つが無いと起動できません。

### 生成物の場所

| 種類 | 場所 |
| --- | --- |
| 実行ファイル | `generated/outputs/<構成>/CakeEngine.exe` |
| 中間ファイル | `generated/obj/<プロジェクト名>/<構成>/` |

`generated/` は `.gitignore` の対象です。

## 実行

実行時のカレントディレクトリは `project/` を前提にしています。コード内のパスはすべて `project/` からの相対パスです（`Assets/`、`EngineResources/`、`ProjectSettings/ProjectSettings.json`）。Visual Studio からデバッグ実行すると、既定の作業ディレクトリがプロジェクトフォルダになるため、そのまま動きます。実行ファイルを直接起動する場合は、カレントディレクトリを `project/` にしてください。

| 構成 | 起動後の動き |
| --- | --- |
| Debug | エディタが開く。前回開いていたシーン（`UserSettings/EditorCache.json`）があればそれを、無ければ `ProjectSettings` の `startupScene` を読み込む |
| Release | `startupScene` を読み込み、メインカメラの視点でゲームを描く。`fullscreen` が true ならボーダレス全画面で起動する |

### 実行時に作られるフォルダとファイル

| 場所 | 内容 | Git |
| --- | --- | --- |
| `project/logs/<日付>/` | `DebugLog` のログファイル | コミットしない |
| `project/Dumps/` | クラッシュ時のミニダンプ（`.dmp`） | コミットしない |
| `project/UserSettings/` | `EditorPreferences.json`、`EditorCache.json` | コミットしない |
| `project/ProjectSettings/ProjectSettings.json` | プロジェクト設定。無ければエディタ構成の初回起動で作られる | コミットする |
| `project/Assets/**/*.meta` | アセットごとの GUID。無ければ起動時に発行される | コミットする |
| `project/imgui.ini` | ImGui のウィンドウ配置 | コミットしない |

### エディタの基本操作

| 操作 | 内容 |
| --- | --- |
| Scene ウィンドウで右ドラッグ + WASD、QE | エディタカメラの移動と回転 |
| W、E、R | 選択中オブジェクトの移動、回転、スケールの操作に切り替える |
| Toolbar の Play、Pause、Step、Stop | 再生の制御。Stop で Play 前の状態に戻る |
| Ctrl+S | シーンと未保存のマテリアルを保存する |
| Hierarchy でクリック | オブジェクトを選択する。Inspector に内容が出る |
| Inspector の Add Component | コンポーネントを追加する |

## CI

`.github/workflows/` に、`main` への push で走るワークフローがあります。

| ワークフロー | ランナー | 内容 |
| --- | --- | --- |
| `DebugBuild.yml` | `windows-2025-vs2026` | Debug 構成を msbuild でビルドする |
| `ReleaseBuild.yml` | `windows-2025-vs2026` | Release 構成を msbuild でビルドする |
| `CheckUnwantedFiles.yml` | `ubuntu-latest` | `.pdb`、`.dmp`、`generated/`、`.vs/` などの混入を検索し、見つかれば失敗する |
| `Development.yml` | `windows-2025-vs2026` | Development 構成をビルドする。対応する構成はプロジェクトから削除済みで、このファイルだけが残っている |

CI がビルドするのは C++ だけです。HLSL は実行時にコンパイルされるため、シェーダーの誤りは CI では検出されません。

## うまく動かないとき

| 症状 | 確認すること |
| --- | --- |
| 起動直後に落ちる、または `assert` で止まる | シェーダーのコンパイルエラーの可能性がある。`project/logs/` の最新ログを見る。Debug 構成では `ShaderCompiler.cpp` の `assert` で止まる |
| アセットが見つからない | カレントディレクトリが `project/` になっているか確認する |
| モデルやテクスチャがマゼンタで表示される | 読み込みに失敗している。エラー用のマテリアルやテクスチャで描かれている状態なので、ログでパスを確認する |
| `.meta` が大量に生成される | `Assets/` に新しいファイルを置くと起動時に発行される。発行された `.meta` はコミットする |
