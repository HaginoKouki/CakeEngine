# Asset層
*3Dモデルやテクスチャ、マテリアルなど*<br>
<br>
アセットの読み込み・所有・索引を担当します。実体は各マネージャが連続配置のプールで持ち、外へは index + generation のハンドルを払い出します。<br>
<br>
## 場所
`project/Engine/Asset/`<br>
<br>
## 構成
Asset（Asset.h/.cpp）が下記の部品を所有し、Initialize で順に初期化します（Texture → Material → Model → AssetDatabase）。<br>
<br>
稼働中:
- Texture/ … TextureManager（DirectXTexで読込、SRV生成、パスでキャッシュ。失敗時は errorTexture を返す）、TextureHandle（gpuHandle同梱）。
- Material/ … Material（cbufferのCPU/GPU同期、テクスチャスロット、MaterialOrigin: Runtime / Asset / Embedded、dirty管理）、MaterialManager（生成・キャッシュ・.mat保存）、MaterialSerializer（.mat JSON）、MaterialHandle。
- Model/ … ModelManager（OBJ/MTL読込、基本図形の生成）、Mesh / SubMesh、ModelHandle / MeshHandle、PrimitiveShape（内蔵図形と固定GUID）。
- Database/ … AssetDatabase（Assets/以下のスキャン、GUID⇄パス索引、{GUID, LocalId}からのロード窓口）、AssetMeta（.meta JSON）、AssetRef<HandleT>（保存されるのはguidとlocalIdのみ）。
- Sprite/ … Sprite（画面配置の情報とマテリアルを持つ2D描画データ）。
- AssetTypes.h/.cpp … ImporterType（.metaに記録するファイル単位の分類。AssetMeta / AssetDatabase が使用中）、ObjectType、サブアセットのタグとID生成（SubAssetTagOf / MakeSubAssetId）、SubAssetNameTable。ObjectType 以降は AssetTypes 自身以外から使われていない。

型定義のみで未接続（.cpp が無く、他から使われていない）:
- Import/ … AssetImporter（基底）、ImportPipeline、ArtifactCache（Libraryフォルダ）、ArtifactKey。
- Artifact/ModelArtifact.h … モデルのインポート結果（GPUリソースを持たない中間表現）。
- Runtime/HandlePool.h … 各マネージャが個別に持つプール処理を1本化するための汎用プール。まだ3マネージャは使っていない。

## 依存関係
規則:
- Overview.md では Asset層は Graphics層より下に記載されている。
- 一方 Asset.h のコメントは「Graphicsに依存する。下は Graphics(RHI), Platform, Foundation」と書いており、Graphics.h のコメントも同じ前提（上位層に Asset を含む）。

注意: 上の2つは食い違っています。実装は後者（AssetがGraphicsに依存）です。どちらを正とするか要判断。

現状の実装:
- Foundation … Identity、Serialize、Math、Debug など。
- Graphics … Material.h / ModelManager.h → Shader/ShaderDefinition.h、MaterialManager.cpp / MaterialSerializer.cpp → ShaderLibrary.h、TextureManager.h → Descriptor/DescriptorHeap.h、TextureManager.cpp → Device/CommandManager.h、Material.cpp / ModelManager.cpp / TextureManager.cpp → Buffer/GPUResourceUtility.h。
- 外部 … DirectXTex、nlohmann/json。
- Platform・Scene・Render・Editor・Application には依存しない。

逆方向（他層 → Asset）:
- Foundation/Reflection/TypeInfo.h が AssetRef と各Handleをincludeしている（Foundationの項を参照）。
- Graphics/Buffer/GPUResourceUtility.cpp が Material.h をincludeしているが、ファイル内で Material は使っていない（不要なinclude）。

## 規則
- ハンドル: 外へ出すのは index + generation。Resolve が返すポインタは次の Emplace まで有効で、保持しない。必要なたびに Resolve する。
- 読み込み失敗でエンジンを落とさない。TextureManager は errorTexture、MaterialManager は errorMaterial を返し、欠損が見える状態で描画を続ける。AssetDatabase のロードは無効ハンドルを返す（落とさない）。
- ModelManager::Load は失敗時も空の ModelData を積んで有効なハンドルを返す（メッシュ0個）。
- 保存に使う参照は {GUID, LocalId} のみ（AssetRef）。ハンドルは実行時のみ有効で、保存しない。読み込み後に Resolve で埋める。
- .meta: 全ファイル・全フォルダに発行する（Assetsルート自身を除く）。既存の.metaが読めなかったら GUID を振り直さず、エラーを出してそのアセットを飛ばす。.meta のバージョンが自エンジンより新しい場合は書き込まない。.metaはコミットする。
- 内蔵基本図形（Builtin/Triangle など）は固定GUID・固定キーを持つ。kPrimitiveShapeGuids と kPrimitiveShapeKeys は変更禁止（保存済みシーンの参照が切れる）。増やすときは enum と配列2本のすべてに追記する。
- MaterialOrigin::Embedded（モデル埋め込み）は読み取り専用で dirty にならない。編集したいときは CreateAssetFrom で .mat として切り出す。保存は SaveDirtyMaterials（Ctrl+S）。
- MaterialManager::CreateAsset はAssetDatabaseへの登録をしない。呼び出し側で AssetDatabase::ImportAsset を呼ぶ（AssetDatabase → MaterialManager の依存があり、逆に参照できないため）。
- モデルとテクスチャのサブアセット（localId が非0）は未対応。非0で呼ぶと警告して無効ハンドルを返す。マテリアルのみ、モデル内の埋め込みマテリアルを指せる。
- SubAssetTagOf の4文字タグは変更禁止（LocalId経由で保存に焼き込まれる）。
- Importer は GPU に触れない（ImportContext に device を入れない）。処理を変えたら GetVersion() を必ず上げる。.mat / .scene のようなネイティブアセットは Importer を通さず専用シリアライザを使う。
- SpriteTransform は HLSL の cbuffer と1バイトも違ってはいけない（32バイトを static_assert で固定）。

## 未実装・作業途中（ソースから読み取れる範囲）
- generation は「将来アンロード用」の予約枠で、現状アンロードは無いためマネージャ側では常に 1。
- Import/・Artifact/・HandlePool は型定義のみ（AssetTypes の ObjectType / SubAsset 系も未使用）。フェーズ番号のコメントがあるが、計画の全体像はソースからは確認できません。
