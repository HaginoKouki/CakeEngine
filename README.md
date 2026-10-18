[![Debug build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml)
[![Release build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml)
<br>
[![Windows](https://custom-icon-badges.herokuapp.com/badge/Windows-1BB2E4.svg?logo=Windows&logoColor=white)]()

# CakeEngine
*It's a piece of cake!*

CakeEngine は、**「プロトタイプ作成から本番環境までをもっと簡単に」** をテーマに開発中の、個人製作ゲームエンジンです。<br>
> [!NOTE]
> 学習を目的に学生個人で開発しており、機能や設計は今後変更される可能性があります。<br>

## できること
| 分野                   | 現在の機能                                                                                                                 |
| :--------------------- | :------------------------------------------------------------------------------------------------------------------------- |
| シーン                 | GameObject と親子階層、データだけを持つコンポーネント、JSON（`.scene`）での保存と復元                                     |
| コンポーネント         | Transform、Camera、Light、MeshRenderer、球と箱の Collider、パーティクル（System、Renderer、Force Field）。ゲーム側で自由に追加できる |
| 描画                   | DirectX 12。モデル描画、パノラマ Skybox、ビルボードのパーティクル（インスタンシング）、デバッグ用の線（ギズモ）           |
| アセット               | OBJ と MTL、テクスチャ（PNG、JPG、DDS など）、`.mat` マテリアル。GUID と `.meta` による参照                                |
| エディタ（Debug 構成） | Scene と Game の 2 ビュー、Hierarchy と Inspector、Play / Pause / Step / Stop、マテリアル編集、ログ、プロファイラ          |
| 入力と音               | キーボード、マウス（Raw Input）、ゲームパッド（XInput）、XAudio2 による再生                                                |

## 内部構造
エンジンは依存関係を基準に８つの層に分類されています。上の層が下の層を使います。<br>
| 層                                                      | 役割                                                 |
| :------------------------------------------------------ | :--------------------------------------------------- |
| [Application](docs/internal/layers/ApplicationLayer.md) | 起動、各層の所有、メインループ、ゲームコードとの接点 |
| [Editor](docs/internal/layers/EditorLayer.md)           | ImGui による編集 UI（Debug 構成のみ）                |
| [Scene](docs/internal/layers/SceneLayer.md)             | GameObject、コンポーネント、System、シリアライズ     |
| [Render](docs/internal/layers/RenderLayer.md)           | 描画パス、Renderer、CameraView                       |
| [Asset](docs/internal/layers/AssetLayer.md)             | モデル、テクスチャ、マテリアル、AssetDatabase        |
| [Graphics](docs/internal/layers/GraphicsLayer.md)       | DirectX 12 のデバイス、コマンド、シェーダー、PSO     |
| [Platform](docs/internal/layers/PlatformLayer.md)       | 時間、入力、音、ファイルダイアログ                   |
| [Foundation](docs/internal/layers/FoundationLayer.md)   | 数学、識別子、ログ、シリアライズ、リフレクション     |

層ごとのコードは `project/Engine/<層名>/` にあり、名前空間は `Cake` です。ゲーム固有のコードは `project/Game/` に置き、`GameModule.cpp` の 2 つの関数（シェーダーの登録、コンポーネントの登録）でエンジンに接続します。<br>
依存の規則、起動から 1 フレームまでの流れ、コーディング規約と設計規則は [内部設計](docs/internal/InternalDesign.md) にまとめています。<br>

## フォルダ構成
| 場所                                             | 内容                                                                                                  |
| :----------------------------------------------- | :---------------------------------------------------------------------------------------------------- |
| `project/CakeEngine.slnx`、`CakeEngine.vcxproj`  | Visual Studio のソリューションとプロジェクト                                                          |
| `project/main.cpp`                               | エントリポイント                                                                                      |
| `project/Engine/`                                | エンジン本体。層ごとのフォルダ                                                                        |
| `project/Game/`                                  | ゲーム固有のコード（コンポーネント、シェーダー登録）                                                  |
| `project/Assets/`                                | ゲームのアセット（モデル、テクスチャ、マテリアル、シーン、ゲーム用シェーダー）。`AssetDatabase` が走査する |
| `project/EngineResources/`                       | エンジンが使うシェーダー、フォント、テクスチャ                                                        |
| `project/ProjectSettings/`                       | プロジェクト設定（`ProjectSettings.json`）                                                            |
| `project/externals/`                             | 外部ライブラリ（ImGui、ImGuizmo、DirectXTex、nlohmann/json など）                                     |
| `docs/internal/`                                 | 内部設計と層ごとの設計書                                                                              |
| `docs/review/`                                   | コードレビューの結果                                                                                  |
| `.github/workflows/`                             | CI（Debug と Release のビルド、不要ファイルの検査）                                                    |

## ビルドと実行
Visual Studio 2026（プラットフォームツールセット v145）と Windows SDK 10.0 が必要です。`project/CakeEngine.slnx` を開き、x64 の Debug または Release をビルドします。コマンドラインからは CI と同じコマンドでビルドできます。<br>

```bash
msbuild project/CakeEngine.slnx /p:Platform=x64,Configuration=Debug
```

| 構成    | 内容                                                           |
| :------ | :------------------------------------------------------------- |
| Debug   | エディタ付き。`ENABLE_EDITOR` と `USE_IMGUI` が定義される      |
| Release | ゲームのみ。ImGui はビルドされず、`ProjectSettings` の `fullscreen` が有効になる |

実行ファイルは `generated/outputs/<構成>/` に出力されます。コード内のパスはすべて `project/` からの相対パスなので、実行時のカレントディレクトリは `project/` にしてください。Visual Studio からデバッグ実行すればそのまま動きます。<br>
HLSL は起動時に DXC で実行時コンパイルされるため、ビルド後イベントでコピーされる `dxcompiler.dll` と `dxil.dll` が出力フォルダに必要です。CI は C++ しかビルドしないので、シェーダーの誤りは起動して確認します。<br>

実行時に作られるものは次のとおりです。
| 場所                                        | 内容                                                | Git           |
| :------------------------------------------ | :-------------------------------------------------- | :------------ |
| `project/logs/<日付>/`                      | ログファイル                                        | コミットしない |
| `project/Dumps/`                            | クラッシュ時のミニダンプ                            | コミットしない |
| `project/UserSettings/`                     | エディタの個人設定と前回の状態                      | コミットしない |
| `project/imgui.ini`                         | ImGui のウィンドウ配置                              | コミットしない |
| `project/ProjectSettings/ProjectSettings.json` | プロジェクト設定。無ければエディタの初回起動で作られる | コミットする   |
| `project/Assets/**/*.meta`                  | アセットごとの GUID。無ければ起動時に発行される     | コミットする   |

### エディタの基本操作
| 操作                                        | 内容                                                               |
| :------------------------------------------ | :----------------------------------------------------------------- |
| Scene ウィンドウで右ドラッグ + WASD、QE     | エディタカメラの移動と回転                                         |
| W、E、R                                     | 選択中オブジェクトの移動、回転、スケールの操作に切り替える         |
| Toolbar の Play、Pause、Step、Stop          | 再生の制御。Stop で Play 前の状態に戻る                            |
| Ctrl+S                                      | シーンと未保存のマテリアルを保存する                               |
| Hierarchy でクリック、右クリック、ドラッグ  | 選択、生成・複製・削除、親子の張り替え                             |
| Inspector の Add Component                  | コンポーネントを追加する                                           |

### うまく動かないとき
| 症状                                     | 確認すること                                                                                                   |
| :--------------------------------------- | :------------------------------------------------------------------------------------------------------------- |
| 起動直後に落ちる、`assert` で止まる      | シェーダーのコンパイルエラーの可能性がある。`project/logs/` の最新ログを見る                                   |
| アセットが見つからない                   | カレントディレクトリが `project/` になっているか確認する                                                       |
| モデルやテクスチャがマゼンタで表示される | 読み込みに失敗している。エラー用のマテリアルやテクスチャで描かれている状態なので、ログでパスを確認する         |
| `.meta` が大量に生成される               | `Assets/` に新しいファイルを置くと起動時に発行される。発行された `.meta` はコミットする                        |

## コードレビュー
このプロジェクトは Claude のスケジュール機能を使用し、定期的にコードレビューを行っています。<br>
レビューの過程で出力されるファイルは `docs/review/` に保存されています。<br>
| ファイルパス                                 | 内容                                                                 |
| :------------------------------------------- | :------------------------------------------------------------------- |
| [docs/review/issue.md](docs/review/issue.md) | レビューにより見つかった、修正されていない問題点が記載されています。 |
| [docs/review/history/](docs/review/history/) | 各回のレビュー記録（`YYYY-MM-DD.md`）が保存されています。            |

## 開発の状態
- Windows 専用です。Platform 層と Graphics 層が Win32 と DirectX 12 に直接依存しています。
- アセットのインポートパイプライン（`Library/` キャッシュ）は型定義のみで、まだ動いていません。モデルの読み込みは OBJ のみ対応です。
- 設計書との食い違いが見つかった場合は、実装に合わせて設計書を更新してください。
