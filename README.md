[![Debug build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/DebugBuild.yml)
[![Release build](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml/badge.svg)](https://github.com/HaginoKouki/CakeEngine/actions/workflows/ReleaseBuild.yml)

# CakeEngine

*It's a piece of cake!*

プロトタイプの作成からゲームの実行までを、もっと簡単にすることを目指して開発しているゲームエンジンです。C++ と DirectX 12 を中心に、学習と開発を進めています。

## プロジェクトについて

このリポジトリには、エンジン本体と設計ドキュメントを含みます。開発中のため、機能や設計は今後変更される場合があります。

## 設計ドキュメント

まず[全体設計](docs/design/Overview.md)で層の構成を確認してください。各層の責務と依存関係は、次のページで説明しています。

| 層 | 概要 |
| --- | --- |
| [Application](docs/design/ApplicationLayer.md) | エンジンを起動するアプリケーション |
| [Editor](docs/design/EditorLayer.md) | ゲーム制作を支援する編集機能 |
| [Scene](docs/design/SceneLayer.md) | ゲーム世界、オブジェクト、コンポーネント |
| [Render](docs/design/RenderLayer.md) | シーンを画面に描画する処理 |
| [Graphics](docs/design/GraphicsLayer.md) | DirectX 12 を扱う描画基盤 |
| [Asset](docs/design/AssetLayer.md) | モデル、テクスチャ、マテリアルなどの資産 |
| [Platform](docs/design/PlatformLayer.md) | OS や実行環境との接続 |
| [Foundation](docs/design/FoundationLayer.md) | OS に依存しない基礎機能 |

## 開発メモ

- [2026-10-04 設計レビュー](docs/review/2026-10-04.md)
- [2026-10-11 設計レビュー](docs/review/2026-10-11.md)
- [2026-10-16 設計レビュー](docs/review/2026-10-16.md)
