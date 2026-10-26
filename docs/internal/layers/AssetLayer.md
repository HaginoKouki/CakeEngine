# Asset 層

モデル、テクスチャ、マテリアルの読み込み、所有、索引を担当する層です。実体は各 Manager が連続配置のプールで持ち、外へは index と generation のハンドルを払い出します。

## この層が担当すること

Asset 層は「ファイルを読み込んで GPU リソースを作り、実行時のハンドルを返す」ことと、「GUID でアセットを引ける索引を持つ」ことを担当します。シーンの保存形式やコンポーネントは Scene 層、描画命令は Render 層の担当です。

## 配置

`project/Engine/Asset/`

## 構成

`Asset`（`Asset.h` / `.cpp`）が下記の部品を所有し、`Initialize` で Texture → Material → Model → AssetDatabase の順に初期化します。

### 稼働中の部品

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| `Texture/` | `TextureManager`、`TextureHandle`、`TextureData` | DirectXTex で読み込み（WIC、sRGB 強制、ミップマップ生成）、SRV を作る。パスでキャッシュし、失敗時は `errorTexture`（マゼンタと黒のチェッカー）を返す。ハンドルは GPU ディスクリプタハンドルを同梱する。ImGui プレビュー用に sRGB を剥がした SRV も 1 枚作る |
| `Material/` | `Material`、`MaterialOrigin` | シェーダー定義に従って cbuffer を確保し、CPU 側の値と GPU 側を `Apply` で同期する。テクスチャスロットと出自（Runtime / Asset / Embedded）、未保存フラグを持つ |
| `Material/` | `MaterialManager`、`MaterialHandle` | 生成、キャッシュ、`.mat` の保存。同名は "Name.1" のようにリネームする。失敗時は `errorMaterial` を返す |
| `Material/` | `MaterialSerializer` | `.mat`（JSON）の読み書き（`SaveMaterials`、`LoadMaterials`、`PeekMaterialInfo`） |
| `Model/` | `ModelManager`、`ModelData`、`ModelLoadDesc`、`ModelHandle`、`MeshHandle` | OBJ と MTL の読み込み、内蔵の基本図形の生成。パスでキャッシュする |
| `Model/` | `Mesh`、`SubMesh`、`VertexData` | 描画の最小単位。`SubMesh` は頂点バッファと、親 `Mesh` のどのマテリアルスロットを使うかを持つ |
| `Model/` | `PrimitiveShape.h` | 内蔵図形（Triangle、Quad、Cube、Sphere、Tetrahedron）の識別子と、固定の GUID とキー |
| `Database/` | `AssetDatabase`、`AssetEntry`、`AssetType`、`MaterialRefEntry` | `Assets/` 以下の走査、GUID とパスの相互索引、`{GUID, LocalId}` からのロード窓口 |
| `Database/` | `AssetMeta` | `.meta`（JSON）の読み書き。GUID、インポータ種別、インポート設定を持つ |
| `Database/` | `AssetRef<HandleT>`、`AssetTraits` | コンポーネントが持つアセット参照。保存されるのは `guid` と `localId` だけで、`handle` は実行時に `Resolve` で埋める |
| 直下 | `AssetTypes.h` | `ImporterType`（ファイル単位の分類。`.meta` に記録）、`ObjectType`（参照先の分類）、サブアセットのタグと ID 生成（`SubAssetTagOf`、`MakeSubAssetId`）、`SubAssetNameTable` |

### 型定義のみで未接続の部品

次の部品は `.cpp` が無く、ほかから使われていません。将来のインポートパイプラインの設計を型として先に置いたものです。

