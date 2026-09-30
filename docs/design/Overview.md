# 全体方針
## 階層構造
各層は自分以下の層にしか依存しないという規則の元、階層を分けています。<br>
<br>
### [Foundation層](FoundationLayer.md)
*基礎の基礎*<br>
<br>
最も低層に位置する、自身とそれに関連するファイル以外への依存関係を持たず、OSに依存せずに動く層です。<br>
他の課題に移植したり、他人に配布しても問題なく動くレベルで軽い依存で作ってください。<br>
<br>
### [Platform層](PlatformLayer.md)
*エンジンの基礎*<br>
<br>
Foundation層やOSに依存する、エンジンの基礎に位置する層です。<br>
<br>
### [Asset層](AssetLayer.md)
*3Dモデルやテクスチャ、マテリアルなど*<br>
<br>
<br>
### [Graphics層](GraphicsLayer.md)
*DirectX12をまとめる*<br>
<br>
<br>
### [Render層](RenderLayer.md)
*画面を描画するための機能*<br>
<br>
<br>
### [Scene層](SceneLayer.md)
*ゲームの内容が詰まってる*<br>
<br>
<br>
### [Editor層](EditorLayer.md)
*ゲーム制作するための層*<br>
<br>
<br>
### [Application層](ApplicationLayer.md)
*アプリをアプリたらしめるところ*<br>
<br>
<br>
