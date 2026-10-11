# Foundation層
*基礎の基礎*



最下層。数学・識別子・ログ・シリアライズ・リフレクションなど、GPUやゲームの概念を持たない部品を置きます。



## 場所
`project/Engine/Foundation/`



## 構成
- Math/ … Vector2/3/4、Matrix2x2/3x3/4x4、Quaternion、Transform、Geometry（Line/Ray/Segment/Sphere/AABB/OBB/Plane など）、IsCollision（図形同士の衝突判定）、Easing（イージング関数群、Lerp、Bezier、CatmullRom）、Gradient（0〜1 の位置に対して色と透明度を返す。ColorKey / AlphaKey を各 kMaxKeys=8 個まで固定長配列で持つ。パーティクルの Color over Lifetime 用）
- Identity/ … Guid（128bit、インポート時に1度だけ生成し.metaへ保存）、LocalId（ファイル内サブオブジェクトの識別子。名前と種別タグのFNV-1aから導出）、AssetId（{Guid, LocalId}の24バイト値型）
- Debug/ … DebugLog（シングルトン。日時別ログファイル出力、画面表示用一覧、HRESULT解析）
- Serialize/ … JsonFile（読み書きの定型を1本化）、BinaryStream（BinaryWriter/BinaryReader。アーティファクト用の生バイナリ）
- Reflection/ … TypeInfo（メンバ構成を実行時データ化。PropertyType は Bool / Int / Float / Vector2 / Vector3 / Vector4 / Color / String / Transform / Gradient / AssetRefModel / AssetRefTexture / AssetRefMaterial / EntityRef）、ReflectMacros（CAKE_REFLECT / CAKE_PROPERTY 系マクロ）
- Utility/ … Convert（std::string と std::wstring の相互変換）

## 依存関係
規則（Overview.md）:
- 自身とその関連ファイル以外に依存しない。OSにも依存しない。

現状の実装:
- 外部ライブラリは nlohmann/json のみ（JsonFile.h）。
- 層内のinclude は Foundation 内部同士（例: Serialize/JsonFile.cpp → Debug/DebugLog.h）のみ。

規則との食い違い（現状の実装）:
- Windows.h に依存している: Debug/DebugLog.h、Utility/Convert.cpp。「OSに依存しない」とは現状一致しない。
- Reflection/TypeInfo.h が上位層をincludeしている: Asset層の AssetRef.h / MaterialHandle.h / ModelHandle.h / TextureHandle.h、Scene層の GameObjectId.h。PropertyType（AssetRefModel、EntityRef など）とPropertyTypeOfの特殊化のため。
- Identity/AssetId.h のコメントに「AssetPipeline層」という記述があるが、該当する層は現状ありません。

## 規則

### 識別子と保存形式

- Identity: GUID は乱数生成して.metaに永続化する。パスや内容から計算しない。全ビット0は無効値。
- LocalId: 導出元にしてよい名前は「ファイルに書かれている名前」だけ。エンジンが付け直した名前（"Mat.1" など）を使わない。0は本体を指す予約値。種別タグを混ぜて導出する（Asset層の MakeSubAssetId が入口）。旧方式の MakeLocalId は非推奨で、旧AssetDatabaseの埋め込みマテリアル探索だけが使う。
- AssetId は24バイトの trivially copyable を保つ（static_assert あり）。メンバを足さない。

### リフレクション

- 対象は仮想関数を持たない素の構造体。`offsetof` でメンバへ到達するため、基底クラスを付けない。
- `name` はシリアライズのキーなので変更すると既存ファイルを読めなくなる。`label` は表示名なので変更できる。
- `CAKE_REFLECT` ブロックは構造体定義と同じヘッダに置く。
- `PropertyType` を追加したら、インスペクタ（`PropertyDrawer`）とシリアライザ（`SceneSerializer`）のswitchにも追加する。

### グラデーション（Gradient）

- キー列は保存順のままで、time の昇順とは限らない（エディタでドラッグ中に並べ替えると、触っているキーが入れ替わるため）。`Evaluate` は昇順を前提にするので、使う側が `Sorted()` のコピーを作ってから呼ぶ。粒ごとではなく、エミッターごとに1フレーム1回だけ並べ替える。
- キー数は 1〜kMaxKeys（8）。範囲外の個数は `ClampKeyCount` で収め、配列の外を読まない。`std::vector` ではなく固定長配列なのは、コンポーネントに直接持たせる（standard-layout を保つ）ため。
- 最初のキーより前は最初の値、最後のキーより後は最後の値、間は前後2キーの線形補間。
- JSON には `{"colorKeys": [{time, color}], "alphaKeys": [{time, alpha}]}` で保存し、個数は配列長から決める（`SceneSerializer`）。9個目以降は読み捨て、1つも読めなかった列は既定値（白・不透明の2キー）のまま。

### バイナリとJSON

- `BinaryStream` にポインタ、ハンドル、`ComPtr` を書かない。エンディアン変換もしない（Libraryは再生成可能なキャッシュ）。
- `BinaryReader` は範囲外アクセスで失敗フラグを立てる。読み込み後に `IsFailed()` を1回確認する。
- `JsonFile` は設定ファイルが壊れていても落とさず、ログを出して `false` を返す。書き込み時は親フォルダを作る。

### ログ

- `Log` 系はどのスレッドから呼んでもよい。`GetLogEntry` / `PumpMainThread` はメインスレッド専用。
- 最初に `GetInstance` を呼んだスレッドをメインスレッドとみなす。終了時までにワーカースレッドを停止する。
