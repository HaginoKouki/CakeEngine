# コーディング規約と設計上の約束

CakeEngine のコードを読むとき、書くときに前提になる約束をまとめます。整形ルールは `project/.clang-format` と `project/.editorconfig` が正で、ここではその要点と、ツールでは検査できない設計上の約束を説明します。

## 言語とツール

| 項目 | 内容 |
| --- | --- |
| 言語 | C++20（`stdcpp20`）。`concept`、指示付き初期化、`std::span`、`std::format` を使う |
| コンパイラ | Visual Studio 2026 の MSVC（プラットフォームツールセット v145） |
| 名前空間 | エンジンはすべて `Cake`。ゲーム固有のコンポーネントはグローバル名前空間、`GameLayer` のような補助は専用の名前空間 |
| 文字コード | UTF-8。コメントは日本語で書く。HLSL ファイルには日本語を書かない（先頭に英語で注記がある） |
| 警告 | Debug 構成では警告をエラー扱いにする。外部ライブラリの警告は抑制する |
| スペルチェック | `.editorconfig` で識別子とコメントの英単語を検査する。固有の単語は `project/exclusion.dic` に追加する |

## 整形

`.clang-format` は LLVM を土台に次の点を上書きしています。

| 項目 | 規則 |
| --- | --- |
| インデント | タブ（表示幅 4）。行内の桁揃えだけスペース |
| 波括弧 | K&R。開き波括弧は宣言と同じ行、`} else {` の形 |
| 1 行の長さ | 上限なし（`ColumnLimit: 0`）。`.editorconfig` の `max_line_length` は 120 |
| ポインタと参照 | 型側に寄せる（`int* p`、`int& r`） |
| include | 自動で並べ替えない。書いた順を保つ |
| 短い関数 | クラス内の短いインライン関数は 1 行にまとめてよい。`if` や ループは必ず改行する |
| アクセス指定子 | クラス本体より 1 段左（`AccessModifierOffset: -4`） |

## 命名

| 対象 | 規則 | 例 |
| --- | --- | --- |
| 型、関数 | パスカルケース | `SceneManager`、`UpdateTransforms` |
| メンバ変数 | 末尾にアンダースコア | `scene_`、`currentPath_` |
| ローカル変数、引数、公開メンバ | キャメルケース | `deltaTime`、`startColor` |
| 定数 | `k` 接頭辞 | `kMaxDeltaTime`、`kSrvReservedCount` |
| 列挙子 | パスカルケース。`BlendMode` のように `k` 接頭辞のものもある | `KeyState::Pressed`、`BlendMode::kBlendModeNormal` |
| シェーダー名 | スラッシュ区切りの階層 | `Unlit/Color`、`Skybox/Panoramic` |
| HLSL のリソース | `g` 接頭辞 | `gMaterial`、`gTexture` |
| ファイル | 型名と同じ。1 ファイル 1 型を基本とする | `Renderer.h` / `Renderer.cpp` |

## ファイルの配置

| 置き場所 | 内容 |
| --- | --- |
| `project/Engine/<層>/<機能>/` | 層ごとにフォルダを分け、その下を機能で分ける。層の入口（`Platform.h`、`Graphics.h`、`Asset.h`）は層フォルダの直下 |
| `project/Game/` | ゲーム固有のコード。Engine からは `GameModule.h` だけを参照する |
| `project/EngineResources/` | エンジンが使うシェーダー、フォント、テクスチャ。`AssetDatabase` の走査対象外 |
| `project/Assets/` | ゲームのアセット。`AssetDatabase` が走査し、`.meta` を発行する |
| `project/externals/` | 外部ライブラリ。整形とスペルチェックの対象外 |

include は `project/` を基準にした絶対パスで書きます（`#include "Engine/Scene/Scene.h"`）。同じフォルダのヘッダも同じ形で書き、`.cpp` が対応する `.h` を include するときだけ `#include "Renderer.h"` のように書きます。

## ヘッダコメントの様式

