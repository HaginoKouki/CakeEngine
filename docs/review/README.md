# レビューの運用

CakeEngine では、Claude に定期的なコードレビューとドキュメント整備を任せています。このページでは、レビューの進め方と、過去のレビューで見つかって未解決のままの事項をまとめます。

## 進め方

| 項目 | 内容 |
| --- | --- |
| プロンプト | [ReviewPrompt.md](ReviewPrompt.md) を Claude に渡す。冒頭の「運用設定」をその回の条件に合わせて書き換える |
| 前回の時点 | 直前のレビュー記録の冒頭に書かれたコミットハッシュを起点にし、`git log <ハッシュ>..HEAD` で差分を見る |
| 記録の保存先 | `docs/review/YYYY-MM-DD.md`。1 回のレビューで 1 ファイル |
| 設計書の更新 | 実装に合わせて `docs/design/` と `docs/guides/` を更新する。履歴や日付は設計書に書かず、記録に書く |
| 未解決事項 | このページの一覧を更新する。解消したものは「解消」に変え、新規のものを追加する |
| ソースコード | レビューで問題を見つけても、依頼が無い限りソースコードとコメントは変更しない。記録に書く |

## レビュー記録

| 日付 | 対象 | 主な内容 |
| --- | --- | --- |
| [2026-10-04](2026-10-04.md) | パーティクルの改修 | 粒数の上限が無い点を High として指摘 |
| [2026-10-11](2026-10-11.md) | パーティクルの拡張、Gradient、CI の追加 | 発生ロジックの無限ループ（Critical）、Development 構成のビルド失敗、CheckUnwantedFiles の不具合 |
| [2026-10-16](2026-10-16.md) | Force Field、`SetCamera`、Blinn-Phong、Development 構成の削除 | `Object3d` シェーダーがコンパイルできない点を Critical として指摘 |

## 未解決事項

2026-10-18 時点の `main`（`607b5a3`）で、ソースを読んで状態を確認した一覧です。「確定」はコードから問題が確定しているもの、「要確認」は仕様か不具合かを人が判断する必要があるものです。

### 実装の問題（確定）

| 番号 | 初出 | 優先度 | 対象 | 内容 | 状態 |
| --- | --- | --- | --- | --- | --- |
| I-1 | 2026-10-16 | Critical | `Assets/Shaders/Object3D/Object3d.PS.hlsl`、`Object3d.VS.hlsl` | PS の `lightDirection` が未宣言。VS の `inverse()` は HLSL に存在しない。起動時の DXC コンパイルで失敗する。CI は C++ しかビルドしないため検出されない | 継続 |
| I-2 | 2026-10-11 | High | `Graphics/Pipeline/PipelineState.cpp` の `GetBlendDesc` | `kBlendModeMultiply` に `break` が無く `kBlendModeScreen` へフォールスルーする。Multiply を使うシェーダーは現状無い | 継続 |
| I-3 | 2026-10-04 | High | `ParticleSystemComponent`、`Renderer::DrawModelInstanced`、`FrameConstantBuffer::Allocate` | 粒数に上限が無い。`burstCount` は下限 1 のみ。`Allocate` は 1 回の要求が容量（1 MiB）を超える場合を検出せず、`memcpy` が確保域を超える | 継続 |
| I-4 | 2026-10-11 | Medium | `ParticleSystemComponent.h`、`ParticleSystem.h`、`ParticleStorage.h` のコメント | 旧仕様（`color`、`emitInterval`、`lifeTime`、「emitInterval 秒ごとに 1 つ生成」）のまま | 継続 |
| I-5 | 2026-10-16 | Low | `Render/Renderer.cpp` の `CameraForGPU` のコメント | 「Standard.PS.hlsl の Camera と同じ並びにすること」とあるが、`Standard.PS.hlsl` に `Camera` は無く、読んでいるのは `Object3d.PS.hlsl` | 継続 |
| I-6 | 2026-10-16 | Low | `EngineResources/fonts/HackGen/` | 10.7 MB のフォントにライセンスファイルが同梱されていない（Inter と NotoSansJP には `OFL.txt` がある） | 継続 |
| I-7 | 2026-10-11 | Low | `Render/Renderer.cpp`、`EngineResources/textures/` | コードは `Skybox.png`、実ファイルは `skybox.png`。旧 `Sky.png` と `Sky.png.meta` が残っている | 継続 |
| I-8 | 2026-10-04 | Low | `Scene/System/Particle/ParticleSystem.cpp` | コメント行のインデントに空白が混在している（他はタブ） | 継続 |
| I-9 | 2026-10-18 | Low | `Graphics/Buffer/GPUResourceUtility.cpp` | Asset 層の `Material.h` を include しているが使っていない | 新規（設計書には以前から記載） |
| I-10 | 2026-10-18 | Low | `Foundation/Debug/DebugLog.h`、`Foundation/Identity/AssetId.h` のコメント | 存在しない「JobSystem」「AssetPipeline 層」を参照している | 新規 |

### 設計上の要確認事項

