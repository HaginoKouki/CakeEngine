# Platform 層

Foundation 層と OS に依存する、エンジンの基礎部品を置きます。時間、入力、音、ファイルダイアログを担当します。

## この層が担当すること

Platform 層は Windows の API をエンジン向けの形に包みます。ウィンドウの生成とメッセージループは Application 層の `WindowApp` が担当し、Platform 層は WndProc から転送された入力メッセージを受け取る側です。

## 配置

`project/Engine/Platform/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| 直下 | `Platform` | `Time`、`InputManager`、`SoundManager` を所有する層の入口。`Initialize(hInstance, hwnd)` と `BeginFrame` を持つ |
| `Time/` | `Time` | エンジン内で唯一、経過時間を測る場所。scaled と unscaled の 2 系統、`timeScale`、一時停止、フレーム数 |
| `Input/` | `InputManager` | キーボード、マウス、ゲームパッドを束ね、ゲーム用とエディタ用の問い合わせ口を分ける |
| `Input/` | `KeyBoard` | Win32 メッセージによるキーボード入力。添字はスキャンコード |
| `Input/` | `Mouse` | ボタン、ホイール、カーソル位置は WndProc から、相対移動量は Raw Input から取る |
| `Input/` | `GamePad` | XInput。最大 4 台。デッドゾーン処理と振動の時間管理 |
| `Input/` | `KeyCode`、`KeyState`、`MouseButton`、`PadButton` | 入力の列挙と、押下状態の共通表現 |
| `Sound/` | `SoundManager` | XAudio2 と MediaFoundation による読み込みと再生（`SoundLoad`、`SoundPlay`、`SoundUnload`） |
| `File/` | `OpenFileDialog`、`SaveFileDialog` | OS のファイル選択ダイアログ。エディタがファイルパスをユーザーに決めさせる唯一の手段 |

## 主要な仕組み

### 時間

`Time::Tick` はフレームの先頭で 1 回だけ呼ばれ、以降その フレーム中の処理はすべて同じ経過時間を見ます。呼び出しは `Platform::BeginFrame` が担当し、`Application::RunFrame` から呼ばれます。

経過時間には 2 系統があります。

| 系統 | 取得関数 | 用途 |
| --- | --- | --- |
| scaled | `GetDeltaTime`、`GetTotalTime` | ゲームロジック。`timeScale` が掛かり、Stop 中は 0 になる |
| unscaled | `GetUnscaledDeltaTime`、`GetUnscaledTotalTime` | エディタカメラや UI のアニメーション。Stop 中も進む |

実測の経過時間は `kMaxDeltaTime`（0.1 秒）で頭打ちにします。デバッガで止めた直後などに巨大な値が流れると、オブジェクトが飛び、当たり判定もすり抜けるためです。捨てた分は取り戻しません。シーン読み込みのように止まっていた時間を混ぜたくない場面では `Reset` を呼びます。

### 入力

入力は 2 段階で扱います。WndProc が非同期に書き込む「生の状態」と、`Update` で取り込むフレーム内で変わらない「スナップショット」です。ゲームループは常にスナップショットだけを見るため、フレームの途中で押下状態が変わることはありません。

`KeyState` は bit0 が今フレーム、bit1 が前フレームの押下状態を表し、`None`、`Pressed`（押した瞬間）、`Release`（離した瞬間）、`Held`（押し続けている）の 4 値になります。

問い合わせ口は 2 つあります。

| 関数 | 用途 | 動き |
| --- | --- | --- |
| `GetGameKeyState` | ゲームロジック | `SetGameInputEnabled(false)` の間は常に `None` を返す。エディタで Game ウィンドウが非フォーカスのときに使う |
| `GetRawKeyState` | エディタ専用 | 常に実際の状態を返す。`EditorCamera` が使う |

フォーカスを失ったときは各デバイスの `Clear` を呼び、押しっぱなしの張り付きを防ぎます。

### ファイルダイアログ

ダイアログは絶対パスを返しますが、そのまま `.scene` などへ保存すると別の PC で開けなくなります。そのため、実行ディレクトリからの相対パスへ変換し、区切りを `/` に揃えて返します。`OFN_NOCHANGEDIR` を必ず指定し、ユーザーが別フォルダを開いてもプロセスのカレントディレクトリが移動しないようにしています。キャンセル時は `false` を返し、出力引数を変更しません。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | 下は Foundation 層のみ。上位層（Graphics、Asset、Render、Scene、Editor）を知らない |
| 実装上の依存 | Foundation（`DebugLog`、`Vector`、`Convert`）と Platform 内部のみ。規則どおり |
| 外部 | Windows API、XInput、XAudio2、MediaFoundation |
| 使われ方 | `Application` が所有し、`SceneManager` が `UpdateContext` 経由でコンポーネントへ `InputManager`、`Time`、`SoundManager` を渡す |

## 守るべき規則

### 時間

- 経過時間は `Time` 1 箇所でしか測らない。`InputManager` や `PerformanceProfiler` も自前で測らず、`Time` から受け取る。
- `Time::Tick` はフレーム先頭で 1 回だけ呼ぶ。
- ゲームロジックは scaled、エディタカメラや UI は unscaled を使う。
- `deltaTime` は `kMaxDeltaTime` で頭打ちにし、捨てた分は取り戻さない。

### 入力

- ゲーム側は `GetGameKeyState`（または `UpdateContext` の `GetKey` 系）を使う。`GetRawKeyState` はエディタ専用。
- 入力デバイスはフォーカスを失ったとき `Clear` する。

### ファイルダイアログ

- 戻り値は必ず相対パスで `/` 区切り。`OFN_NOCHANGEDIR` を必ず指定する。
- Windows 専用で、移植時はこのファイルを差し替える。

## 未実装・要確認

- `SoundManager` は `SoundData` を生ポインタで返し、解放は呼び出し側が `SoundUnload` で行います。エンジン内で `SoundManager` を使っている箇所は、`UpdateContext` へ参照を載せている部分のみです。
