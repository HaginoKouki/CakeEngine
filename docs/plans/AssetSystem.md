# アセットシステム 設計書とロードマップ（第2版）

アセットシステムを作り直すための設計と計画。実装済みの内容は docs/internal/ の各層のページに書く。

## 1. 概要

アセットシステムを「元ファイル側（エディタ専用）」と「実行時側（Debug と Release で共通）」に分け、両者を「成果物」と「対応表」だけでつなぐ。Debug では元ファイル側が Library に成果物を作り、Release はエディタが出力したパッケージだけを読む。

```
[元ファイル側] AssetPipeline.lib（Debug でだけリンク）
  元ファイルと .meta ──→ SourceIndex とインポータ ──→ Library
                                                        │
                         EditorArtifactProvider ←───────┤
                                   │                    ▼
                                   │              PackageBuilder
                                   │                    │
                                   │                    ▼
                                   │            Package/Assets.pak
                                   │ Debug の入口       │ Release の入口
[実行時側] Asset 層                ▼                    ▼
  AssetStorage × 6 型 ←── AssetManager ←── PackageArtifactProvider
                               ▲
                               │ 実体を引く
  AssetRef を持つもの ──→ 描画リスト ──→ Render 層のパス
```

Debug は EditorArtifactProvider、Release は PackageArtifactProvider を通して、同じ AssetManager が成果物を読む。描画リストにはハンドルだけが載り、実体は Render 層が AssetManager から引く。

**確定した判断**

- 判断1：元ファイル側と実行時側を分ける（分離型）
- 判断2：実行時側は共通の格納基盤と、型ごとの変換処理で作る
- 判断3：寿命は到達可能性による回収で決める
- 判断4：依存関係を追跡する
- 判断5：.mat と .scene も変換を通す
- 判断6：非同期を前提にする
- 判断7：テクスチャの SRV は番号をスロットごとに固定し、中身を書き換える
- 判断8：音声（AudioClip）を最初から型に入れる
- 判断9：描画は描画リストの形にそろえ、Render 層から Scene 層への参照をなくす
- 判断10：Release は最初からパッケージだけを読む。LibraryArtifactProvider は作らない
- 判断11：AssetPipeline 層を静的ライブラリに分け、Debug でだけリンクする
- 判断12：スカイボックスは、シーン設定が参照するマテリアルで決める

**運用の決定**

- テスト用のプロジェクトを作り、CI で走らせる（14章）
- この設計書をリポジトリの docs/plans/AssetSystem.md にも置く

**これまでの決定で引き続き有効なもの**：Unity と同じ操作感、全ファイル・全フォルダへの .meta の発行、参照は {GUID, LocalId}、1オブジェクト1メッシュ、MeshFilter と MeshRenderer（依存宣言つき）、アイコンはフォント、シェーダーはアセットにしない。

**用語**

- 元ファイル：Assets/ と EngineResources/ に置かれた .obj・.png・.mat・.scene・.wav など。正は常にこれ。
- 成果物：元ファイルを変換した結果。実行時側が読むのはこれだけで、捨てても再生成できる。
- オブジェクト：{GUID, LocalId} で指せる単位。1つの元ファイルから複数できる（OBJ から ModelPrefab・メッシュ・マテリアル）。
- 実体：成果物から作った実行時の値（GPU バッファを持つメッシュなど）。
- パッケージ：ビルドに含めるシーンから届く成果物だけを1つにまとめたファイル。Release が読む。
- 描画リスト：Scene 層が毎フレーム作る「描くもの」の一覧。ハンドルと行列だけを持つ。

## 2. 層構成とビルド構成

元ファイル側のために、エディタ専用の層「AssetPipeline」を新設し、静的ライブラリ AssetPipeline.lib として Debug でだけリンクする。Render 層は Scene 層を include しない形に直し、層の順序を実装と規則で一致させる。

**依存の規則**

- 層の順序（下から）：Foundation → Platform → Graphics → Asset → Render → Scene → Editor → Application。上の層が下の層を使う
- AssetPipeline → Asset・Platform・Foundation だけ。Graphics・Render・Scene は知らない（GPU に触らない規則を層で保証する）
- Editor → AssetPipeline を含む全層
- Asset・Render・Scene・Application の Release で動くコードは AssetPipeline を知らない。Application が AssetPipeline を使うのは ENABLE\_EDITOR の中だけ
- Render → Scene の include はなくす（12章）。Scene 層は Render 層の描画リストの型を使ってよい

**ビルドの単位**

- CakeEngine.vcxproj（既存）：エンジン本体とゲーム。Debug と Release
- AssetPipeline.vcxproj（新規、project/ 直下）：Engine/AssetPipeline/ のソースだけを持つ静的ライブラリ。Debug 構成でだけビルドし、CakeEngine の Debug 構成だけがリンクする。ソリューションの Release 構成ではビルドしない（slnx の書き方は着手時に確認する）
- Tests.vcxproj（新規、project/Tests/）：テスト用の exe（14章）
- CakeEngine.Common.props（新規、project/ 直下）：3つのプロジェクトで共通の設定（C++20、/utf-8、include の起点 project/、警告の扱い、構成ごとの定義）。各 vcxproj はこれを読み込み、設定のずれを防ぐ
- AssetPipeline 層のソースには ENABLE\_EDITOR の #ifdef を書かない。Release ではビルドされないため
- 実行時側のコードが AssetPipeline の関数を呼ぶと、Release のリンクで失敗して CI で分かる。ヘッダの include はビルドでは止められないので、14章の include の検査で拾う

**このついでに解消する層違反**

- Foundation 層の TypeInfo.h が Asset 層と Scene 層のヘッダを include している（フェーズ4、11章）
- Render 層と Scene 層の相互参照（フェーズ5、12章）
- Graphics 層の GPUResourceUtility.cpp が使っていない Material.h を include している（フェーズ5の削除で消える）
- BuildConfig.h と ProjectSettings.h の置き場所、GizmoSystem.h の ImGui 依存は、この設計の範囲外

**配置**

新しく作るフォルダとファイルを層ごとに並べる。ファイル単位の細かい分け方は各フェーズの着手時に決める。

- Foundation 層
  - Engine/Foundation/Identity/（既存）：AssetId（作成済み）
- Platform 層
  - Engine/Platform/Job/（新規フォルダ）：JobSystem。ワーカーの起動で COM の初期化とスレッド名の登録（Windows の API）を行うので、OS に依存しない Foundation 層ではなく Platform 層に置く。Platform が所有する
  - Engine/Platform/Sound/（既存）：SoundManager を PCM のバッファから再生する形に書き換える
- Graphics 層
  - Engine/Graphics/Upload/（新規フォルダ）：UploadQueue
  - Engine/Graphics/Device/（既存）：DeferredReleaseQueue。CommandManager にフェンス値の取得口を足す
  - Engine/Graphics/Descriptor/（既存）：SrvManager に空きリストと統計を足す（新しいクラスは作らない）
- Asset 層（実行時側）
  - Engine/Asset/AssetTypes.h（既存）：ObjectType だけを残し、AudioClip を足す
  - Engine/Asset/Core/（新規フォルダ）：AssetHandle、AssetStorage、AssetManager、AssetLoader、IArtifactProvider、AssetRef、LoadGroup
  - Engine/Asset/Artifact/（既存、中身は入れ替え）：成果物の共通形式と型ごとの形式
  - 型ごとの実体とローダー：Engine/Asset/Mesh/（新規）、Texture/（既存、入れ替え）、Material/（既存、入れ替え）、ModelPrefab/（新規）、SceneAsset/（新規）、Audio/（新規）
  - Engine/Asset/Builtin/（新規フォルダ）：内蔵アセットの生成
  - Engine/Asset/Provider/（新規フォルダ）：PackageArtifactProvider、パッケージの形式、MemoryArtifactProvider（テストとデバッグ用）
- Render 層
  - Engine/Render/SceneDrawData.h（既存フォルダ直下）：1フレーム分の描画リストの束
  - Engine/Render/Mesh/（新規フォルダ）：MeshDrawList
  - Engine/Render/Particle/（新規フォルダ）：ParticleDrawList
  - Engine/Render/Sprite/（既存）：SpriteDrawList のテクスチャを AssetHandle に変える
- Scene 層
  - Engine/Scene/SceneSettings.h（既存フォルダ直下）：シーン設定（スカイボックスのマテリアル）
  - Engine/Scene/Component/（既存）：MeshFilterComponent
  - Engine/Scene/System/Render/（既存）：CollectSceneDrawData（描画リストを積む）
  - Engine/Scene/System/（既存）：InstantiateModelPrefab、AssetRef の列挙（回収のルート）
- AssetPipeline 層（新規の層。Engine/AssetPipeline/ は新規フォルダ）
  - Source/：SourceIndex、AssetMeta と ImporterType（1-A のものを移す）
  - Import/：AssetImporter、ImportContext、ImportResult、ImportKey、ImportScheduler
  - Importers/：TextureImporter、MaterialImporter、ObjImporter、SceneImporter、AudioImporter
  - Library/：SourceDB、ArtifactStore
  - Build/：PackageBuilder
  - 直下：EditorArtifactProvider
- Editor 層
  - Engine/Editor/EditorSelection.h（既存フォルダ直下）
  - Engine/Editor/Project/（新規フォルダ）：ProjectWindow、ProjectTree、AssetIcons
  - Engine/Editor/Inspector/（既存）：AssetInspector と型ごとの描画、シーン設定の描画
  - Engine/Editor/ImGui/（既存）：AssetDragDrop、ImAssetRefField
  - Engine/Editor/Build/（新規フォルダ）：PackageBuildCommand（ProjectSettings と ShaderLibrary から入力を集めて PackageBuilder を呼ぶ）
  - Engine/Editor/Debug/（新規フォルダ）：GraphicsDebugWindow