| 番号 | 初出 | 対象 | 確認したい内容 | 選択肢と影響 |
| --- | --- | --- | --- | --- |
| D-1 | 2026-10-18 | Render 層と Scene 層 | 相互参照をどう扱うか。Render のパスが Scene の System を呼び、Scene の System が Render の型を使う | (a) System を Render 層へ移す (b) パスが受け取る「描くもの」を Scene が用意して渡す形にする (c) 現状を許容し、ヘッダ単位の循環だけ禁止する。どれを選ぶかで設計書の層の順序が変わる |
| D-2 | 2026-10-18 | Asset 層と Graphics 層 | 層の順序。以前の設計書は Asset を Graphics の下に置いていたが、`Asset.h` のコメントと実装は Asset → Graphics | 実装どおり Asset を上とするなら、設計書は本レビューで更新済み。Graphics を上にするなら、`TextureManager` と `Material` が Graphics の型を使わない形へ変える必要がある |
| D-3 | 2026-10-18 | Foundation 層の `TypeInfo.h` | Asset 層と Scene 層のヘッダを include している。`PropertyTypeOf` の特殊化のため | 特殊化を各層側（`AssetRef.h`、`GameObjectId.h`）へ移すと Foundation が閉じる。`PropertyType` の列挙自体は Foundation に残せる |
| D-4 | 2026-10-18 | `BuildConfig.h`、`ProjectSettings.h` | Application フォルダにあるが、Graphics 層と Editor 層から参照されている | 共有設定として Foundation か専用フォルダへ移すか、現状を許容するか |
| D-5 | 2026-10-18 | `Scene/System/GizmoSystem.h` | `GizmoSettings` のために ImGui と ImGuizmo を無条件に include している | `ImGuizmo::MODE` と `OPERATION` をエンジン側の列挙に置き換えれば Scene 層が ImGui から離れる |
| D-6 | 2026-10-16 | `Game/GameModule.cpp` の `shininess` | Blinn-Phong の指数が UI 上 0 から 1 にしか設定できない。`pow(x, 1 以下)` ではハイライトが広がり、0 では未定義になる | 一般的な範囲（1 から 128 など）に変えるか、意図した表現か |
| D-7 | 2026-10-11 | `ParticleSystemComponent::startSpeed` | 各軸 ±1 の乱数ベクトルに掛けるため、速さは 0 から √3 倍でばらつく。コメントは「m/s」 | 方向を正規化して速さにするか、コメントを「各軸のスケール」に直すか |
| D-8 | 2026-10-16 | `ParticleForceFieldComponent` | すべてのエミッターに効き、絞り込みが無い | コライダーの `layer` と `mask` のような仕組みを入れるか。入れる前に適用順と合算の規則を決める |
| D-9 | 2026-10-16 | `Scene/Serialize/SceneSerializer.cpp` の `ReadProperty` | `Int` は `is_number_integer()` のときだけ代入し、小数で保存された値は既定値になる | 丸めて読むか、警告を出すか、現状の「読めない値は既定値」を維持するか |
| D-10 | 2026-10-16 | `externals/imgradient/` と `externals/imgui/ImGradient.*` | 用途の重なる 2 つのライブラリがあり、`imgradient` は未使用 | どちらを Gradient 編集に使うか。未使用のまま残すか |
| D-11 | 2026-10-16 | `.github/workflows/Development.yml` | 対応する構成が無いのにファイルが残っている | 削除してよいか |
| D-12 | 2026-10-11 | `EngineResources/textures/` | `Sky.png.meta` と `uvChecker.png.meta` がある。`EngineResources` は `AssetDatabase` の走査対象外で、`skybox.png` には `.meta` が無い | `EngineResources` に `.meta` を置く方針があるか |
| D-13 | 2026-10-18 | `AssetDatabase::DetectType` | `.gltf`、`.glb`、`.fbx` を索引に載せるが、`ModelManager` は OBJ しか読めない | 対応予定の先行登録か、対象から外すか |
| D-14 | 2026-10-18 | `AssetDatabase.h` の `AssetType` と `AssetTypes.h` の `ImporterType` | ファイル分類の列挙が 2 つ並存している | インポートパイプライン接続時に `ImporterType` へ統合する予定か |
| D-15 | 2026-10-18 | `Editor/EditorDrawContext.h` の `allowAssetEditing` | コメントは「毎フレーム Editor が更新する」とあるが、書き換える箇所が無く常に `true` | Play 中にアセット編集を止める実装を入れるか、コメントを直すか |

### 情報として記録しているもの

| 番号 | 対象 | 内容 |
| --- | --- | --- |
| N-1 | `Editor/Editor.cpp` の `DrawEditorUI` | `ImGui::ShowDemoWindow()` を毎フレーム呼んでいる |
| N-2 | `Editor/Input/InputVisualizerWindow`、`ModelEditor::DrawModelList`、`ImGuiManager::GetMainFont`、`Renderer::DrawSprite` | 定義されているが呼び出し元が無い |
| N-3 | `Scene/Object/ParticleStorage.h` の `Particle::rotate`、`scale` | System が変更しないため常に 0 と (1, 1)。将来の予約枠と読める |
| N-4 | `Asset/Import/`、`Asset/Artifact/`、`Asset/Runtime/HandlePool.h`、`AssetTypes.h` の `ObjectType` 以降 | 型定義のみで未接続 |
| N-5 | `ParticlePass` | エミッター順に描くだけでソートが無い。複数エミッターや半透明の重なりで前後関係が崩れうる |
| N-6 | CI | HLSL は実行時コンパイルのため CI で検証されない。`dxc` を Windows ランナーで走らせれば検証できる |

### 解消済み

| 初出 | 内容 | 解消した変更 |
| --- | --- | --- |
| 2026-10-11 | パーティクルの発生間隔が 0 になり無限ループする | 下限 `kMinEmitInterval` と `burstCount` の下限 1 を追加 |
| 2026-10-11 | Emission 無効中もタイマーが溜まり、再開時にまとめて出る | 無効中はタイマーを進めない |
| 2026-10-11 | `burstCount` が `float` | `int` に変更 |
| 2026-10-11 | Development 構成がコンパイルできない | Development 構成を削除 |
| 2026-10-11 | `CheckUnwantedFiles.yml` のスクリプトが常に失敗する | スクリプトを修正。その後 `.pdb` の混入を実際に検出した |