| フォルダ | 型 | 想定している役割 |
| --- | --- | --- |
| `Import/` | `AssetImporter`、`ImportContext` | 元ファイルからアーティファクト（バイト列）への変換の基底クラス。GPU に触らない |
| `Import/` | `ImportPipeline`、`ImportStatus` | インポータを束ね、キャッシュが有効なら再利用し、無ければインポートして保存する |
| `Import/` | `ArtifactCache` | `Library/` フォルダへの保管と読み戻し |
| `Import/` | `ArtifactKey` | 元ファイルの内容、インポート設定、インポータの版数から作る鍵 |
| `Artifact/` | `ModelArtifact`、`MeshArtifact`、`SubMeshArtifact`、`MaterialArtifact` | モデルのインポート結果。GPU リソースを持たない中間表現 |
| `Runtime/` | `HandlePool<T, HandleT>` | 3 つの Manager が個別に持つプール処理を 1 本化するための汎用プール。`Replace`（generation 据え置き）と `Recreate`（generation 更新）を区別する |

`AssetTypes.h` の `ObjectType` 以降（`SubAssetTagOf`、`MakeSubAssetId`、`SubAssetNameTable`）も、`AssetTypes` 自身以外からは使われていません。`AssetDatabase` は旧方式の `MakeLocalId`（名前のみ）で埋め込みマテリアルを探しています。

## 主要な仕組み

### ハンドル

3 つの Manager は同じ作法でハンドルを払い出します。

| 要素 | 意味 |
| --- | --- |
| `index` | Manager 内部の `std::vector` の添字。実体の場所 |
| `generation` | スロットが何代目か。将来アンロードでスロットを再利用したときに古いハンドルを検出する予約枠。現状アンロードは無いため常に 1 |

実体へは `Resolve(handle)` で到達します。返されたポインタは次の追加（`Emplace`）まで有効で、`vector` の再確保で移動します。ポインタを保持せず、必要なたびに `Resolve` します。`MaterialManager::CreateAssetFrom` はこの落とし穴を踏まないよう、`CreateAsset` の後に元のマテリアルを `Resolve` し直しています。

`TextureHandle` だけは `gpuHandle`（SRV の GPU ディスクリプタハンドル）を同梱します。描画側が `TextureManager` に問い合わせることなく、ハンドルをそのまま `SetGraphicsRootDescriptorTable` に渡せるようにするためです。

### 索引（AssetDatabase）と .meta

起動時に `AssetDatabase::Rescan` が `Assets/` 以下を再帰的に走査します。対象拡張子のファイルごとに `.meta` を読み、無ければ GUID を新規発行して書き出し、GUID とパスの相互索引を作ります。シーンやマテリアルは GUID だけを保存し、実行時にここでパスへ解決します。これにより、アセットを移動したり名前を変えたりしても参照が切れません。

| 項目 | 内容 |
| --- | --- |
| 対象拡張子 | モデルは `.obj`、`.gltf`、`.glb`、`.fbx`、テクスチャは `.png`、`.jpg`、`.jpeg`、`.dds`、`.tga`、`.bmp`、マテリアルは `.mat` |
| 対象外 | `.mtl`（OBJ の付属物として埋め込みマテリアルで扱う）、`.meta` 自身、上記以外の拡張子 |
| `.meta` の形式 | `guid`、`importer`（`ImporterType` の文字列）、`settings`（インポータごとの JSON）、`version`（現在 2）。タブでインデントして書く |
| 読めない `.meta` | GUID を振り直さず、エラーを出してそのアセットを飛ばす。振り直すと既存の参照が切れるため |
| 新しい版の `.meta` | 自エンジンより新しい `version` の `.meta` には書き込まない |
| GUID の重複 | `.meta` ごとコピーして同じ GUID が 2 つあると、後から見つかった方をエラーにして飛ばす |

実ファイルを持たない内蔵図形は、`PrimitiveShape.h` の固定 GUID（上位 64 ビットが `0xB0117100_00000000`、下位が 1 からの通し番号）と `Builtin/Cube` のようなキーで索引に登録されます。`MeshRendererComponent` はこれを通常のモデルと同じ `AssetRef<ModelHandle>` で指せます。

走査対象の拡張子に `.gltf`、`.glb`、`.fbx` が含まれていますが、`ModelManager::Load` が読めるのは OBJ だけです。

### ロード窓口

`AssetDatabase` は `{GUID, LocalId}` を受け取り、対応する Manager へ委譲してハンドルを返します。未登録、種別違い、Manager 未設定のいずれでも無効ハンドルを返し、落としません。

