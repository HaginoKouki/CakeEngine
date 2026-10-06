# Platform層
*エンジンの基礎*



Foundation層とOSに依存する、エンジンの基礎部品を置きます。時間・入力・音・ファイルダイアログを担当します。



## 場所
`project/Engine/Platform/`



## 構成
- Platform（Platform.h/.cpp）… Time / InputManager / SoundManager を所有する層の入口。Initialize(hInstance, hwnd) と BeginFrame（時間を測り、入力を更新）。
- Time/ … Time。エンジン内で唯一の時間の権威。scaled（timeScale適用済み）と unscaled、Pause、Reset、frameCount。
- Input/ … InputManager（統括）、KeyBoard、Mouse（ボタン・ホイールはWndProc、相対移動量はRaw Input）、GamePad（XInput、最大4台）、KeyCode、KeyState。
- Sound/ … SoundManager（XAudio2 + MediaFoundation。SoundLoad / SoundPlay / SoundUnload）。
- File/ … FileDialog（OpenFileDialog / SaveFileDialog）。

## 依存関係
規則（Overview.md、Platform.hのコメント）:
- 下はFoundation層のみ。上位層（Graphics / Asset / Render / Scene / Editor）を一切知らない。

現状の実装:
- 集計した限り、includeはFoundation（DebugLog、Vector、Convert）とPlatform内部のみ。規則どおり。
- 外部: Windows API、XInput、XAudio2、MediaFoundation。

## 規則

### 時間

- 経過時間は Time 1箇所でしか測らない。InputManager や PerformanceProfiler も自前で測らず、Time から受け取る。
- Time::Tick はフレーム先頭で1回だけ呼ぶ（Application::RunFrame の platform_.BeginFrame が担当）。
- ゲームロジックは scaled、エディタカメラやUIアニメーションは unscaled を使う。Stop中は timeScale が 0 になるため。
- deltaTime は kMaxDeltaTime（0.1秒）で頭打ちにし、捨てた分は取り戻さない。シーン読み込みなど止まっていた時間を混ぜたくない場面では Time::Reset を呼ぶ。

### 入力

- 入力はWndProcが非同期に書く「生の状態」と、Updateで取り込むフレーム内不変の「スナップショット」を分ける。
- ゲーム側は GetGameKeyState を使う（ゲーム入力が無効のとき None を返す）。GetRawKeyState はエディタ専用。
- 入力デバイスはフォーカスを失ったとき Clear して押しっぱなしの張り付きを防ぐ。

### ファイルダイアログ

- FileDialog は必ず実行ディレクトリからの相対パスを '/' 区切りで返し、OFN_NOCHANGEDIR を必ず指定してカレントディレクトリを変えない。キャンセル時は false で out を変更しない。Windows専用で、移植時は差し替える。