- Application 層
  - Engine/Application/ProjectSettings（既存）：startupScene を AssetId に変え、buildScenes と alwaysIncludedAssets を足す
- 層に属さないもの
  - project/EngineResources/materials/（新規フォルダ）：DefaultSkybox.mat
  - project/Library/（新規フォルダ、バージョン管理しない）：成果物のキャッシュ
  - project/Package/（新規フォルダ、バージョン管理しない）：パッケージの出力先
  - project/Tests/（新規フォルダ）、project/externals/doctest/（新規フォルダ）
  - .github/workflows/（既存）：Tests.yml、CheckLayerIncludes.yml
  - docs/plans/（新規フォルダ）：AssetSystem.md（この設計書）
  - .gitignore（既存）：project/Library/ と project/Package/ を足す

## 3. 識別

保存データの中でアセットを指すのは AssetId（GUID と LocalId）だけで、実行中の管理にはハンドルを使う。2つは役割が違うので混ぜない。

**AssetId**（Engine/Foundation/Identity/AssetId.h、作成済み）

- { Guid guid; LocalId localId; } の 24 バイトの値型。等価比較とハッシュを持つ
- localId == 0 はファイルの主オブジェクト、それ以外はサブオブジェクト
- シーン、.mat、成果物、パッケージ、ProjectSettings に保存されるのはこれだけ

**GUID と LocalId**

- GUID はファイルとフォルダごとに1つ。.meta に保存する。生成は既存の Guid::Generate（CoCreateGuid）
- サブオブジェクトの LocalId は MakeSubAssetId(ObjectType, ファイル内の名前) で作る（1-A で実装済み。種別タグ入り）
- 名前は SubAssetNameTable で一意にする（空名は既定名、重複は \_1、\_2）。エンジンが付け直した名前（MaterialManager の "Mat.1" など）は使わない

**ObjectType（実行時側、Asset 層）**

- None、Default、Folder、ModelPrefab、Mesh、Material、Texture、Scene、AudioClip
- AudioClip は末尾に足す。種別タグは他の型と同じく固定の4文字で持ち、AssetTypes.cpp の static\_assert で固定する
- Folder と Default は元ファイル側にだけ存在し、実行時に読み込まれることはない

**ImporterType（元ファイル側、AssetPipeline 層へ移す）**

- Default、Folder、Model、Texture、Material、Scene、Audio
- AssetDatabase.h の AssetType は、AssetDatabase と一緒に消す（分類の列挙を2つ持たない）

**1つの元ファイルから出るオブジェクト**

- .obj：{G, 0} ModelPrefab、メッシュ n 個、埋め込みマテリアル m 個
- .png・.jpg など：{G, 0} Texture
- .mat：{G, 0} Material
- .scene：{G, 0} Scene
- .wav・.mp3：{G, 0} AudioClip
- フォルダ：{G, 0} Folder（成果物なし）
- 未対応の拡張子、.mtl、.hlsl：{G, 0} Default（成果物なし。.mtl は OBJ の変換時の依存になる）

**内蔵アセット**

- コードで作る常駐のアセット。コード内の定数として固定の内蔵 GUID を1つ持ち、固定の LocalId をぶら下げる
- Mesh：Triangle、Quad、Cube、Sphere、Tetrahedron。Material：Default、Error。Texture：White、Error
- AssetRef から普通のアセットと同じ形で指せる。今の kPrimitiveShapeGuids（図形ごとの GUID）は引き継がない。シーン v1 を読まないので、保存済みの参照を守る必要がないため

**EngineResources/ の資源**

- 2つ目の元ファイル置き場として SourceIndex に登録し、普通のアセットと同じく .meta を持ちインポートを通る。.meta はエンジンと一緒にコミットする（.gitignore は修正済み）
- アセットとして使うのは materials/DefaultSkybox.mat（新規）と textures/skybox.png。コードから使われていない textures/Sky.png と uvChecker.png は消す
- フォントはアセットにしない（ImGuiManager がファイルを直接読む）。シェーダーもアセットにしない（ShaderLibrary がパスで読む）。どちらも索引には Default として載る
- Project ウィンドウでは読み取り専用のルートとして見せる（13章）

**ハンドル**

- AssetHandle\<T>：{ uint32 index; uint32 generation; } の 8 バイト。T で型を区別するので、メッシュのハンドルをマテリアルとして使うとコンパイルエラーになる
- ハンドルには SRV の番号を入れない（今の TextureHandle との違い）。番号はテクスチャのスロットが持ち、ハンドルが有効な間は変わらない（9章）。描画側はハンドルからスロットを引いて番号を得る（配列の添字アクセス1回）

## 4. 元ファイル側：登録と変更検知

SourceIndex が Assets/ と EngineResources/ を走査して全ファイル・全フォルダに GUID を振り、パスと GUID の対応と親子関係を持つ。ディスクを見るのはここだけで、Project ウィンドウも Inspector の候補一覧もここを読む。

**走査の対象**

- 対象：各ルート以下の全ファイル・全フォルダ
- 除外：.meta 自身、名前が「.」で始まるもの、「\~」で終わるもの、ルートフォルダ自身（.meta の置き場所がないため）
- パスはスラッシュ区切りの相対パスで、ディスク上の綴り（大文字小文字）のまま持つ
- 今の Assets/ には .meta を除いて 559 のファイルと 20 のフォルダがある（OBJ と MTL が 179 ずつ、画像が 173）。初回の起動では、このうち変換が要るものがすべてインポートの対象になる

**エントリが持つ情報**

- GUID、パス、ImporterType、親フォルダ、子（フォルダの場合）
- .meta の内容（インポート設定）
- 変更検知用の記録（最終書き込み時刻とサイズ）
- インポート状態（未インポート／最新／要再インポート／失敗）と最後のログ
- サブオブジェクトの表（{LocalId, ObjectType, 名前} の一覧）。インポートの結果から作るので、モデルを読み込まずに中身を一覧できる

**.meta（v2、1-A で実装済み）**

- 項目：version、guid、importer、settings
- 読めない .meta の GUID は振り直さない。エラーを出してそのファイルを飛ばす
- 自分より新しい版の .meta は書き換えない
- 同じ GUID が2つ見つかった場合（.meta ごとコピーしたとき）：前回の索引に無かった方に新しい GUID を振り、警告を出す。両方とも新しい場合はパスの辞書順で後の方を振り直す。飛ばすとファイルが見えなくなるので、振り直す方を選ぶ
- 本体が無い .meta（孤立）：消さずに警告だけ出す。ブランチの切り替えの途中などで、一時的に本体が消えることがあるため。まとめて消すメニューを別に用意する

**Refresh（変更検知）**

- 走るタイミング：起動時、エディタのウィンドウにフォーカスが戻ったとき、Refresh 操作（Ctrl+R）、エディタ自身がファイルを書いた直後（通知を受けてそのファイルだけ）
- 手順：走査 → 前回の記録と比較 → 追加・削除・移動・変更に分類 → 変更されたものと、それに依存するもの（5章）を再インポートの予約へ
- 変更の判定：時刻とサイズが同じなら変更なしとみなす（毎回ハッシュを取ると遅いため）。違えば内容のハッシュを取り、インポート鍵で最終判断する
- 移動・リネーム：.meta ごと動いたなら同じ GUID が別のパスで見つかるので、移動として扱い参照は切れない。エディタの外で本体だけを動かした場合は新しい GUID になる（Unity と同じ）
- 削除：エントリを消す。その GUID を指していた参照は、Inspector に「Missing」と表示する
- 走査はワーカースレッドで行い、結果の反映だけをメインスレッドでまとめて行う
- ファイル監視（ReadDirectoryChangesW）は後回し。フォーカス復帰時の Refresh で実用上は足りる

## 5. 変換（インポート）

インポータは「元ファイルと設定から、オブジェクトごとの成果物を作る」純粋な処理で、読んだファイルと参照先を申告する。GPU には触らず、ワーカースレッドで並列に走る。

**インポータの枠組み**

- 型ごとに1クラス。持つもの：ImporterType、版数、対応する拡張子、設定の既定値と正規化、Import 本体
- 規則：グローバルな状態を持たない、GPU に触らない、スレッド安全、失敗しても落とさない（ログを出して失敗を返す）
- 既存の運用を引き継ぐ：インポータを直したら必ず版数を上げる

**ImportContext（インポータが受け取るもの）**

- 元ファイルのパス、GUID、設定
- ReadFile(path)：ファイルを読む唯一の入口。読んだファイルは自動で「変換時の依存」に記録される。存在しなかったファイルも記録する（後から作られたときに再インポートするため）
- FindAssetId(path)：元ファイル側の索引で、パスから AssetId を引く（.mtl に書かれたテクスチャなど）
- Log(警告／エラー)：Inspector にインポートのログとして出る
- Media Foundation など OS の機能を使うインポータは、使う前に ReadFile で中身を読み、メモリ上のバイト列から展開する。パスを直接 OS に渡すと、依存の記録から漏れるため

**ImportResult（インポータが返すもの）**

- オブジェクトの一覧：{LocalId, ObjectType, 名前, 成果物のバイト列, 実行時の依存（AssetId の一覧）}
- 主オブジェクト（LocalId 0）は必ず含む。失敗したときは何も出さずに失敗を返す（前回の成果物はそのまま残る）

