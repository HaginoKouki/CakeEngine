[![Debug build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml)
[![Release build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml)

# CakeEngine

*It's a piece of cake!*

CakeEngine は、プロトタイプの作成からゲームの実行までを簡単にすることを目指して開発している、C++ と DirectX 12 のゲームエンジンです。学習を目的に個人で開発しており、機能や設計は今後変わる可能性があります。

## できること

| 分野 | 現在の機能 |
| --- | --- |
| シーン | GameObject と親子階層、データだけを持つコンポーネント、JSON（`.scene`）での保存と復元 |
| コンポーネント | Transform、Camera、Light、MeshRenderer、球と箱の Collider、パーティクル（System、Renderer、Force Field）。ゲーム側で自由に追加できる |
| 描画 | DirectX 12。モデル描画、パノラマ Skybox、ビルボードのパーティクル（インスタンシング）、デバッグ用の線（ギズモ） |
| アセット | OBJ と MTL、テクスチャ（PNG、JPG、DDS など）、`.mat` マテリアル。GUID と `.meta` による参照 |
| エディタ（Debug 構成） | Scene と Game の 2 ビュー、Hierarchy と Inspector、Play / Pause / Step / Stop、マテリアル編集、ログ、プロファイラ |
| 入力と音 | キーボード、マウス（Raw Input）、ゲームパッド（XInput）、XAudio2 による再生 |

## 構造

機能は 8 つの層に分かれ、上の層が下の層を使います。

```
Application   起動、各層の所有、メインループ、ゲームコードとの接点
Editor        ImGui による編集 UI（Debug 構成のみ）
Scene         GameObject、コンポーネント、System、シリアライズ
Render        描画パス、Renderer、CameraView
Asset         モデル、テクスチャ、マテリアル、AssetDatabase
Graphics      DirectX 12 のデバイス、コマンド、シェーダー、PSO
Platform      時間、入力、音、ファイルダイアログ
Foundation    数学、識別子、ログ、シリアライズ、リフレクション
```

層ごとのコードは `project/Engine/<層名>/` にあり、名前空間は `Cake` です。ゲーム固有のコードは `project/Game/` に置き、`GameModule.cpp` の 2 つの関数（シェーダーの登録、コンポーネントの登録）でエンジンに接続します。

依存の規則と実装の現状、起動から 1 フレームまでの流れは [docs/design/Overview.md](docs/design/Overview.md) にまとめています。

## フォルダ構成

| 場所 | 内容 |
| --- | --- |
| `project/CakeEngine.slnx`、`CakeEngine.vcxproj` | Visual Studio のソリューションとプロジェクト |
| `project/main.cpp` | エントリポイント |
| `project/Engine/` | エンジン本体。層ごとのフォルダ |
| `project/Game/` | ゲーム固有のコード（コンポーネント、シェーダー登録） |
| `project/Assets/` | ゲームのアセット（モデル、テクスチャ、マテリアル、シーン、ゲーム用シェーダー）。`AssetDatabase` が走査する |
| `project/EngineResources/` | エンジンが使うシェーダー、フォント、テクスチャ |
| `project/ProjectSettings/` | プロジェクト設定（`ProjectSettings.json`） |
| `project/externals/` | 外部ライブラリ（ImGui、ImGuizmo、DirectXTex、nlohmann/json など） |
| `docs/` | 設計書、手順書、レビュー記録 |
| `.github/workflows/` | CI（Debug と Release のビルド、不要ファイルの検査） |

## ビルドと実行

Visual Studio 2026 と Windows SDK 10.0 が必要です。`project/CakeEngine.slnx` を開き、x64 の Debug または Release をビルドします。

```bash
msbuild project/CakeEngine.slnx /p:Platform=x64,Configuration=Debug
```

| 構成 | 内容 |
| --- | --- |
| Debug | エディタ付き。`ENABLE_EDITOR` と `USE_IMGUI` が定義される |
| Release | ゲームのみ。ImGui はビルドされない |

実行ファイルは `generated/outputs/<構成>/` に出力されます。実行時のカレントディレクトリは `project/` を前提にしているため、Visual Studio からデバッグ実行するか、`project/` をカレントにして起動してください。詳細は [docs/guides/BuildAndRun.md](docs/guides/BuildAndRun.md) にあります。

## 最初に読む順番

1. [docs/design/Overview.md](docs/design/Overview.md) で層の並びと、起動から描画までの流れをつかむ。
2. [docs/design/SceneLayer.md](docs/design/SceneLayer.md) でコンポーネントと System の分け方を読む。ほかの層はこの考え方の上に乗っている。
3. [docs/guides/AddingComponents.md](docs/guides/AddingComponents.md) で、実際に 1 つコンポーネントを足してみる。
4. 気になる層のページと [docs/design/Conventions.md](docs/design/Conventions.md) を読む。

ドキュメント全体の一覧は [docs/README.md](docs/README.md) にあります。

## 開発の状態

- Windows 専用です。Platform 層と Graphics 層が Win32 と DirectX 12 に直接依存しています。
- アセットのインポートパイプライン（`Library/` キャッシュ）は型定義のみで、まだ動いていません。モデルの読み込みは OBJ のみ対応です。
- 既知の問題と設計上の要確認事項は [docs/review/README.md](docs/review/README.md) にまとめています。設計書との食い違いが見つかった場合は、実装に合わせて設計書を更新してください。
