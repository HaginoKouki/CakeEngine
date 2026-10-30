# Foundation 層

最下層です。数学、識別子、ログ、シリアライズ、リフレクションなど、GPU やゲームの概念を持たない部品を置きます。

## この層が担当すること

Foundation 層は、ほかのすべての層が使う値型と小さな道具を提供します。ウィンドウや描画のような OS や GPU に結びついた処理は、Platform 層と Graphics 層が担当します。

## 配置

`project/Engine/Foundation/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| `Math/` | `Vector2/3/4`、`Matrix2x2/3x3/4x4`、`Quaternion`、`Transform` | 値型。算術演算子と、型を作る静的関数（`Make～`、`FromEuler`、`FromDegreeUnit` など）だけを持つ。行列は行ベクトル規約 |
| `Math/` | `VectorMath.h`、`MatrixMath.h`、`QuaternionMath.h`（`Math` 名前空間） | 長さ、正規化、内積と外積、逆行列と転置、共役と逆数など、型に対する計算。`Math::TransformCoord` は `v * M` の順に掛けて w で割り、`Math::TransformNormal` は平行移動を掛けない |
| `Math/` | `Math.h` | 上の型と計算関数をまとめて include するだけのヘッダ |
| `Math/` | `Geometry.h`（`Line`、`Ray`、`Segment`、`Triangle`、`Quad`、`Plane`、`Sphere`、`Capsule`、`AABB`、`OBB`） | 当たり判定や可視化に使う基本図形 |
| `Math/` | `IsCollision` | 図形同士の衝突判定（球、平面、三角形、AABB、OBB の組み合わせ） |
| `Math/` | `Easing.h` | イージング関数群と `Lerp`。`Vector2/3/4` と `Quaternion` の成分ごとの補間、`Vector3` と `Quaternion` の `Slerp` |
| `Math/` | `Gradient` | 0 から 1 の位置に対して色と透明度を返す。パーティクルの Color over Lifetime 用 |
| `Math/` | `Math::DecomposeAffine` | アフィン行列を scale / rotate / translate へ分解する。親の張り替えとギズモ操作で使う |
| `Identity/` | `Guid` | 128 ビットの識別子。アセットのファイルごとに 1 つ |
| `Identity/` | `LocalId` | ファイル内のサブオブジェクトを指す 64 ビット値。名前と種別タグから導出する |
| `Identity/` | `AssetId` | `Guid` と `LocalId` の組。24 バイトの値型 |
| `Debug/` | `DebugLog`、`AssertHRESULT` | シングルトンのログ。日時別のファイル出力、画面表示用の一覧、HRESULT の解析 |
| `Serialize/` | `JsonFile` | JSON ファイルの読み書きの定型を 1 本化したもの |
| `Serialize/` | `BinaryWriter`、`BinaryReader` | インポート結果を生のバイナリで読み書きする最小のストリーム |
| `Reflection/` | `TypeInfo`、`PropertyDesc`、`PropertyType`、`NumericWidget` | 構造体のメンバ構成を実行時に読めるデータにしたもの |
| `Reflection/` | `ReflectMacros.h`（`CAKE_REFLECT`、`CAKE_PROPERTY` 系） | 構造体定義の隣に書いて `TypeInfo` を作るマクロ |
| `Utility/` | `Convert` | `std::string` と `std::wstring` の相互変換（UTF-8） |

## 主要な仕組み

### 数学ライブラリの分け方

数学ライブラリは「型」と「型に対する計算」をファイルと名前空間で分けています。

| 置き場所 | 中身 | 例 |
| --- | --- | --- |
| `Vector.h`、`Matrix.h`、`Quaternion.h` | 値型そのもの。算術演算子と、その型を作る静的関数 | `Vector3::Zero`、`Matrix4x4::MakeAffineMatrix`、`Quaternion::FromEuler`、`Quaternion::Multiply` |
| `VectorMath.h`、`MatrixMath.h`、`QuaternionMath.h` | `Cake::Math` 名前空間のフリー関数。型を受け取って別の値を返す計算 | `Math::Length`、`Math::Normalize`、`Math::DotProduct`、`Math::Inverse`、`Math::Transpose`、`Math::TransformCoord` |
| `Math.h` | 上の 6 ファイルをまとめて include する | 計算関数を使う `.cpp` はこれを include する |

型のヘッダは互いを前方宣言で参照するだけなので、`Vector.h` を include してもほかの型は入ってきません。計算関数の多くは同じ名前で型ごとに多重定義されていて（`Math::Normalize` はベクトルと `Quaternion` の両方にある）、引数は `float` もベクトルも値渡しです。座標変換が `Transform` ではなく `TransformCoord` という名前なのは、姿勢を表す `Cake::Transform` と名前が重なるのを避けるためです。

`Vector3::Multiply` は成分ごとの積、`Quaternion::Multiply` は回転の合成（ハミルトン積）で、同じ名前でも意味が違います。

### 識別子

アセットの参照は `Guid` と `LocalId` の組で表します。`Guid` はインポート時に 1 回だけ `CoCreateGuid` で生成し、`.meta` ファイルに保存します。パスや内容から計算しないため、ファイルを移動したり名前を変えたりしても参照が切れません。全ビットが 0 の値は無効を表します。文字列化は区切りなし 32 桁の小文字 16 進で、UUID 標準のバイト順とは一致しません（`TryParse` と対で使います）。

`LocalId` は 1 つのファイルの中にある複数のオブジェクト（OBJ に埋め込まれたマテリアルなど）を区別します。値はファイルに書かれている名前と 4 文字の種別タグから FNV-1a で導出します。連番にしないのは、再インポートで順序が変わったときに参照が別のものを指すのを避けるためです。0 は本体を指す予約値です。

### リフレクション

`TypeInfo` は型名と、メンバごとの `PropertyDesc`（シリアライズ用の `name`、表示用の `label`、`PropertyType`、バイトオフセット、UI のウィジェット種別と範囲、ドラッグ速度）を持ちます。インスペクタの描画、JSON への保存、JSON からの復元は、この一覧を回すだけの汎用コードで書かれています。コンポーネントを 1 つ足すときに触る場所が、その型の定義だけで済むのはこの仕組みのためです。

メンバへは「インスタンスの先頭アドレス + offset」で到達します。これが成立するのは、対象が仮想関数を持たない素の構造体だからです。

`PropertyType` には次の値があります。

| 分類 | 値 |
| --- | --- |
| 基本 | `Bool`、`Int`、`Float`、`String` |
| 数学 | `Vector2`、`Vector3`、`Vector4`、`Color`（レイアウトは `Vector4` と同じで UI がカラーピッカーになる）、`Transform`、`Gradient` |
| 参照 | `AssetRefModel`、`AssetRefTexture`、`AssetRefMaterial`、`EntityRef`（同じシーン内の `GameObjectId`） |

C++ の型から `PropertyType` を引く対応表が `PropertyTypeOf` です。対応表に無い型を `CAKE_PROPERTY` に渡すと、`static_assert` でコンパイルエラーになります。

`CAKE_PROPERTY` 系のマクロは次のとおりです。

| マクロ | 用途 |
| --- | --- |
| `CAKE_PROPERTY(member, "Label")` | 型を自動判別して登録する。通常はこれを使う |
| `CAKE_PROPERTY_MIN(member, "Label", min)` | 下限だけを縛る Drag |
| `CAKE_PROPERTY_MAX(member, "Label", max)` | 上限だけを縛る Drag |
| `CAKE_PROPERTY_CLAMP(member, "Label", min, max)` | 両側を縛る Drag |
| `CAKE_PROPERTY_RANGE(member, "Label", min, max)` | Slider。範囲が狭く端から端まで動かしたい値に使う |
| `CAKE_PROPERTY_DRAG(member, "Label", min, max, speed)` | ドラッグ速度まで指定する。桁が大きい値に使う |
| `CAKE_PROPERTY_COLOR(member, "Label")` | `Vector4` をカラーピッカーとして扱う |

### ログ

`DebugLog` はファイルとデバッグ出力には `Log` の中ですぐ書き、画面表示用の一覧には保留キューに積むだけにします。メインスレッドが毎フレーム `PumpMainThread` を呼んで、保留キューの中身を表示用一覧へ移します。表示用一覧をメインスレッドだけが触る形にしているのは、`LogWindow` が一覧を参照で受け取って描画している間に、別スレッドの追加で配列が再確保されるのを防ぐためです。

最初に `GetInstance` を呼んだスレッドをメインスレッドとみなします。`Application::Initialize` の先頭でログを出すので、ワーカースレッドを起動する前に必ず決まります。ログファイルは実行ディレクトリの `logs/<日付>/<時刻>.log` に出力され、1 行ごとに flush します。

`AssertHRESULT(hr, message)` は失敗した HRESULT をログへ出し（名前と説明の対応表を内蔵）、`assert(false)` で止めます。

### Gradient

`Gradient` は色のキーと透明度のキーを別々の固定長配列（各 8 個まで）で持ちます。`std::vector` ではなく固定長配列にしているのは、コンポーネントに直接持たせるためです（コンポーネントは固定サイズの素の構造体に保つ決まりがあります）。

キーの並びは保存順のままで、time の昇順とは限りません。エディタでドラッグ中に並べ替えると、触っているキーが入れ替わるためです。`Evaluate` は昇順を前提にするので、使う側が `Sorted()` のコピーを作ってから呼びます。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | ほかの層にも OS にも依存しない |
| 実装上の外部依存 | nlohmann/json（`JsonFile.h`）、Win32（`DebugLog.h`、`Guid.cpp`、`Convert.cpp`） |
| 規則との食い違い | `Reflection/TypeInfo.h` が Asset 層の `AssetRef.h`、`MaterialHandle.h`、`ModelHandle.h`、`TextureHandle.h` と、Scene 層の `GameObjectId.h` を include している。`PropertyType` の `AssetRef～` と `EntityRef` に対応する `PropertyTypeOf` の特殊化のため |
| 規則との食い違い | `Debug/DebugLog.h` と `Utility/Convert.cpp` が `Windows.h` に依存している |
| コメントの不一致 | `Identity/AssetId.h` のコメントに「AssetPipeline 層」という記述があるが、該当する層は存在しない |

## 守るべき規則

### 数学

- 型のヘッダには算術演算子と「その型を作る」静的関数だけを置く。長さや逆行列のような計算は `Math` 名前空間のフリー関数にする。
- 計算関数の引数は値渡しにする（`Quaternion` を受け取る `Math::Rotate` だけは参照）。
- 座標変換は `Math::TransformCoord`、法線や方向は `Math::TransformNormal` を使う。`Transform` という名前の関数は作らない（`Cake::Transform` と衝突する）。
- `Math::Normalize` は長さ 0 のベクトルに `Zero` を返し、`Math::Inverse(Matrix4x4)` は行列式 0 に `Matrix4x4::Zero` を返す。`Math::Clamp` は `min <= max` を `assert` で検査する。

### 識別子

- `Guid` は乱数で生成して `.meta` に永続化する。パスや内容から計算しない。
- `LocalId` の導出元にしてよい名前は、ファイルに書かれている名前だけにする。エンジンが衝突回避で付け直した名前（"Mat.1" など）は使わない。
- 種別タグを混ぜて導出する。入口は Asset 層の `MakeSubAssetId` で、名前だけから導出する `MakeLocalId` は非推奨。
- `AssetId` は 24 バイトの trivially copyable を保つ（`static_assert` あり）。メンバを足さない。

### リフレクション

- 対象は仮想関数を持たない素の構造体にする。基底クラスを付けない。
- `name` はシリアライズのキーなので、変更すると既存のファイルを読めなくなる。`label` は表示名なので変更してよい。
- `CAKE_REFLECT` ブロックは構造体定義と同じヘッダに置く。
- `PropertyType` を追加したら、`PropertyTypeOf` の特殊化、`PropertyDrawer`（Editor 層）、`SceneSerializer`（Scene 層）の 3 箇所を揃える。

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
- `Easing.h` の `Lerp` は `Gradient` が使っていますが、`Ease` 系、`Slerp`、`CatmullRom` を使っている箇所はエンジン内にありません。`VectorMath.h` の `Rotate`、`Distance`、`Clamp`、角度の変換、`QuaternionMath.h` の全関数、`Quaternion` の静的関数も同様に呼び出し元がありません。
- `Math::Inverse` は `Matrix4x4` だけ行列式 0 を検査して `Zero` を返し、`Matrix2x2` と `Matrix3x3` は検査せずに 0 で割ります。