**依存の追跡（判断4）**

- 変換時の依存：ReadFile で読んだ全ファイル（パス単位）。OBJ なら .obj 自身と .mtl。テクスチャは GUID を引くだけで中身を読まないので、含まれない
- 実行時の依存：成果物が指す他のオブジェクト（マテリアルからテクスチャ、ModelPrefab からメッシュとマテリアル、シーンからシーン内の全参照）。成果物の先頭に書く（6章）
- 逆引き表：「このパスが変わったら再インポートすべき GUID」を SourceDB に持つ。Refresh で見つかった変更から、依存している元ファイルを引く
- 今の Assets/ では .mtl 179 個のうち 177 個が Textures/colormap.png を参照している。参照は GUID で持つので、colormap.png を書き換えても OBJ の再インポートは起きず、テクスチャだけが差し替わる

**インポート鍵（既存の ArtifactKey の後継）**

- 含めるもの：インポータの種類と版数、正規化した設定のハッシュ、変換時の依存すべての（パスと内容のハッシュ）、成果物の共通形式の版数
- 一致すれば既存の成果物を使い、どれかが変われば作り直す

**Library（バージョン管理しない。消しても再生成できる）**

- 置き場所：project/Library/（新規フォルダ。.gitignore に足す）
- Library/SourceDB：GUID ごとの記録（パス、ImporterType、インポート鍵、サブオブジェクトの表、変換時と実行時の依存、状態、ログ、時刻とサイズ）。起動時に読み、Refresh とインポートの後に書く
- Library/Artifacts/xx/<インポート鍵>\_\<LocalId>：オブジェクトごとの成果物。先頭2文字でサブフォルダに分け、一時ファイルに書いてからリネームする
- オブジェクトごとに分けるのは、メッシュ1つを読むときにモデル全体を読まずに済ませるため
- どの記録からも参照されなくなった成果物は、起動時にまとめて削除する

**インポートの実行（ImportScheduler）**

- JobSystem のワーカーで並列に実行し、SourceDB の更新と実行時側への通知だけをメインスレッドで行う
- 優先度：実行時側から要求されたもの（今すぐ表示したいもの）が最優先。それ以外は Refresh で見つかった順
- 進捗をエディタに表示する（件数と今のファイル）
- 終わったら IArtifactProvider 経由で実行時側へ知らせ、読み込み済みなら差し替える（7章）
- Play 中も外部の変更による再インポートは行う。インポート設定の Apply は Play 中は無効

**型ごとのインポート設定**

- Model（OBJ）：flipHandedness（既定 true）、scale（既定 1.0）、defaultShader（既定 "Standard"）、materialRemap（ファイル内のマテリアル名から .mat の GUID へ）、keepCpuData（既定 false）
- Texture：sRGB（既定 true）、generateMips（既定 true）
- Audio：初版は設定なし（ストリーミングやモノラル化は後回し）
- Material、Scene、Folder、Default：設定なし

**AudioImporter の注意**

- Media Foundation はプロセスで MFStartup が要る。AssetPipeline 層の初期化で呼び、終了で MFShutdown を呼ぶ
- ワーカースレッドは JobSystem が起動時に COM（MTA）を初期化する（7章）

## 6. 接点：成果物と ArtifactProvider

実行時側は IArtifactProvider という1つのインターフェースだけを通して成果物を受け取る。実装は Debug 用の EditorArtifactProvider と Release 用の PackageArtifactProvider の2つだけで、旧仕様の LibraryArtifactProvider は作らない（判断10）。

**成果物の共通形式**（全型で同じ先頭部分）

- 識別子（'CKAR'）と共通形式の版数
- ObjectType と、型ごとの形式の版数。ObjectType は種別タグで書き、列挙の数値は書かない
- 自分の AssetId
- 実行時の依存（AssetId の配列）
- 本体（型ごとの形式。既存の BinaryStream で書く）
- 先頭部分だけを読めば依存が分かる。版数が合わなければ本体を読まずに失敗とし、Debug なら再インポートへ回す

**IArtifactProvider の責務**

- Contains(AssetId)、GetObjectType(AssetId)：そのオブジェクトがあるか、何の型かを、成果物を読まずに答える
- RequestArtifact(AssetId, 完了時の処理)：成果物のバイト列を非同期で渡す。ワーカースレッドから呼ばれてもよい
- FindByPath(path) → AssetId：パスからの引き当て（LoadByPath 用）
- 変更通知：成果物が作り直された AssetId をメインスレッドへ知らせる（Debug でだけ起きる）

**実装**

- EditorArtifactProvider（AssetPipeline 層、Debug）：SourceIndex と Library を使う。要求された成果物が古ければ、その場でインポートを最優先で予約し、終わってから渡す
- PackageArtifactProvider（Asset 層、Release）：パッケージを読む
- MemoryArtifactProvider（Asset 層、テストとデバッグ用）：メモリ上に置いた成果物を渡す。フェーズ2で元ファイル側なしに実行時側を確かめるのに使う

**パッケージの中身**

- 出力先は project/Package/Assets.pak（新規フォルダ。.gitignore に足す）。ファイルは1つ
- 先頭：識別子（'CKPK'）と版数、成果物の共通形式の版数
- 対応表：AssetId から位置・大きさ・ObjectType。パスから AssetId（常に含める一覧にあるものだけ。LoadByPath のため）
- 本体：Library の成果物をそのまま連結する（変換し直さない）
- 形式の定義は Asset 層の Engine/Asset/Provider/ に置き、書く側（PackageBuilder）と読む側（PackageArtifactProvider）が同じ定義を使う

**パッケージの出力**

- 入力：ProjectSettings の buildScenes（ビルドに含めるシーンの AssetId、startupScene を含む）と alwaysIncludedAssets（LoadByPath で使うもの）
- 手順：未インポートと古いものをインポートし切る → 入力から実行時の依存をたどり、届くオブジェクトをすべて集める → 一時ファイルに書いてからリネームする
- 集めた中にインポートに失敗したもの、あるいは参照先が見つからないものがあれば、出力を止めて一覧をログに出す。壊れたパッケージを作らないため
- 実行するのはエディタのメニュー（File の Build Package、13章）。Play 中は無効
- PackageBuilder（AssetPipeline 層）は入力を構造体で受け取り、エディタの UI も GPU も知らない。将来、コマンドラインから出力して CI でパッケージを作るときも、そのまま使える

**Release が読むもの**

- ProjectSettings/ProjectSettings.json（起動シーンの AssetId）
- Package/Assets.pak
- シェーダーのソース（EngineResources/shaders/ と Assets/Shaders/ を ShaderLibrary がパスで読む。シェーダーはアセットではないので、パッケージに入らない）
- これ以外の Assets/ と EngineResources/ のファイルは読まない
- パッケージが無い、または版数が合わないときは、ログを出してメッセージボックスで知らせ、終了する。アセットが1つも無い状態ではゲームが成り立たないため
- 配布するときは、exe、dxcompiler.dll と dxil.dll、上の3つを同じ配置で置く。配布用フォルダを自動で組み立てる処理は後回し（17章）

## 7. 実行時側：格納基盤と非同期読み込み

AssetManager が全型のアセットを1つの仕組みで管理し、型ごとの違いはローダー（成果物から実体への変換）だけに閉じ込める。格納基盤に触れるのはメインスレッドだけにして、ロックを不要にする。

**ハンドルの規則**

- generation はスロットが解放されるたびに増える。古いハンドルで引くと「無し」が返り、落ちない
- 差し替え（再インポート）では generation を変えない。ハンドルを持っている側は何もしなくてよい（既存の HandlePool の Replace と Recreate の区別を引き継ぐ）

**AssetStorage\<T>**（型ごとに1つ。中身は共通のテンプレート）

- スロットが持つもの：実体 T、読み込み状態、AssetId（ランタイム生成なら無し）、実行時の依存、常駐フラグ、Pin の数、失敗の理由
- AssetId からスロットへの索引と、空きスロットの再利用
- 実体の置き場所を安定させる。スロットの配列が伸びても既存の実体が動かない構造にする（今の Manager にある「次の追加までしかポインタが有効でない」問題をなくすため）
- テクスチャのスロットは SRV の番号も持つ（9章）

**読み込み状態**

- Unloaded → Queued → Reading（成果物の取得）→ Decoding（ワーカーで CPU 処理）→ Creating（メインスレッドで実体を作り、GPU 転送を予約）→ Uploading（転送の完了待ち）→ Ready
- どの段階で失敗しても Failed（理由つき）。Failed でも落とさず、代替を返す
- 依存先の完了は待たない。マテリアルはテクスチャを待たずに Ready になり、テクスチャは届くまで代替で描かれる。まとめて揃えたいときは読み込みグループを使う

**AssetManager の API**（形のみ）

- Load\<T>(AssetId) → AssetHandle\<T>：すぐにハンドルを返す。同じ AssetId なら同じハンドル。未読み込みなら読み込みを予約する
- LoadByPath\<T>(path)：パスで引く。リネームに弱いので、コンポーネントからは使わず AssetRef を使う。Release では常に含める一覧にあるものだけが引ける
- GetState(handle)、TryGet(handle) → const T\*（Ready 以外は nullptr）
- GetOrFallback(handle) → const T&：描画用。Ready なら実体、読み込み中なら「読み込み中」の代替、失敗・無効なら「エラー」の代替
- WaitFor(handle)：完了まで待つ（内部で Update を回す）。エディタの道具と起動時の必須アセット用で、ゲームのフレーム中には使わない
- CreateRuntime\<T>(…)／Destroy(handle)：コードで作る実体。AssetId を持たず、回収の対象外で、作った側が明示的に破棄する。その実体が参照するハンドルは回収のルートになる（8章）
- Pin(handle)／Unpin(handle)：コンポーネントの外で持ち続けるものを回収から守る
- Update()：毎フレーム1回、メインスレッドで呼ぶ。完了した処理の反映、転送完了の確認、差し替え、SRV の書き換え、状態遷移をまとめて行う。呼ぶ位置はコマンドの記録が始まる前（12章）
- CollectUnused(roots)：回収（8章）