主要なヘッダは、先頭に次の形の説明を置いています。初めて読む人がファイルの目的と注意点を最初に読めるようにするためです。

```cpp
#pragma once
/*====================================
 *
 * このファイルが何をするかの 1 から 3 行の説明。
 *
 * 【見出し】
 * 設計上の判断と、その理由。「なぜこうなっているか」を書く。
 *
 * ====================================*/
```

関数やメンバのコメントは `// 説明.` のように末尾にピリオドを付けます。

## ビルド構成フラグの使い分け

`BuildConfig.h` が 2 つのフラグを定義しています。

| フラグ | 意味 | 使う場所 |
| --- | --- | --- |
| `USE_IMGUI` | ImGui ライブラリを使うか（include、リンク、API 呼び出しの可否） | 純粋に ImGui API を叩くだけの箇所 |
| `ENABLE_EDITOR` | エディタ機能を有効にするか | `Editor` クラス、ビューポート、インスペクタなど、エディタ機能そのもの |

`ENABLE_EDITOR` を定義して `USE_IMGUI` を定義しない構成は `#error` になります。

## 設計上の約束

層をまたいで守られている約束です。各層のページにも、その層に固有の規則があります。

### 依存の向き

- 上の層から下の層へだけ依存する。層の入口クラスのヘッダコメントに、知ってよい層と知ってはいけない層を書く。
- 下の層が上の層の型を必要とするときは、前方宣言と引数渡しで受け取る。`RenderContext` や `UpdateContext` のように「そのフレームに使ってよいもの」を束ねた構造体で渡す。
- 層の初期化に必要な依存は Desc 構造体（`AssetInitializeDesc`、`RendererInitDesc`）で渡し、`IsValid` で検査する。

### 所有と参照

- 実体は 1 箇所（Manager、`Scene`）が所有し、ほかはハンドル（index と generation）か `GameObjectId` で参照する。生ポインタは「次の追加まで有効」で、保持しない。
- 非所有のポインタはコメントに「非所有」と書く。
- `Application` のメンバ宣言順が初期化順で、破棄は逆順。順序に意味がある場合はコメントに理由を書く。

### 保存形式

- ファイルに書くのは、実行をまたいでも変わらない値だけ。GUID、LocalId、型名、プロパティ名。
- ハンドル、型 ID、ワールド行列、ダーティフラグは保存しない。
- 保存形式を変えたら版数（`kVersion`、`kSceneVersion`、`kCurrentMetaVersion`）を上げる。
- 設定ファイルは「チームで共有する値」（`ProjectSettings`）と「個人の値」（`EditorPreferences`、`EditorCache`）を分け、後者は `UserSettings/` に置いてコミットしない。

### 失敗時の振る舞い

- 読み込み失敗でエンジンを落とさない。エラー用のテクスチャやマテリアルを返し、欠損が見える状態で動き続ける。
- 設定ファイルが壊れていても既定値で起動する。ログを出して `false` を返す。
- 初期化時の抜けは `assert` と `IsValid` で早く検出する。

### コンポーネントと System

- コンポーネントは仮想関数を持たないデータだけの構造体にする。処理は System が担当する。
- 構造体の直後に `CAKE_REFLECT` ブロックを書く。メンバを増やしたら `CAKE_PROPERTY` も足す。
- `Update` を持つ型だけが自動更新される。継承やインターフェースは使わない。
- 可変長のデータはコンポーネントに持たせず、`ParticleStorage` のように `Scene` 側に置く。

### GPU との契約

- HLSL 側の構造体と C++ 側の構造体は 1 バイトも違ってはいけない。`static_assert` でサイズを固定する。
- ルートシグネチャは 1 つを全シェーダーで共有する。番号の割り当ては [GraphicsLayer.md](GraphicsLayer.md) を参照。
- 毎フレーム GPU を待ち切る同期設計を前提にしている。これを変えるときは `FrameConstantBuffer`、`FrameVertexBuffer`、`PerformanceProfiler` を作り直す。
