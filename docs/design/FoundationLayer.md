# Foundation 層

最下層です。数学、識別子、ログ、シリアライズ、リフレクションなど、GPU やゲームの概念を持たない部品を置きます。

## この層が担当すること

Foundation 層は、ほかのすべての層が使う値型と小さな道具を提供します。ウィンドウや描画のような OS や GPU に結びついた処理は、Platform 層と Graphics 層が担当します。

## 配置

`project/Engine/Foundation/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| `Math/` | `Vector2/3/4`、`Matrix2x2/3x3/4x4`、`Quaternion`、`Transform` | ベクトル、行列、姿勢の計算 |
| `Math/` | `Geometry.h`（`Line`、`Ray`、`Segment`、`Sphere`、`AABB`、`OBB`、`Plane` など） | 当たり判定や可視化に使う基本図形 |
| `Math/` | `IsCollision` | 図形同士の衝突判定 |
| `Math/` | `Easing.h` | イージング関数群、`Lerp`、`Bezier`、`CatmullRom` |
| `Math/` | `Gradient` | 0 から 1 の位置に対して色と透明度を返す。パーティクルの Color over Lifetime 用 |
| `Identity/` | `Guid` | 128 ビットの識別子。アセットのファイルごとに 1 つ |
| `Identity/` | `LocalId` | ファイル内のサブオブジェクトを指す 64 ビット値。名前と種別タグから導出する |
| `Identity/` | `AssetId` | `Guid` と `LocalId` の組。24 バイトの値型 |
| `Debug/` | `DebugLog` | シングルトンのログ。日時別のファイル出力、画面表示用の一覧、HRESULT の解析 |
| `Serialize/` | `JsonFile` | JSON ファイルの読み書きの定型を 1 本化したもの |
| `Serialize/` | `BinaryWriter`、`BinaryReader` | インポート結果を生のバイナリで読み書きする最小のストリーム |
| `Reflection/` | `TypeInfo`、`PropertyDesc`、`PropertyType` | 構造体のメンバ構成を実行時に読めるデータにしたもの |
| `Reflection/` | `ReflectMacros.h`（`CAKE_REFLECT`、`CAKE_PROPERTY` 系） | 構造体定義の隣に書いて `TypeInfo` を作るマクロ |
| `Utility/` | `Convert` | `std::string` と `std::wstring` の相互変換 |

## 主要な仕組み

### 識別子

アセットの参照は `Guid` と `LocalId` の組で表します。`Guid` はインポート時に 1 回だけ乱数で生成し、`.meta` ファイルに保存します。パスや内容から計算しないため、ファイルを移動したり名前を変えたりしても参照が切れません。全ビットが 0 の値は無効を表します。

`LocalId` は 1 つのファイルの中にある複数のオブジェクト（OBJ に埋め込まれたマテリアルなど）を区別します。値はファイルに書かれている名前と 4 文字の種別タグから FNV-1a で導出します。連番にしないのは、再インポートで順序が変わったときに参照が別のものを指すのを避けるためです。0 は本体を指す予約値です。

### リフレクション

`TypeInfo` は型名と、メンバごとの `PropertyDesc`（シリアライズ用の `name`、表示用の `label`、`PropertyType`、バイトオフセット、UI の範囲）を持ちます。インスペクタの描画、JSON への保存、JSON からの復元は、この一覧を回すだけの汎用コードで書かれています。コンポーネントを 1 つ足すときに触る場所が、その型の定義だけで済むのはこの仕組みのためです。

メンバへは「インスタンスの先頭アドレス + offset」で到達します。これが成立するのは、対象が仮想関数を持たない素の構造体だからです。

`PropertyType` には次の値があります。

| 分類 | 値 |
| --- | --- |
| 基本 | `Bool`、`Int`、`Float`、`String` |
| 数学 | `Vector2`、`Vector3`、`Vector4`、`Color`（レイアウトは `Vector4` と同じで UI がカラーピッカーになる）、`Transform`、`Gradient` |
| 参照 | `AssetRefModel`、`AssetRefTexture`、`AssetRefMaterial`、`EntityRef`（同じシーン内の `GameObjectId`） |

C++ の型から `PropertyType` を引く対応表が `PropertyTypeOf` です。対応表に無い型を `CAKE_PROPERTY` に渡すと、`static_assert` でコンパイルエラーになります。

### ログ

`DebugLog` はファイルとデバッグ出力には `Log` の中ですぐ書き、画面表示用の一覧には保留キューに積むだけにします。メインスレッドが毎フレーム `PumpMainThread` を呼んで、保留キューの中身を表示用一覧へ移します。表示用一覧をメインスレッドだけが触る形にしているのは、`LogWindow` が一覧を参照で受け取って描画している間に、別スレッドの追加で配列が再確保されるのを防ぐためです。

最初に `GetInstance` を呼んだスレッドをメインスレッドとみなします。`Application::Initialize` の先頭でログを出すので、ワーカースレッドを起動する前に必ず決まります。ログファイルは実行ディレクトリの `logs/<日付>/` に出力されます。

### Gradient

`Gradient` は色のキーと透明度のキーを別々の固定長配列（各 8 個まで）で持ちます。`std::vector` ではなく固定長配列にしているのは、コンポーネントに直接持たせるためです（コンポーネントは固定サイズの素の構造体に保つ決まりがあります）。

キーの並びは保存順のままで、time の昇順とは限りません。エディタでドラッグ中に並べ替えると、触っているキーが入れ替わるためです。`Evaluate` は昇順を前提にするので、使う側が `Sorted()` のコピーを作ってから呼びます。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | ほかの層にも OS にも依存しない |
| 実装上の外部依存 | nlohmann/json（`JsonFile.h`） |
| 規則との食い違い | `Reflection/TypeInfo.h` が Asset 層の `AssetRef.h`、`MaterialHandle.h`、`ModelHandle.h`、`TextureHandle.h` と、Scene 層の `GameObjectId.h` を include している。`PropertyType` の `AssetRef～` と `EntityRef` に対応する `PropertyTypeOf` の特殊化のため |
| 規則との食い違い | `Debug/DebugLog.h` と `Utility/Convert.cpp` が `Windows.h` に依存している |
| コメントの不一致 | `Identity/AssetId.h` のコメントに「AssetPipeline 層」という記述があるが、該当する層は存在しない |

## 守るべき規則

### 識別子

- `Guid` は乱数で生成して `.meta` に永続化する。パスや内容から計算しない。
- `LocalId` の導出元にしてよい名前は、ファイルに書かれている名前だけにする。エンジンが衝突回避で付け直した名前（"Mat.1" など）は使わない。
- 種別タグを混ぜて導出する。入口は Asset 層の `MakeSubAssetId` で、名前だけから導出する `MakeLocalId` は非推奨。
- `AssetId` は 24 バイトの trivially copyable を保つ（`static_assert` あり）。メンバを足さない。

### リフレクション

- 対象は仮想関数を持たない素の構造体にする。基底クラスを付けない。
- `name` はシリアライズのキーなので、変更すると既存のファイルを読めなくなる。`label` は表示名なので変更してよい。
- `CAKE_REFLECT` ブロックは構造体定義と同じヘッダに置く。
- `PropertyType` を追加したら、`PropertyDrawer`（Editor 層）と `SceneSerializer`（Scene 層）の switch にも追加する。

### Gradient

- `Evaluate` を呼ぶ前に `Sorted()` のコピーを作る。粒ごとではなく、エミッターごとに 1 フレーム 1 回だけ並べ替える。
- キー数は 1 から `kMaxKeys`（8）に収める。範囲外の個数は `ClampKeyCount` で収め、配列の外を読まない。
- 最初のキーより前は最初の値、最後のキーより後は最後の値、間は前後 2 キーの線形補間になる。
- JSON には `{"colorKeys": [{time, color}], "alphaKeys": [{time, alpha}]}` で保存し、個数は配列長から決める。9 個目以降は読み捨て、1 つも読めなかった列は既定値（白・不透明の 2 キー）のままにする。

### バイナリと JSON

- `BinaryStream` にポインタ、ハンドル、`ComPtr` を書かない。エンディアン変換もしない（Library は再生成できるキャッシュのため）。
- `BinaryReader` は範囲外アクセスで失敗フラグを立てる。読み込み後に `IsFailed()` を 1 回確認する。
- `JsonFile` は設定ファイルが壊れていても落とさず、ログを出して `false` を返す。書き込み時は親フォルダを作る。

### ログ

- `Log` 系はどのスレッドから呼んでもよい。`GetLogEntry` と `PumpMainThread` はメインスレッド専用。
- 終了時までにワーカースレッドを停止する。

## 未実装・要確認

- `BinaryStream` を使う Import 系の型（Asset 層）は定義のみで、まだ接続されていません。
- `DebugLog.h` のコメントは「JobSystem の終了処理で保証する」と書いていますが、JobSystem は存在しません。