**読み込みグループ（LoadGroup）**

- AssetId の集合と、その実行時の依存をたどって届くものをすべて読み込み、全部が Ready か Failed になったら完了とする
- シーンの読み込みで使う。全部が揃ってからシーンを有効にするので、読み込み途中の見た目が出ない。進捗はロード画面に使える

**ローダー（型ごと）**

- Decode(成果物) → CPU データ：ワーカースレッド。解析と展開だけ
- Create(CPU データ, 生成環境) → T：メインスレッド。GPU リソースと SRV の確保、GPU 転送の予約
- Destroy(T)：解放の予約（遅延解放へ回す）
- 代替の実体（読み込み中・エラー）を起動時に作る

**スレッド（JobSystem、Platform 層）**

- 固定数のワーカースレッド（論理コア数−1、最低1。hardware\_concurrency が 0 を返したら 1 として扱う）
- 優先度つきの作業キュー（高・通常・低）。作業は「ワーカーで走る処理」と「メインスレッドで走る完了処理」の組で投げる
- API は Submit、PumpMainThread、WaitFor。PumpMainThread は毎フレーム1回呼び、1フレームで使ってよい時間に上限を設ける
- 取り消しは強制停止ではなく、完了時に世代を比べて古い結果を捨てる方式にする（「読み込み中に変更されたらやり直す」に使う）
- WaitFor で待っている間は、メインスレッドもキューの処理を手伝う（ワーカー1本の環境でも詰まらないように）
- ワーカーの起動時に CoInitializeEx（MTA）と SetThreadDescription を呼ぶ。ワーカー内の例外は捕まえ、失敗として完了処理へ渡す
- 終了時は「受付停止 → 未着手を破棄 → 実行中を待つ → join」の順にする。DebugLog より先に止める
- ワーカーでやること：成果物の取得（ファイルの読み込み）、Decode、インポート、Refresh の走査、パッケージの書き出し
- ワーカーでやらないこと：格納基盤・GPU・ImGui・シーンへのアクセス
- 計画外の並列化（カリングなどフレーム処理の分割）をしたくなったら、API を変えずに中身だけをワークスティーリングに差し替える

## 8. GPU との接点

GPU への転送は UploadQueue、解放は DeferredReleaseQueue、SRV の番号は SrvManager が一手に引き受ける。どれも今の「毎フレーム GPU を待ち切る」同期設計を前提にし、描画を複数フレーム重ねる構成に変えるときは中身だけを差し替える。

**UploadQueue（Graphics 層、Engine/Graphics/Upload/）**

- ローダーは「このリソースへこのデータを転送」と予約するだけ。UploadQueue が転送用の一時バッファを確保し、コピー命令を積み、完了をフェンスで追う
- 1フレームに転送する量に上限を設ける。大きなテクスチャが一度に届いてもフレームが飛ばないようにするため
- 初期実装はメインのコマンドキュー（DIRECT）で、フレームのコマンドリストの先頭に転送を積む。今の描画は Renderer::EndFrame で毎フレーム GPU を待つので、専用のコピーキューに分けても描画と重なる時間がほとんど無いため
- 静的なメッシュとテクスチャは DEFAULT ヒープに置く（今のメッシュは UPLOAD ヒープに直接書いている）
- 今の TextureManager は1枚読むたびにフレームのコマンドリストで ExecuteAndWait している。これをなくすのが UploadQueue の目的の1つ

**DeferredReleaseQueue（Graphics 層、Engine/Graphics/Device/）**

- GPU リソースと SRV の番号は、GPU が使い終わるまで解放しない。そのフレームに捨てたものを溜め、フレームの終わり（ExecuteAndWait の後）にそのフレームのフェンス値で締め、次のフレームの先頭で完了済みのものを解放する
- フェンス値は1フレームに1回進むとは限らない（今の TextureManager がフレームの途中で ExecuteAndWait するため）。「次のフェンス値」で登録すると早すぎる解放になるので、必ずフレームの終わりで締める
- CommandManager（既存）に、最後に送ったフェンス値と完了済みのフェンス値を返す関数を足す
- 終了時は GPU の完了を待ってからすべて解放する

**SrvManager（Graphics 層、Engine/Graphics/Descriptor/、判断7）**

- 新しいクラスは作らず、今の SrvManager に Free（空きリストへの返却）を足す。Free は DeferredReleaseQueue を通して呼ぶ
- 二重解放を検出し、使用数と最大使用数を数える（GraphicsDebugWindow で表示する）
- 容量は定数にし、初期値は 4096 とする。今の 512 はテクスチャ1枚に2つ使うため約 250 枚で尽きる。値はフェーズ1で計測して見直す
- 尽きたときは Release でも無効な番号を返してログを出す（今は assert だけで、Release ではヒープの外を指す）。番号を取れなかったテクスチャは、常駐の Error テクスチャの番号を使う

**テクスチャの SRV の固定番号**

1. Load を予約したときに、スロット用の番号を SrvManager から確保し、常駐の White テクスチャを指す SRV を書く（読み込み中の表示）
2. 転送が終わって Ready になったら、同じ番号へ本物の SRV を書く
3. Failed になったら、同じ番号へ常駐の Error テクスチャを指す SRV を書く
4. 差し替え（再インポート）では、新しいリソースの SRV を同じ番号へ書き、古いリソースを遅延解放へ回す
5. 回収でスロットを解放したら、リソースと番号を遅延解放へ回す。番号は GPU が使い終わってから空きリストに戻る

- 書き換えは AssetManager::Update の中だけで行う。この時点では前のフレームの GPU 処理が終わっていて、今のフレームの記録がまだ始まっていないため、GPU が参照中の SRV を書き換えることがない
- 描画を複数フレーム重ねる構成に変えたら、書き換えもフェンスで遅らせる
- 描画側は状態を見ずに番号を使えばよい。例外はスプライトで、テクスチャの寸法を使うために状態を見る（12章）
- プレビュー用の SRV（sRGB を外した版）は ENABLE\_EDITOR のときだけ、2つ目の固定番号として同じ規則で持つ。Release ではテクスチャ1枚に番号1つ
- バインドレス（シェーダーモデル 6.6 の ResourceDescriptorHeap）へ移るときは、この固定番号をそのままシェーダーに渡す番号に使う（後回し）

**SRV を使うその他の側**

- ImGuiManager：初期化時に4枠を確保して貸し出す今の仕組みをやめ、確保と解放のコールバックから SrvManager を直接呼ぶ。解放は DeferredReleaseQueue を通す
- SceneRenderTarget：今のまま（初期化で1つ確保し、リサイズでは同じ番号へ作り直す）

**テストのしやすさ**

- 空きリスト、フェンス値の管理、転送量の上限の計算は、D3D12 の呼び出しから切り離して書く。Tests から GPU なしで確かめるため（14章）

## 9. 寿命：到達可能性による回収

回収は決まったタイミングでだけ走り、ルートから実行時の依存をたどって届かなかったアセットを解放する。毎フレームは走らせない。

**ルート**

- 読み込まれている全シーンの全 AssetRef。コンポーネントのプロパティ（配列の中も含む）と、シーン設定のプロパティを、リフレクションでたどる。たどる関数は Scene 層に置く（11章）
- 常駐アセット（内蔵アセットと代替の実体）
- Pin されているもの（エディタで選択中・プレビュー中のアセット、コードが保持しているもの）
- 読み込み中のものと、進行中の読み込みグループ
- ランタイム生成の実体が参照するハンドル。CreateRuntime で作った実体そのものは回収の対象外だが、そこから先（ランタイムのマテリアルが使うテクスチャなど）はルートとしてたどる。Pin のし忘れで参照先が消えることを防ぐため

**たどり方**

- ルートから、各アセットの実行時の依存（成果物の先頭に書かれたもの）をたどれるだけたどって印を付ける
- 印の付かなかった Ready と Failed のスロットを解放する。generation を増やし、実体と SRV の番号は遅延解放へ回す

**実行するタイミング**

- シーンを切り替えた後（SceneManager が呼ぶ）
- エディタで Play を止めた後
- ゲームコードやエディタのメニューからの明示的な呼び出し

**音声の実体**

- AudioClip の PCM は共有ポインタで持ち、再生中のボイスも同じポインタを持つ。XAudio2 は再生中にバッファを読み続けるので、回収や差し替えで実体が消えても、鳴っている音は最後まで鳴る
- そのため、一度きりの再生のために AudioClip を Pin する必要はない

**利用側の規則**

- コンポーネントの外でアセットを持ち続けるなら、Pin するか AssetRef で持つ
- 解放済みのハンドルで引いても落ちない（無しが返る）
- Play 前のシーンのスナップショット（JSON 文字列）はルートに含めない。Stop で復元するときに読み直されるだけなので、正しさには影響しない

## 10. 型ごとの仕様

実行時の型は Mesh・Texture・Material・ModelPrefab・SceneAsset・AudioClip の6つで、それぞれ「読み込み中」と「エラー」の代替を持つ。

