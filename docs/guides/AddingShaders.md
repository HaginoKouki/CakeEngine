# シェーダーとマテリアルを追加する

新しいシェーダーを登録し、マテリアルから選べるようにするまでの手順です。シェーダーの定義、PSO、マテリアルの仕組みは [GraphicsLayer.md](../design/GraphicsLayer.md) と [AssetLayer.md](../design/AssetLayer.md) にあります。

## 全体の流れ

1. HLSL ファイル（VS と PS）を書く。
2. `ShaderDefinition` を作り、`ShaderLibrary::Register` に渡す。登録時に cbuffer のレイアウト計算と PSO 生成まで行われる。
3. マテリアルでそのシェーダー名を選ぶ。エディタの Material List から新しい `.mat` を作り、シェーダーを切り替える。

## 1. HLSL を書く

| 置き場所 | 用途 |
| --- | --- |
| `project/EngineResources/shaders/<名前>/` | エンジンの既定シェーダー |
| `project/Assets/Shaders/<名前>/` | ゲーム固有のシェーダー |

ルートシグネチャは 1 つを全シェーダーで共有するため、レジスタの割り当ては固定です。

| レジスタ | ステージ | 内容 |
| --- | --- | --- |
| `b0` | PS | マテリアルのパラメータ。`ShaderDefinition::params` の並びと一致させる |
| `b0` | VS | 変換行列。`TransformationMatrix`（`WVP` と `World` の `float4x4` が 2 つ） |
| `b1` | PS | 平行光源。`DirectionalLight`（`float4 color`、`float3 direction`、`float intensity`） |
| `b2` | PS | カメラ。`float3 worldPosition` と `float` のパディング |
| `t0` から `t3` | PS | テクスチャスロット。`TextureSlotDesc::registerIndex` と一致させる |
| `t0` | VS | インスタンシング用の `StructuredBuffer<ParticleForGPU>`。`requiresInstancing` のシェーダーのみ |
| `s0` | PS | サンプラー |

頂点入力は `VertexLayoutType` に合わせます。`Mesh` は `POSITION0`（`float4`）、`TEXCOORD0`（`float2`）、`NORMAL0`（`float3`）です。

HLSL ファイルには日本語を書きません。HLSL は起動時に DXC でコンパイルされ、CI では検査されません。コンパイルエラーがあると Debug 構成では `assert` で止まります。

## 2. 定義を登録する

ゲーム側は `project/Game/GameModule.cpp` の `Game::RegisterGameShaders` に書きます。`Object3D` の登録が例です。

```cpp
Cake::ShaderDefinition object3d;
object3d.name = "Object3D";
object3d.vsPath = L"Assets/Shaders/Object3D/Object3d.VS.hlsl";
object3d.psPath = L"Assets/Shaders/Object3D/Object3d.PS.hlsl";
object3d.params = {
	Cake::ShaderParamDesc{
		.name = "color",
		.type = Cake::ShaderParamType::Color,
		.defaultValue = {1, 1, 1, 1},
		.uiMin = 0.0f,
		.uiMax = 1.0f
	},
	// ...
};
object3d.textures = {
	Cake::TextureSlotDesc{
		.name = "albedo",
		.registerIndex = 0
	}, // t0.
};
shaderLibrary.Register(object3d);
```

| 項目 | 内容 |
| --- | --- |
| `name` | シェーダー名。PSO の名前とマテリアルの選択肢になる。`Unlit/Color` のようにスラッシュで階層を付けられる |
| `vsPath`、`psPath` | `project/` からの相対パス（ワイド文字列） |
| `params` | cbuffer `b0` に並ぶパラメータ。`name` は HLSL の変数名と一致させる。型は `Float`、`Float2`、`Float3`、`Float4`、`Color`、`Int` |
| `textures` | テクスチャスロット。`name` はマテリアル側で指定する名前、`registerIndex` は `t` のレジスタ番号（0 から 3） |
| `blendMode`、`depthTest`、`depthWrite`、`cullMode`、`fillMode` | PSO の設定。既定はブレンド無し、深度テストと書き込みあり、背面カリング、塗りつぶし |
| `vertexLayout`、`topologyType` | 頂点入力とトポロジ。既定は `Mesh` と三角形 |
| `requiresInstancing` | true にすると `Renderer::DrawModelInstanced` 専用になる。通常の `DrawModel` ではエラーマテリアルに差し替えられる |
| `hiddenInEditor` | true にするとマテリアルの選択肢に出さない（`Debug/Gizmo` など） |

`params` のオフセットは HLSL のパッキング規則で自動計算され、PS のリフレクションが取れれば実際の値で上書きされます。それでも HLSL 側の構造体と `params` の並びは合わせておきます。

エンジンの既定シェーダーを足す場合は `project/Engine/Graphics/Shader/ShaderLibrary.cpp` の `RegisterDefaultShaders` に同じ形で書きます。

## 3. マテリアルを作る

Debug 構成で起動し、Material List ウィンドウの作成ボタンから新しい `.mat` を作ります。作った `.mat` は `Assets/` 以下に保存され、`AssetDatabase` の索引に登録されます。Inspector でシェーダーを切り替えると、cbuffer が作り直され、`params` の既定値で初期化されます。

`.mat` は JSON で、シェーダー名、パラメータの値、テクスチャスロットごとのテクスチャを持ちます。

```json
{
  "materials": {
    "PhongReflectionMaterial": {
      "params": { "color": [1.0, 1.0, 1.0, 1.0], "shininess": [1.0] },
      "shader": "Object3D",
      "textures": {}
    }
  }
}
```

編集は Inspector で行い、Ctrl+S で書き戻します。モデルに埋め込まれたマテリアル（`.mtl` 由来）は読み取り専用なので、編集したいときは Create Material Asset で `.mat` に切り出します。

## 守ること

- 登録は `Asset` の初期化より前に行う。`Application::Initialize` が `Game::RegisterGameShaders` をその位置で呼んでいる。
- HLSL 側の cbuffer と C++ 側の `params` の並びを一致させる。`b1` と `b2` の構造体はエンジン側で固定されている。
- `b2` を読むシェーダーは `float3 worldPosition` の並びに合わせる。
- 既存の `name` を変えると、その名前を保存している `.mat` が読めなくなる。
