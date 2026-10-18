# ドキュメントの地図

CakeEngine のドキュメントは 3 つの種類に分かれています。目的に合わせて読む場所を選んでください。

| 知りたいこと | 読む場所 |
| --- | --- |
| エンジン全体の構造と、起動から 1 フレームまでの流れ | [design/Overview.md](design/Overview.md) |
| 各層の中身、守るべき規則、未実装の部分 | `design/` の層ごとのページ（下記） |
| コードを書くときの約束（整形、命名、設計上の決まり） | [design/Conventions.md](design/Conventions.md) |
| ビルドして動かす手順、実行時に作られるファイル | [guides/BuildAndRun.md](guides/BuildAndRun.md) |
| コンポーネントやシェーダーの追加手順 | `guides/` の各ページ（下記） |
| これまでのレビューで見つかった問題と、その状態 | [review/README.md](review/README.md) |

## design/ 設計書

実装の構造と設計上の判断を、層ごとに説明します。各ページは「この層が担当すること」「構成」「主要な仕組み」「他の層との関係」「守るべき規則」「未実装・要確認」の順で書かれています。実装と説明が異なる場合は、実装に合わせて更新します。

| ページ | 内容 |
| --- | --- |
| [Overview.md](design/Overview.md) | 層の一覧、依存の規則と現状、起動の流れ、1 フレームの流れ、ビルド構成、共通の考え方 |
| [ApplicationLayer.md](design/ApplicationLayer.md) | `Application`、`WindowApp`、`ProjectSettings`、ゲームコードとの接点 |
| [EditorLayer.md](design/EditorLayer.md) | エディタの構成、1 フレームの処理、Play / Stop、設定ファイル、操作 |
| [SceneLayer.md](design/SceneLayer.md) | GameObject とコンポーネント、型の登録、更新順、衝突判定、パーティクル、シリアライズ |
| [RenderLayer.md](design/RenderLayer.md) | `Renderer` の関数、パス列、視点の作り方、ギズモ |
| [AssetLayer.md](design/AssetLayer.md) | ハンドル、`.meta` と索引、ロード窓口、マテリアルの出自、未接続のインポート系 |
| [GraphicsLayer.md](design/GraphicsLayer.md) | 初期化順、ルートシグネチャ、シェーダー定義と PSO、ディスクリプタの予約枠、使い捨てバッファ |
| [PlatformLayer.md](design/PlatformLayer.md) | 時間、入力、音、ファイルダイアログ |
| [FoundationLayer.md](design/FoundationLayer.md) | 識別子、リフレクション、ログ、Gradient、バイナリと JSON |
| [Conventions.md](design/Conventions.md) | 言語とツール、整形、命名、ファイル配置、ヘッダコメント、ビルドフラグ、設計上の約束 |

## guides/ 手順書

目的から逆引きできる手順をまとめます。

| ページ | 内容 |
| --- | --- |
| [BuildAndRun.md](guides/BuildAndRun.md) | 必要なもの、ビルド、生成物の場所、実行時の前提、CI、うまく動かないとき |
| [AddingComponents.md](guides/AddingComponents.md) | 構造体、リフレクション、登録、`Update`、依存宣言、新しい `PropertyType` |
| [AddingShaders.md](guides/AddingShaders.md) | HLSL、`ShaderDefinition` の登録、マテリアルの作成 |

## review/ レビュー記録

定期レビューの運用と記録です。

| ページ | 内容 |
| --- | --- |
| [README.md](review/README.md) | レビューの運用方法、未解決事項の一覧 |
| [ReviewPrompt.md](review/ReviewPrompt.md) | Claude に渡す定期レビューのプロンプト |
| `YYYY-MM-DD.md` | 各回のレビュー記録。変更点、ドキュメントの更新、指摘、要確認事項、所感、調査範囲 |

## 書き方の約束

ドキュメントを更新するときは、次の点を揃えます。

- 設計書には日付や変更履歴を書かない。履歴はレビュー記録と Git に任せる。
- 実装されている事実、設計上の意図、未確定の事項を混ぜない。未確定の事項は「未実装・要確認」の節に書く。
- クラス名、関数名、ファイル名は実装と一致させる。
- 一覧性が必要な情報は表にし、説明文は地の文で書く。
- 同じ内容を複数のページに書かず、参照リンクで済ませる。