**Mesh**

- 成果物：頂点形式の識別子、頂点配列、インデックス配列（32bit）、サブメッシュ（インデックスの開始と個数）、AABB、サブメッシュごとの既定マテリアル（AssetId）、CPU データを残すかの印
- 既定マテリアルは「メッシュの既定マテリアルに合わせる」操作のための情報で、実行時の依存には入れない（メッシュを読んだだけでマテリアルまで読まないため）
- 頂点形式は今の VertexData（位置 float4、UV float2、法線 float3）を引き継ぐ。形式を変えるときは識別子と版数で区別する
- インポート時に、位置・UV・法線が同じ頂点をまとめてインデックス化する（今は頂点を展開したまま持っている）
- 実体：頂点バッファとインデックスバッファ（DEFAULT ヒープ）、ビュー、サブメッシュ、AABB、必要なら CPU 側の配列
- パーティクルも同じ Mesh を使う（インスタンシングで描く）
- 代替：読み込み中もエラーも「何も描かない」

**Texture**

- インポート：DirectXTex でメモリ上のバイト列から読み、設定に従って sRGB の指定とミップマップの生成を行う（今の TextureManager::Load と同じ処理。今は全テクスチャを sRGB として読んでいる）
- 成果物：幅・高さ・形式・ミップ数と、全ミップのピクセル列（展開済み）。圧縮（BC7 など）は後回し
- 実体：テクスチャリソース（DEFAULT ヒープ）、固定の SRV 番号（エディタではプレビュー用も）、幅と高さ
- 代替：読み込み中は白、エラーはマゼンタと黒の格子。どちらも同じ SRV 番号へ書かれるので、描画側は状態を知らない（8章）
- sRGB の既定は true。法線マップなどのデータ用テクスチャは false にする

**Material**

- 元ファイル（.mat v2）：シェーダー名、パラメータ、テクスチャスロットからテクスチャの AssetId への対応
- 今の .mat はテクスチャをパスで持っているので、テクスチャをリネームすると切れる。v2 で AssetId に変える。v1 の .mat はインポート時にパスを AssetId へ読み替え、エディタで次に保存したときに v2 で書き直す（Assets/Materials/ の9個が対象）
- 埋め込みマテリアル（OBJ と MTL に由来するもの）も同じ成果物の形式で、モデルのサブオブジェクトとして出す。エディタでは読み取り専用
- 成果物：シェーダー名、パラメータの値、スロット名からテクスチャの AssetId への対応。実行時の依存にテクスチャを書く
- 実体：シェーダー定義、定数バッファ、スロットからテクスチャのハンドルへの対応
- シェーダーはアセットにしない（ShaderLibrary に名前で登録されたものを使う）。読み込み時に名前が見つからなければ Failed にしてエラーの代替を使う
- requiresInstancing のシェーダーは通常の描画では使わずエラーマテリアルへ差し替える、という今の規則を引き継ぐ
- 描画時はスロットのテクスチャのハンドルから SRV 番号を引く。番号は差し替えでも変わらない（8章）
- 代替：読み込み中は内蔵の Default（灰色）、エラーは内蔵の Error（マゼンタ）

**ModelPrefab**

- 成果物：ノードの配列（名前、親の番号、ローカルの位置・回転・拡縮、メッシュの AssetId（無ければ無効）、マテリアルの AssetId の配列）。materialRemap は適用済み
- OBJ は階層を持たないので、「ファイル名のルートノードと、o ごとの子ノード」になる
- 実体：ノード配列そのもの（GPU リソースなし）
- インスタンス化は Scene 層の InstantiateModelPrefab が行う（ノードごとに GameObject を作り、MeshFilter と MeshRenderer を設定する）

**SceneAsset**

- 元ファイル（.scene v2、11章）を書くのは SceneSerializer（Scene 層）
- インポート：SceneImporter（AssetPipeline 層）は JSON の木をたどり、{"guid": …, "localId": …} の形をした値をすべて実行時の依存として集める。AssetRef の保存形式だけを知っていればよく、リフレクションもコンポーネントの型も知らない
- 旧仕様にあった dependencies（.scene に書く依存の一覧）はやめる。本体から集めれば、手で .scene を編集しても依存と本体が食い違わないため
- 成果物：シーンの JSON 本体と実行時の依存。実行時の依存は読み込みグループの先読みとパッケージの収集に使う
- 実体：解析済みの JSON。GameObject を作るのは SceneManager

**AudioClip**

- 元ファイル：.wav と .mp3（初版はこの2つ。今の Assets/Sounds/ に各 1 個）
- インポート：Media Foundation で PCM に展開する（今の SoundManager::SoundLoad と同じ変換を、実行時からインポートへ移す）
- 成果物：形式（チャンネル数、サンプリング周波数、ビット数、ブロックの大きさ）、PCM のバイト列、長さ（秒）
- 実体：形式と、共有ポインタで持つ PCM。GPU リソースは持たない
- 再生：SoundManager（Platform 層）に形式と PCM の共有ポインタを渡す。ボイスは再生が終わるまでポインタを持ち、終わったら破棄される（今の SoundPlay がボイスを破棄しない問題もここで直す）
- 代替：読み込み中もエラーも無音（再生しない）
- 長い音（BGM）のストリーミングは後回し。それまでは全体をメモリに展開する（44.1 kHz、ステレオ、16 bit なら1分あたり約 10 MB）
- AudioSource コンポーネント（作業中）は AssetRef\<AudioClip> で持つ。再生の制御など AudioSource そのものの設計は、この設計書の範囲外

**内蔵アセット**（コードで作る。常駐）

- Mesh：Triangle、Quad、Cube、Sphere、Tetrahedron（今の PrimitiveShape の生成処理を移す）
- Material：Default（Standard シェーダー、白）、Error
- Texture：White、Error
- Inspector の候補一覧では「Built-in」としてまとめて並べる
- 今の内蔵図形は既定マテリアルとしてエラーマテリアル（マゼンタ）を割り当てているが、Default（灰色）に変える
- スカイボックスの既定は内蔵アセットにしない。EngineResources/materials/DefaultSkybox.mat という普通の .mat にする（12章）

## 11. 参照とシリアライズ

コンポーネントとシーン設定は AssetRef\<T>（AssetId とハンドル）でアセットを持ち、保存されるのは AssetId だけ。型ごとの手書きはなくし、Inspector・保存・回収はすべてリフレクションで AssetRef をたどる。

**AssetRef\<T>（Engine/Asset/Core/）**

- 中身：AssetId id と AssetHandle\<T> handle（handle は保存しない）
- Resolve(AssetManager&)：handle = Load\<T>(id)。待たずに戻る
- 状態は4つに読める：未設定（id が無効）、読み込み中、Ready、失敗（参照先が無い Missing を含む）。今の SpriteRenderer が IsEmpty と IsResolved で作っている「未設定」と「見つからない」の区別は、この状態で置き換える
- 型の不一致（Mesh の欄にマテリアルの AssetId が入っているなど）は、成果物の ObjectType と要求した型を比べて Failed にし、ログを出す

**リフレクション**

- PropertyType の AssetRefModel・AssetRefTexture・AssetRefMaterial を、AssetRef の1種類にまとめる。PropertyDesc に「要求する ObjectType（整数として持つ）」と「AssetRef を読み書きする関数ポインタ」を持たせる。これで AudioClip のようにアセットの型を増やしても、リフレクション・Inspector・シリアライザは変わらない
- PropertyType::Array と ArrayOps（要素数、要素数の変更、要素の取得）を足す。対象は固定長の配列と個数の組（下の MeshRenderer を参照）。配列の入れ子は static\_assert で禁止する
- 既存の Gradient と EntityRef はそのまま残す
- 層違反の解消：PropertyTypeOf を「各層が自分の型について特殊化する」形にし、AssetRef の特殊化は Asset 層、GameObjectId の特殊化は Scene 層へ移す。TypeInfo.h は Foundation 層のヘッダだけを include する
- 回収のルートを集める関数（全コンポーネントとシーン設定の AssetRef を、配列の中までたどる）を Scene 層に置く。今の ResolveSceneAssets（型ごとの手書き）は、この関数と読み込みグループに置き換えて消す

**コンポーネント**

- MeshFilterComponent（新規）：AssetRef\<Mesh> mesh
- MeshRendererComponent：AssetRef\<Material> の固定長配列と個数、bool visible。CAKE\_REQUIRE\_COMPONENTS(MeshRendererComponent, MeshFilterComponent)
- ParticleRendererComponent：AssetRef\<Mesh> mesh と AssetRef\<Material> material（今の AssetRef\<ModelHandle> から型だけ変える）
- SpriteRendererComponent：AssetRef\<Texture> texture（型だけ変える）
- ゲーム側の SpriteTesterComponent：AssetRef\<Texture> alternateTexture（型だけ変える）
- AudioSource（作業中）：AssetRef\<AudioClip>

**MeshRenderer のマテリアルを固定長にする理由**

- エンジンの規則「可変長のデータはコンポーネントに持たせない」と「コンポーネントは素の構造体」に合わせるため。旧仕様の std::vector はこの規則に反するので改める（Gradient のキー列と同じ作り）
- 上限の初期値は 8。超えるサブメッシュはエラーマテリアルで描き、インポートと Inspector で警告する。値は実際のモデルで確かめて見直す（17章）
- 描画：materials\[i\] でサブメッシュ i を描く。足りない分はエラーマテリアルで描き、余る分は無視する。どちらも Inspector で警告する
- メッシュを差し替えてもマテリアルの配列は自動では変えない。Inspector に「メッシュの既定マテリアルに合わせる」操作を置く
- MeshFilter が無い MeshRenderer（Scene::AddComponent を直接呼んだ場合など）は描かずに飛ばす