| 関数 | 動き |
| --- | --- |
| `LoadModel` | `localId` が 0 以外なら警告して無効ハンドル。内蔵図形は `ModelManager::FindHandle`、ファイルは `ModelManager::Load` |
| `LoadTexture` | `localId` が 0 以外なら警告して無効ハンドル |
| `LoadMaterial` | `localId` が 0 なら `.mat` を `MaterialManager::LoadFromFile`。0 以外なら GUID をモデルとみなして読み込み、埋め込みマテリアルを `LocalId` で探す |
| `ImportAsset` | 実行中に生成されたファイル（新しい `.mat` など）を 1 件だけ索引へ追加する |
| `GetSelectableMaterials` | インスペクタの選択候補。`.mat` 本体と、読み込み済みモデルの埋め込みマテリアルを並べる |

コンポーネントの `AssetRef` は、`SceneSerializer` が読み込み時に `Resolve` を呼び、さらにシーン読み込み直後に `ResolveSceneAssets`（Scene 層）が未解決のものを拾い直します。

### モデルの読み込み

`ModelManager::LoadObjFile` は OBJ を 1 行ずつ読み、`o` ごとに `Mesh`、`usemtl` ごとに `SubMesh` を作ります。面はファン分割で三角形にし、`ModelLoadDesc::flipHandedness`（既定 true）で X を反転して巻き順を入れ替えます。`mtllib` の `.mtl` は `newmtl` と `map_Kd` だけを読み、各マテリアルを `MaterialOrigin::Embedded` で `MaterialManager` に作らせます。マテリアルが 1 つも無い OBJ には、モデル名で既定のマテリアルを 1 つ作ります。

内蔵図形は `Initialize` で生成します。平面図形（Triangle、Quad）のオモテ面は `kPlaneFacingZ`（-Z）へ向け、既定のカメラから見えるようにしています。内蔵図形のマテリアルスロットは `errorMaterial` を指すため、`MeshRendererComponent::material` を設定しないとマゼンタで描かれます。

### マテリアルの出自

`Material` は「どこから来たか」を `MaterialOrigin` で持ち、これで編集の可否と保存先が決まります。

| 出自 | 由来 | 編集 | 保存先 |
| --- | --- | --- | --- |
| `Runtime` | コードが作った一時マテリアル（Skybox 用など） | 可 | 無し |
| `Asset` | `.mat` ファイル | 可 | 元の `.mat` へ上書き |
| `Embedded` | モデル（`.mtl`）に埋め込まれた既定マテリアル | 不可（読み取り専用） | 無し |

`Embedded` を編集不可にしているのは、モデルの既定を書き換えるとそのモデルを使う全オブジェクトへ影響し、しかも `.obj` や `.mtl` へは書き戻せないためです。編集したいときは `CreateAssetFrom` で `.mat` として切り出します。

編集は `MarkDirty` を立てるだけで、書き戻しは `SaveDirtyMaterials`（エディタの Ctrl+S）がまとめて行います。

### .mat の形式

`.mat` は JSON で、1 ファイルに 1 マテリアルを書きます。キーがマテリアル名、`shader` がシェーダー名、`params` が型ごとの配列、`textures` がスロット名からテクスチャパスへの対応です。未割り当て（白ダミー）のスロットは書きません。

```json
{
  "materials": {
    "PlayerMaterial": {
      "params": {
        "color": [1.0, 1.0, 1.0, 1.0],
        "lightingType": [1],
        "offset": [0.0, 0.0],
        "tiling": [1.0, 1.0]
      },
      "shader": "Standard",
      "textures": {
        "albedo": "Assets/kenney_cube-pets_1.0/Models/OBJ format/Textures/colormap.png"
      }
    }
  }
}
```