**シーン設定（SceneSettings、Scene 層、Engine/Scene/SceneSettings.h、判断12）**

- シーンに1つだけある設定の構造体。Scene が所有し、GameObject には属さない
- 初版の項目は AssetRef\<Material> skyboxMaterial だけ
- コンポーネントと同じく CAKE\_REFLECT でリフレクションに載せる。これで Inspector（PropertyDrawer）、保存（SceneSerializer）、回収のルート、Play と Stop のスナップショットが、専用のコードなしに設定も扱える
- 項目を足すときは、メンバと CAKE\_PROPERTY を足すだけになる

**シーン形式 v2**

- kSceneVersion を 2 に上げる。v1 は読まない（エラーを出して開かない）。既存のシーンは GameScene.scene（5オブジェクト）だけなので、作り直す
- 追加：settings（SceneSettings のプロパティ。コンポーネントと同じ書き方）
- AssetRef の書き方は今と同じ {"guid": "…", "localId": n}。SceneImporter はこの形を目印に依存を集めるので、他のプロパティに同じ形を使わない
- 配列のプロパティは JSON の配列（個数は配列の長さから決め、上限を超えた分は読み捨てて警告する）
- Play と Stop のスナップショットは今と同じく JSON 文字列でメモリに持つ（アセットの仕組みは通さない）

**パスで持っている参照を AssetId に変える**

- ProjectSettings.startupScene（今は "Assets/GameScene.scene"）
- EditorCache.openScenePath（前回開いていたシーン）
- どちらもシーンをリネーム・移動すると切れるため。ProjectSettings には buildScenes と alwaysIncludedAssets（どちらも AssetId の配列）を足す（6章）

## 12. 描画との接点

Scene 層が1フレームに1回、描くものを SceneDrawData に積み、Render 層のパスはそれだけを受け取って描く（判断9）。描画リストに載るのはハンドルだけで、実体を引くのは Render 層の Renderer が AssetManager から行う。

**SceneDrawData（Render 層、Engine/Render/SceneDrawData.h）**

- MeshDrawList：メッシュのハンドル、マテリアルのハンドルの並び、ワールド行列
- ParticleDrawList：エミッターごとのワールド行列、メッシュとマテリアルのハンドル、粒の位置・回転・拡縮・色の写し。ビルボードの向きは視点ごとに違うので、行列はパスが視点ごとに作る
- SpriteDrawList（既存）：テクスチャを AssetHandle\<Texture> に変え、isTextureMissing は参照の状態に置き換える
- 平行光源（今の CollectDirectionalLight の結果）と、スカイボックスのマテリアルのハンドル
- 積むのは Scene 層の CollectSceneDrawData（Engine/Scene/System/Render/）。SceneManager::Update の後に1回呼び、エディタのゲームビューとシーンビューは同じものを使う
- ギズモは今と同じく、エディタがシーンビュー用に積んで RenderViewOptions で渡す

**Render 層の変更**

- SceneRenderer::Render は SceneDrawData、視点、表示オプションを受け取る。各パスは Scene 層の System を呼ばず、SceneDrawData だけを読む
- TryMakeMainCameraView は Scene 層（CameraSystem）へ移す。視点を作るのは呼び出し側（Release は Application、Debug は Editor）
- Renderer：DrawModel を DrawMesh（インデックス描画、サブメッシュごとのマテリアル）、DrawModelInstanced を DrawMeshInstanced に変える。DrawSprite と DrawSkybox はハンドルを受け取る
- RendererInitDesc の TextureManager・MaterialManager・ModelManager は AssetManager に置き換える
- この結果、Render 層から Scene 層への include がなくなる。14章の include の検査で、戻っていないことを確かめ続ける

**スカイボックス（判断12）**

- シーン設定の skyboxMaterial が指すマテリアルで描く。未設定なら描かず、背景は今のクリア色になる
- 受け付けるのは、名前が Skybox/ で始まるシェーダーのマテリアルだけ。それ以外なら描かず、Inspector で警告する
- 新しく作ったシーンには EngineResources/materials/DefaultSkybox.mat（新規。Skybox/Panoramic シェーダーで textures/skybox.png を貼る）を設定しておく
- Renderer が起動時に作っているランタイムのマテリアル（\_\_Skybox）と、Skybox.png をパスで読む処理は消す。大文字小文字の食い違い（コードは Skybox.png、実ファイルは skybox.png）も、パス自体がなくなるので解消する
- シーンに入るので、パッケージにも回収にも特別扱いなしで載る

**スプライトの描き方**

- 未設定なら内蔵の White で、size の大きさで描く（今と同じ）
- 読み込み中は描かない。寸法が分からないまま描くと、読み込みが終わった瞬間に大きさが変わるため。シーンの読み込みは読み込みグループで揃うので、これが起きるのは実行中に新しいテクスチャを指したときだけ
- Ready なら今と同じ規則で描く。失敗・参照先が無いときはエラーのテクスチャで、size の大きさで描く（今と同じ）

**1フレームの順序**（Application::RunFrame）

1. DebugLog::PumpMainThread と JobSystem::PumpMainThread（ワーカーの完了処理）
2. Platform::BeginFrame、描画先の遅延リサイズ、CheckResize（今と同じ）
3. DeferredReleaseQueue で、GPU が使い終わったものを解放する
4. AssetManager::Update（状態の遷移、SRV の書き換え、差し替え）。ここまではコマンドを記録していない
5. Renderer::BeginFrame の直後に、UploadQueue がそのフレームの転送をコマンドリストの先頭に積む
6. シーンの更新、CollectSceneDrawData、描画（Debug は Editor、Release はゲームの視点だけ）
7. Renderer::EndFrame（ExecuteAndWait と Present）。その後、DeferredReleaseQueue をこのフレームのフェンス値で締め、UploadQueue に転送の完了を知らせる
8. Platform::EndFrame（60 FPS の待機。今と同じ）

**初期化の順序**（Application::Initialize）

1. DebugLog、ProjectSettings、ウィンドウ（今と同じ）
2. Platform。中で JobSystem を起動する
3. Graphics、Game::RegisterGameShaders（今と同じ）
4. Debug では AssetPipeline（SourceIndex の読み込みと Refresh）を初期化して EditorArtifactProvider を、Release では PackageArtifactProvider を作る
5. AssetManager（Provider を渡す）と内蔵アセット
6. Renderer、コンポーネントの登録、SceneRenderer、SceneManager
7. 起動シーンを読み込みグループで読んで待つ（Debug では前回開いていたシーンを優先する）
8. Editor（Debug のみ）、Time::Reset
9. 終了は逆の順。JobSystem は Platform と一緒に止まるので、AssetManager より後、DebugLog より前になる

## 13. エディタ機能

最小要件の4つ（Project ウィンドウ、アイコン、選択時の情報表示、D&D）は、すべて SourceIndex と AssetId の上に作る。これにシーン設定の編集と、パッケージの出力を加える。

**選択（EditorSelection）**

- 種類：無し／GameObject／アセット
- アセットは AssetId と ObjectType で持つ。フォルダも Default も GUID を持つので、同じ形で扱える
- エディタ全体で1つ。Hierarchy と Project のどちらで選んでも、もう一方の選択は外れる
- 選択中のアセットは Pin する（Inspector に表示している間に回収されないようにするため）

**Project ウィンドウ**

- SourceIndex のツリーを表示する。ディスクは直接見ない
- 並び：フォルダ、ファイルの順で、それぞれ名前順
- モデルファイルは展開でき、子にメッシュと埋め込みマテリアルが並ぶ（サブオブジェクトの表から作るので、読み込みは要らない）
- アイコン：ObjectType ごとのフォントアイコン。最初に対応するのはマテリアル、テクスチャ、シーン、フォルダで、それ以外は「?」。インポートに失敗したものには警告の印を重ねる。AudioClip とモデルのアイコンは、最小要件の後に足す
- 操作：クリックで選択、ダブルクリックで開く（シーンなら開く）、右クリックで Reimport・エクスプローラで表示・Create Material、Refresh ボタン
- EngineResources/ は別のルートとして読み取り専用で表示する。D&D の元にはなるが、中の .mat は編集できない

**アセット Inspector**

- 見出し：名前、ObjectType、パス、GUID（コピー可）、インポート状態とログ
- インポート設定：インポータごとの描画と Apply／Revert。Play 中は無効
- 中身の表示：実行時側で読み込んで表示する（表示中は Pin）。型ごとの描画は後から足し、最初は見出しとインポート設定だけにする
- 型ごとに予定している表示：Texture（プレビュー、サイズ、形式）、Mesh（頂点数、インデックス数、サブメッシュ数、AABB）、Material（編集）、ModelPrefab（ノードの一覧、materialRemap の編集、埋め込みマテリアルの Extract）、AudioClip（長さ、形式、試聴）
- 参照先が見つからない AssetRef は「Missing」と表示する

**D&D**

- ペイロード：{AssetId, ObjectType}。ペイロード名は1種類（Hierarchy の親子の張り替えに使う "HierarchyObject" とは分ける）
- 受け側の判定 CanAssign(要求する型, ペイロード)：型が一致すれば受け入れる。追加の規則として、Mesh の欄に「メッシュを1つだけ持つ ModelPrefab」が来たら、そのメッシュを採用する
- 型が合わないときはドロップ枠を出さない
- ImAssetRefField：候補の一覧（SourceIndex から型で絞り、内蔵アセットも含める）とドロップの受け口を1つにしたウィジェット。今の ImAssetRefCombo を置き換える。候補の一覧だけの版をフェーズ5で作り、ドロップの受け口をフェーズ6で足す
- Hierarchy と Scene ビューへの ModelPrefab のドロップ：InstantiateModelPrefab で GameObject を作る

**シーン設定の編集**

- 今の Environment ウィンドウを Scene Settings ウィンドウに変え、開いているシーンの SceneSettings を PropertyDrawer で描く。スカイボックスの欄は ImAssetRefField
- その下に、参照先のマテリアルの Inspector を表示する。EngineResources/ の DefaultSkybox.mat なら読み取り専用。空を変えたいときは、Assets/ に自分の .mat を作って指し替える
- 変更はシーンの変更として扱い、Ctrl+S で .scene に保存される。Play 中の変更は Stop で戻る（コンポーネントと同じ）

**編集と保存の流れ**

- エディタが書いたファイルは、書いた直後に SourceIndex へ知らせる（Refresh を待たない）
- マテリアル：Inspector での変更はすぐ実体に反映して見た目を確かめられるようにし、同時に「未保存」にする。Ctrl+S で .mat を書き、通知、再インポート、差し替えの順に進む（内容は同じなので見た目は変わらない）。未保存の変更があるときに外から .mat が変わったら、エディタ側を優先して警告を出す
- シーン：保存で .scene を書いて知らせる。エディタで開いているシーン自体は差し替えない（編集中の状態を優先する）
- モデルのインポート設定：Apply で .meta を書き、再インポートして差し替える
- 埋め込みマテリアルの Extract：.mat を作り、モデルの materialRemap に登録して再インポートする（今の Create Material Asset を置き換える）
- Create Material：選択中のフォルダに .mat を作る（今の MaterialList ウィンドウの Create を置き換える）

**パッケージの出力（Build Package）**

- File メニューに Build Package を足す。Play 中は無効
- PackageBuildCommand（Editor 層、Engine/Editor/Build/）が、ProjectSettings の buildScenes と alwaysIncludedAssets を集めて PackageBuilder に渡す
- 未保存のシーンやマテリアルがあるときは、保存するかを尋ねてから出力する
- 進捗と結果（含めたオブジェクトの数、ファイルの大きさ、失敗したもの）をログに出す
- ProjectSettings の設定ウィンドウに、buildScenes と alwaysIncludedAssets を編集する欄を足す

## 14. テストと検査

GPU を使わない部分はテスト用のプロジェクトで自動で確かめ、GPU を使う部分は常設のデバッグウィンドウで確かめる。層の境界は include の検査で守る。どれも CI で push のたびに走らせる。

**Tests プロジェクト（project/Tests/、新規フォルダ。どの層にも属さない）**

- Tests.vcxproj：コンソールの exe。Debug 構成だけを作る。設定は CakeEngine.Common.props を読み込む
- テストライブラリは doctest（ヘッダを1つ置くだけで使える）。project/externals/doctest/（新規フォルダ）にライセンスと一緒に置く
- テストの対象にする .cpp だけを Tests.vcxproj に入れる。入れた .cpp が上の層の関数を呼んでいればリンクで失敗するので、層の境界の検査にもなる
- 主な対象
  - Foundation：AssetId、Guid、LocalId
  - Platform：JobSystem（優先度、取り消し、WaitFor、終了の順序）
  - Graphics：SrvManager の空きリスト、DeferredReleaseQueue のフェンス値の管理、UploadQueue の転送量の上限（どれも D3D12 を呼ばない部分、8章）
  - Asset：AssetStorage の状態遷移、差し替え、回収、読み込みグループ（MemoryArtifactProvider を使う）、成果物とパッケージの形式の読み書き
  - AssetPipeline：一時フォルダを使った SourceIndex の走査と .meta、移動・削除・重複の扱い、インポート鍵と依存による再インポート、ObjImporter と MaterialImporter（v1 の読み替え）、パッケージの出力と読み戻し
  - Scene：AssetRef の列挙、シーン v2 の保存と読み込みの往復
- テスト用の入力ファイル（小さい OBJ、MTL、PNG、旧形式の .mat）は project/Tests/Data/ に置く
- WARP（D3D12 のソフトウェア実装）で UploadQueue の転送と読み戻しを確かめるテストは、CI のランナーで WARP が動くかを確かめてから足す（17章）

**GraphicsDebugWindow（Editor 層、Engine/Editor/Debug/、Debug のみ）**

- 常に表示するもの：SRV の使用数と最大使用数、今のフレームの転送量、解放待ちの数、ジョブのキューの長さ、状態ごとのアセットの数
- テスト用の操作：ワーカーでテクスチャを作って UploadQueue で転送して表示する、作成と破棄を数千回繰り返す、MemoryArtifactProvider から Mesh と Texture を非同期で読む

**CI（.github/workflows/）**

- Tests.yml（新規）：Windows のランナーで Tests をビルドして実行する。失敗したテストがあれば失敗する
- CheckLayerIncludes.yml（新規）：層ごとに include してはいけないフォルダを grep で探し、見つかれば失敗する。既存の CheckUnwantedFiles.yml と同じく Linux のランナーで走らせる
  - 調べる例：Engine/Asset・Engine/Render・Engine/Scene から Engine/AssetPipeline への include、Engine/Render から Engine/Scene への include、Engine/Foundation から他の層への include
  - 既存の層違反が残っている間は、その箇所を例外として列挙する。解消したら例外から消すので、例外の一覧がそのまま残りの層違反の一覧になる
- 既存の ReleaseBuild.yml：AssetPipeline.lib をリンクしないので、実行時側が元ファイル側を呼んでいればここで失敗する

## 15. 既存コードの扱い

既存のアセット関連のクラスは考え方を引き継ぐものも含めて新しい構成で作り直し、フェーズ5の切り替えでまとめて削除する。

**削除するもの（→ 置き換え先）**

- AssetDatabase と AssetType → SourceIndex（元ファイル側）と AssetManager（実行時側）
- TextureManager・TextureHandle → AssetStorage\<Texture> と TextureLoader、AssetHandle\<Texture>
- MaterialManager・MaterialHandle・MaterialSerializer → AssetStorage\<Material> と MaterialLoader、MaterialImporter、エディタの保存処理
- ModelManager・ModelData・ModelHandle・Mesh.h・PrimitiveShape → Mesh 型、ModelPrefab 型、内蔵アセット
- Engine/Asset/Runtime/HandlePool → AssetStorage（Replace と Recreate の区別は引き継ぐ）
- Engine/Asset/Database/AssetRef.h（AssetTraits）→ Engine/Asset/Core/ の AssetRef
- Engine/Asset/Import/（ImportPipeline・ArtifactCache・ArtifactKey・AssetImporter）→ AssetPipeline 層のインポート一式
- Engine/Asset/Artifact/ModelArtifact → 型ごとの成果物の形式
- ResolveSceneAssets → AssetRef の列挙と読み込みグループ
- Renderer の \_\_Skybox（ランタイムのマテリアル）と Environment ウィンドウの中身 → シーン設定と DefaultSkybox.mat
- SoundManager::SoundLoad（パスから読む）と SoundData → AudioClip
- ModelEditor・ModelList・MaterialList ウィンドウ・Create Material Asset・マテリアルスロットのコンボ → アセット Inspector、Project ウィンドウの Create Material と Extract
- ImAssetRefCombo → ImAssetRefField
- ImGuiManager の4枠の貸し出し → SrvManager を直接呼ぶコールバック
- 1-A の暫定変更（AssetDatabase.cpp の ToImporterType）と、非推奨の MakeLocalId
- 呼び出し元の無い Material::LoadMaterialTemplateFile と FindMaterialByName
- GPUResourceUtility.cpp の使っていない Material.h の include
- EngineResources/textures/Sky.png と uvChecker.png（コードから使われていない。フェーズ1で消す）

**そのまま使うもの**

- Guid、LocalId、AssetId、BinaryStream、JsonFile、DebugLog
- 1-A の ObjectType、MakeTaggedLocalId、MakeSubAssetId、SubAssetNameTable、.meta v2（AssetMeta と ImporterType は AssetPipeline 層へ移す）
- 依存宣言（CAKE\_REQUIRE\_COMPONENTS）
- ShaderLibrary・ShaderDefinition（シェーダーはアセットにしない）
- SceneRenderTarget、GizmoDrawList

**書き換えるもの**

- Renderer：DrawMesh、DrawMeshInstanced、DrawSprite、DrawSkybox をハンドルで受け取る形に。RendererInitDesc は AssetManager を受け取る
- SceneRenderer と各パス：SceneDrawData を受け取り、Scene 層を include しない
- RenderSystem、SpriteSystem、LightSystem、CameraSystem、ParticlePass：描画リストを積む形と、描画リストを読む形に分ける
- SpriteRendererComponent、ParticleRendererComponent、MeshRendererComponent、ゲーム側の SpriteTesterComponent：AssetRef の型
- Scene（SceneSettings を持つ）、SceneManager（AssetId で読み込み、読み込みグループ、切り替え後の回収）、SceneSerializer（v2 と settings）
- SoundManager：PCM の共有ポインタから再生し、ボイスを再生の終わりで破棄する
- SrvManager、CommandManager、TextureManager（フェーズ1で空きリストへ移し、フェーズ5で削除）
- Application：初期化と RunFrame の順序（12章）、Release の起動（PackageArtifactProvider）
- ProjectSettings（startupScene を AssetId に、buildScenes と alwaysIncludedAssets を追加）、EditorCache.openScenePath（AssetId に）
- Editor（選択の一元化、Scene Settings ウィンドウ）、MenuBar（保存の流れ、Ctrl+S、Build Package）、SceneInspector、SceneWindow、SettingsWindow
- .gitignore（project/Library/ と project/Package/）
- AssetId.h のコメント（「AssetPipeline 層」はこの設計書を指す形にする）、DebugLog.h のコメント（JobSystem を作った時点で実態と一致する）