読み込みは先に `shader` を適用して cbuffer のレイアウトを決め、次に `params` をシェーダー定義の型で読み、最後に `textures` を `TextureManager::Load` で解決します。シェーダー側のスロットが増えて未割り当てが残った場合は白ダミーで埋めます。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則（`Asset.h` のコメント） | Graphics に依存する。下は Graphics、Platform、Foundation。上位層（Render、Scene、Editor）を知らない |
| 実装上の依存 | Foundation（Identity、Serialize、Math、Debug）、Graphics（`ShaderDefinition`、`ShaderLibrary`、`SrvManager`、`CommandManager`、`CreateBufferResource`） |
| 外部 | DirectXTex、nlohmann/json |
| 使っていない下位層 | Platform |
| 逆方向の参照 | Foundation 層の `TypeInfo.h` が `AssetRef` と各 Handle を include している。Graphics 層の `GPUResourceUtility.cpp` が `Material.h` を include している（未使用） |

## 守るべき規則

### ハンドルと読み込み失敗

- 外へ出すのは index と generation。`Resolve` が返すポインタは次の `Emplace` まで有効で、保持しない。
- 読み込み失敗でエンジンを落とさない。`TextureManager` は `errorTexture`、`MaterialManager` は `errorMaterial` を返し、欠損が見える状態で描画を続ける。`AssetDatabase` のロードは無効ハンドルを返す。
- `ModelManager::Load` は失敗時も空の `ModelData` を積んで有効なハンドルを返す（メッシュ 0 個）。

### 永続化する識別子

- 保存に使う参照は `{GUID, LocalId}` のみ（`AssetRef`）。ハンドルは保存しない。
- `.meta` は対象ファイルすべてに発行し、コミットする。読めない `.meta` があっても GUID を振り直さない。
- 内蔵図形の `kPrimitiveShapeGuids` と `kPrimitiveShapeKeys` は変更しない（保存済みシーンの参照が切れる）。図形を増やすときは enum と配列 2 本のすべてに追記する（要素数は `static_assert` で検査される）。
- モデルとテクスチャのサブアセット（`localId` が 0 以外）は未対応。マテリアルだけがモデル内の埋め込みマテリアルを指せる。
- `SubAssetTagOf` の 4 文字タグは変更しない（`LocalId` 経由で保存に焼き込まれる）。`AssetTypes.cpp` の `static_assert` が導出結果を固定している。

### マテリアル

- `Embedded` は読み取り専用で `dirty` にならない。編集したいときは `CreateAssetFrom` で `.mat` として切り出す。
- `MaterialManager::CreateAsset` は `AssetDatabase` への登録をしない。呼び出し側で `AssetDatabase::ImportAsset` を呼ぶ（`AssetDatabase` → `MaterialManager` の依存があり、逆には参照できないため）。
- `Material::SetTexture` はシェーダー定義に無いスロット名を警告して無視する。

### インポートとデータ配置

- Importer は GPU に触れない（`ImportContext` に device を入れない）。処理を変えたら `GetVersion()` を必ず上げる。
- `.mat` や `.scene` のようなエディタで編集するアセットは Importer を通さず、専用のシリアライザで読み書きする。

## 未実装・要確認

- `generation` は将来のアンロード用の予約枠で、現状は常に 1 です。
- `Import/`、`Artifact/`、`HandlePool` は型定義のみです。ヘッダコメントに「フェーズ 2」「フェーズ 5」「仕様書 12 章」などの参照がありますが、該当する計画書はリポジトリにありません。
- `AssetDatabase.h` の `AssetType` と `AssetTypes.h` の `ImporterType` は、どちらもファイルの分類を表します。`RegisterAsset` が `ToImporterType` で変換して `.meta` に書いており、2 つの列挙が並存しています。
- `.gltf`、`.glb`、`.fbx` は索引に載りますが、読み込みは OBJ のみ対応です。
- `TextureManager::RegisterTexture` はテクスチャ 1 枚ごとに `ExecuteAndWait` を呼びます。起動時のアップロードは逐次です。
- `Material::LoadMaterialTemplateFile` と `FindMaterialByName` は旧実装で、現在は `ModelManager::LoadMtlFile` が同じ役割を担っています。呼び出し元はありません。