## 16. ロードマップ

6つのフェーズで進める。フェーズ5の切り替えまでは既存の機能を動かしたまま、新しい仕組みを横に作る。旧版のフェーズ7（配布版）は判断10によりフェーズ5に含めた。ファイル単位の作業は各フェーズの着手時に決める。

```
フェーズ1 土台 ──→ フェーズ2 実行時側 ─┬─→ フェーズ3 元ファイル側 ─┬─→ フェーズ5 切り替え ──→ フェーズ6 エディタ
                                       └─→ フェーズ4 リフレクション ─┘
```

フェーズ3とフェーズ4はどちらもフェーズ2の後に始められ、フェーズ5はその両方が終わってから始める。

**フェーズ0（完了）**：コンポーネントの依存宣言、型と ID の基盤（1-A）

**フェーズ1：土台（進行中）**

- 目的：非同期読み込みと回収の前提になる下回りと、確かめる手段を作る
- 手順1〜3（完了）：EngineResources/ への移動、AssetId、DebugLog のスレッド安全化
- 手順4：JobSystem（7章）
- 手順5：CommandManager のフェンス値の取得口と DeferredReleaseQueue
- 手順6：SrvManager の空きリスト化。容量の定数化、Release でも尽きたことを検出、ImGuiManager のコールバックを直接つなぐ、TextureManager を空きリスト経由にする（旧版の「DescriptorAllocator を新設」はやめる）
- 手順7：UploadQueue
- 手順8：確かめる手段。CakeEngine.Common.props、Tests（doctest）と Tests.yml、CheckLayerIncludes.yml、GraphicsDebugWindow
- 手順9：仕上げ。Sky.png と uvChecker.png を消す。転送量の上限、ワーカー数、SRV の容量を計測して決め、17章に書き戻す
- 完了条件：TextureManager が SrvManager の空きリスト経由で動く。ワーカーで作ったテクスチャを UploadQueue で転送して表示できる。作成と破棄を数千回繰り返しても SRV が尽きない。Tests と CheckLayerIncludes が CI で通る。Debug と Release の両方で、デバッグレイヤーの警告がなく、終了時の ReportLiveObjects でリークがない

**フェーズ2：実行時側**

- 目的：AssetManager を、元ファイル側なしで動かせる状態にする
- 成果：AssetHandle、AssetStorage、AssetManager（状態遷移、非同期、差し替え、Pin、読み込みグループ、回収）、IArtifactProvider、成果物の共通形式と型ごとの形式、Mesh・Texture（SRV の固定番号）・Material・ModelPrefab・SceneAsset・AudioClip の実体とローダー、内蔵アセット、MemoryArtifactProvider
- 完了条件：状態遷移・差し替え・回収・読み込みグループのテストが通る。GraphicsDebugWindow で、MemoryArtifactProvider から Mesh と Texture を非同期で読んで描ける。テクスチャが「白 → 本物」「白 → エラー」と、同じ SRV 番号のまま変わる

**フェーズ3：元ファイル側**

- 目的：Assets/ と EngineResources/ の元ファイルから成果物を作れるようにする
- 成果：AssetPipeline.vcxproj と AssetPipeline 層、SourceIndex（走査、.meta、Refresh、移動・削除・重複の扱い）、インポートの枠組み（ImportContext、依存の記録、インポート鍵）、ImportScheduler、Library（SourceDB、成果物）、TextureImporter・MaterialImporter（.mat v1 の読み替えを含む）・ObjImporter・SceneImporter・AudioImporter、EditorArtifactProvider
- 完了条件：起動すると全ファイル・全フォルダに .meta が付き、Library に成果物ができる。.mtl だけを変えても OBJ が再インポートされる。エディタの外で .png を上書きすると、読み込み済みのテクスチャが入れ替わる。.wav と .mp3 が AudioClip の成果物になる。Release が AssetPipeline.lib をリンクせずに CI でビルドできる

**フェーズ4：リフレクション**（フェーズ3と並行できる）

- 目的：配列と汎用の AssetRef を扱えるようにし、Foundation 層の層違反をなくす
- 成果：PropertyType の Array（固定長）と AssetRef、PropertyTypeOf の特殊化の移動、Inspector と SceneSerializer の対応、AssetRef の列挙（コンポーネントとシーン設定）
- 完了条件：既存のコンポーネントが今までどおり Inspector で編集・保存できる（旧 PropertyType も残した状態で）。TypeInfo.h が Foundation 層のヘッダだけを include し、CheckLayerIncludes の例外から外れる

**フェーズ5：切り替え**（途中はビルドが通らない期間があるので、ブランチで作業する）

- 目的：描画・シーン・音声を新しい仕組みに載せ替え、Release をパッケージで起動し、旧実装を消す
- 5-A 描画：MeshFilter と MeshRenderer、SceneDrawData と描画リスト、Renderer の新しい関数、パスの書き換え、Render から Scene への include をなくす
- 5-B シーン：SceneSettings と DefaultSkybox.mat、Scene Settings ウィンドウ、SceneManager（読み込みグループと回収）、シーン v2、起動シーンの AssetId 化、Application の初期化順、Sprite・Particle・SpriteTester の参照の型、ImAssetRefField（候補の一覧だけ）、GameScene.scene の作り直し
- 5-C 音声：SoundManager の PCM 再生とボイスの管理、AudioClip からの再生経路
- 5-D パッケージ：PackageBuilder、PackageArtifactProvider、ProjectSettings の buildScenes と alwaysIncludedAssets、Build Package メニュー、Release の起動
- 5-E 削除：15章の削除対象
- 完了条件：新しく作ったシーンで、保存・読み込み・複製・Play と Stop・シーンの切り替えが崩れない。切り替え後の回収で、使わなくなったテクスチャとその SRV 番号が解放される。Scene Settings で選んだスカイボックスが保存され、開き直しても残る。Release がパッケージだけで起動する（Assets/ のシェーダー以外を除いた状態で確かめる）。CheckLayerIncludes の Render から Scene への例外が消える

**フェーズ6：エディタ（最小要件）**

- 目的：最小要件の4つを満たす
- 成果：EditorSelection、Project ウィンドウ、アイコン、アセット Inspector（見出しとインポート設定）、D&D（ImAssetRefField のドロップの受け口）、ModelPrefab のドロップ、マテリアルの編集と保存の流れ、Create Material、Extract
- 完了条件：最小要件の4項目が動く。複数メッシュの OBJ をドロップすると、メッシュごとの GameObject ができる

**旧版から変わったところ**

- フェーズ1：DescriptorAllocator の新設をやめ、SrvManager を拡張する。確かめる手段（Tests、CI、GraphicsDebugWindow）を加えた
- フェーズ2と3：AudioClip を加えた。LibraryArtifactProvider は作らない
- フェーズ5：描画リスト、シーン設定、音声、パッケージを加えた
- 旧版のフェーズ7（配布版）はなくなった

## 17. 未決事項と後回しにするもの

以下は仕様として確定していない。実装に着手するフェーズで確認・計測して決め、この章に書き戻す。

**実装時に確認・計測すること**

- AssetStorage で実体の置き場所を安定させる具体的な構造（フェーズ2）
- JobSystem のワーカー数と、インポートと読み込みの優先度の配分（フェーズ1〜3）
- 1フレームあたりの GPU 転送量の上限（フェーズ1）
- SRV ヒープの容量。初期値は 4096（フェーズ1）
- MeshRenderer のマテリアル数の上限。初期値は 8（フェーズ5。Assets/ のモデルでサブメッシュ数を数えて決める）
- コピーキューに切り替える場合のリソース状態の扱い（暗黙の状態遷移の規則）。切り替えるときに D3D12 の仕様で確認する
- CI の Windows ランナーで WARP が動くか（フェーズ1）
- slnx で、ソリューションの Release 構成から AssetPipeline を外す書き方（フェーズ3）
- Media Foundation をメモリ上のバイト列から使うときの手順と、対応する形式（フェーズ3）
- パッケージの出力にかかる時間（フェーズ5）
- 初回起動で Assets/ をすべてインポートするのにかかる時間（フェーズ3）

**後回しにするもの**（この設計書の範囲外）

- ファイル監視による自動 Refresh
- Project ウィンドウでのリネーム・移動・削除・複製
- グリッド表示とサムネイル
- 複数選択
- glTF・FBX のインポータ
- テクスチャの圧縮と最大サイズの設定
- 音声のストリーミング再生と、インポート設定（モノラル化など）
- AudioSource コンポーネントそのものの設計
- パッケージの差分更新と分割読み込み
- 配布用フォルダの自動組み立て（exe、DLL、シェーダーのソース、ProjectSettings のコピー）
- コマンドラインからのパッケージの出力と、CI でのパッケージの作成
- Debug をパッケージから起動するオプション
- バインドレス（SRV の固定番号を引き継ぐ）
- シェーダーのアセット化
- シーン設定へのスカイボックス以外の項目（環境光、背景色など）
