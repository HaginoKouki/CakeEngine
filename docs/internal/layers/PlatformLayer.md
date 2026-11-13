# Platform 層

Foundation 層と OS に依存する、エンジンの基礎部品を置きます。時間、入力、音、ファイルダイアログ、ワーカースレッド（ジョブ）を担当します。

## この層が担当すること

Platform 層は Windows の API をエンジン向けの形に包みます。ウィンドウの生成とメッセージループは Application 層の `WindowApp` が担当し、Platform 層は WndProc から転送された入力メッセージを受け取る側です。

## 配置

`project/Engine/Platform/`

## 構成

| フォルダ | 主な型 | 役割 |
| --- | --- | --- |
| 直下 | `Platform` | `Time`、`InputManager`、`SoundManager`、`JobSystem` を所有する層の入口。`Initialize(hInstance, hwnd, initialTime)`、`BeginFrame`、`EndFrame` を持つ。初期化でタイマーの分解能を 1 ms に上げ、破棄で戻す |
| `Time/` | `Time` | エンジン内で唯一、経過時間を測る場所。scaled と unscaled の 2 系統、`timeScale`、一時停止、フレーム数、目標フレームレートまでの待機 |
| `Input/` | `InputManager` | キーボード、マウス、ゲームパッドを束ね、ゲーム用とエディタ用の問い合わせ口を分ける |
| `Input/` | `KeyBoard` | Win32 メッセージによるキーボード入力。添字はスキャンコード |
| `Input/` | `Mouse` | ボタン、ホイール、カーソル位置は WndProc から、相対移動量は Raw Input から取る。押下中はウィンドウ外でも離した通知が届くようキャプチャする |
| `Input/` | `GamePad` | XInput。最大 4 台。円形デッドゾーン処理と振動の時間管理。未接続スロットは 1 秒おきにだけ再検索する |
| `Input/` | `KeyCode`、`KeyState`、`MouseButton`、`PadButton` | 入力の列挙と、押下状態の共通表現 |
| `Sound/` | `SoundManager` | XAudio2 と MediaFoundation による読み込みと再生（`SoundLoad`、`SoundPlay`、`SoundUnload`） |
| `File/` | `OpenFileDialog`、`SaveFileDialog` | OS のファイル選択ダイアログ。エディタがファイルパスをユーザーに決めさせる唯一の手段 |
| `Job/` | `JobSystem`、`JobPriority`、`JobHandle`、`JobResult` | 固定数のワーカースレッドで作業を進め、完了処理をメインスレッドで実行する |

## 主要な仕組み

### 時間

`Time::Tick` はフレームの先頭で 1 回だけ呼ばれ、以降そのフレーム中の処理はすべて同じ経過時間を見ます。呼び出しは `Platform::BeginFrame` が担当し、`Application::RunFrame` から呼ばれます。

経過時間には 2 系統があります。

| 系統 | 取得関数 | 用途 |
| --- | --- | --- |
| scaled | `GetScaledDeltaTime`、`GetScaledTotalTime` | ゲームロジック。`timeScale` が掛かり、Stop 中は 0 になる |
| unscaled | `GetUnscaledDeltaTime`、`GetUnscaledTotalTime` | エディタカメラや UI のアニメーション。Stop 中も進む |

実測の経過時間は `kMaxDeltaTime`（0.1 秒）で頭打ちにします。デバッガで止めた直後などに巨大な値が流れると、オブジェクトが飛び、当たり判定もすり抜けるためです。捨てた分は取り戻しません。シーン読み込みのように止まっていた時間を混ぜたくない場面では `Reset` を呼びます。

一時停止と `timeScale` は `RequestSetPaused` と `RequestSetTimeScale` で変えます。どちらも値を控えるだけで、次の `Tick` から効きます。呼んだフレームの `GetScaledDeltaTime` は変わりません。

### フレームレートの固定

`Platform::EndFrame` が `Time::WaitForTargetFrame` を呼び、フレームの長さを目標フレームレート（`targetFrameRate_`、60）に合わせます。呼ぶのは Present の後で、`Application::RunFrame` の末尾です。

| 手順 | 内容 |
| --- | --- |
| 1 | フレーム開始（直前の `Tick` の時刻）からの経過が 1/65 秒以上なら、VSync がすでに刻んだとみなして待たない |
| 2 | 1/60 秒の期限の 2 ms 前まで `sleep_until` で休む |
| 3 | 残りは `yield` を繰り返して待ち、Sleep の寝過ごしで期限を過ぎないようにする |

Sleep の精度を上げるため、`Platform::Initialize` で `timeBeginPeriod(1)` を呼び、`~Platform` で `timeEndPeriod(1)` を呼びます。`timeBeginPeriod` に失敗した場合は警告ログを出して続行します。`Platform` はコピーを禁止していて、`timeEndPeriod` が二重に呼ばれないようにしています。

### 入力

入力は 2 段階で扱います。WndProc が非同期に書き込む「生の状態」と、`Update` で取り込むフレーム内で変わらない「スナップショット」です。ゲームループは常にスナップショットだけを見るため、フレームの途中で押下状態が変わることはありません。

`KeyCode` の値は PC のスキャンコード（Set 1）で、拡張キーは最上位ビットを立てた形です。仮想キーコードと違い物理的なキー位置を表すため、JIS と US で配列が変わっても WASD の位置関係が崩れません。

`KeyState` は bit0 が今フレーム、bit1 が前フレームの押下状態を表し、`None`、`Pressed`（押した瞬間）、`Release`（離した瞬間）、`Held`（押し続けている）の 4 値になります。

問い合わせ口は 2 つあります。

| 関数 | 用途 | 動き |
| --- | --- | --- |
| `GetGameKeyState` | ゲームロジック | `SetGameInputEnabled(false)` の間は常に `None` を返す。エディタで Game ウィンドウが非フォーカスのときに使う |
| `GetRawKeyState` | エディタ専用 | 常に実際の状態を返す。`EditorCamera` が使う |

フォーカスを失ったとき（`WM_KILLFOCUS`）は各デバイスの `Clear` を呼び、押しっぱなしの張り付きを防ぎます。

### ファイルダイアログ

ダイアログは絶対パスを返しますが、そのまま `.scene` などへ保存すると別の PC で開けなくなります。そのため、実行ディレクトリからの相対パスへ変換し、区切りを `/` に揃えて返します。`OFN_NOCHANGEDIR` を必ず指定し、ユーザーが別フォルダを開いてもプロセスのカレントディレクトリが移動しないようにしています。キャンセル時は `false` を返し、出力引数を変更しません。

### ジョブ

`JobSystem` は、重い作業をメインスレッドの外で進めるための仕組みです。作業は「ワーカーで動く処理」と「メインスレッドで動く完了処理」の組で `Submit` に渡し、`Submit` は作業を指す `JobHandle` を返します。ワーカーは作業を終えると完了処理を完了のキューに積むだけで、完了処理を実行するのは `PumpMainThread` と `WaitFor` です。アセットや GPU に触れるのは完了処理の中だけ、という決まりをこの形で守ります。

| 項目 | 内容 |
| --- | --- |
| ワーカーの本数 | `Initialize` の引数で指定する。0 なら論理コア数−1（最低 1）。`hardware_concurrency` が 0 を返したときは 1 として扱う |
| ワーカーの起動 | `CoInitializeEx`（MTA）で COM を初期化し、`SetThreadDescription` と `DebugLog::SetCurrentThreadName` で `Worker01` の形の名前を付ける。COM の初期化が Windows の API に依存するため、この仕組みは Foundation 層ではなく Platform 層に置いている |
| 優先度 | `High`、`Normal`、`Low` の 3 本のキューを持ち、ワーカーは `High` から取り出す。同じ優先度の中では積んだ順 |
| 結果の受け渡し | `Submit<T>` は作業の戻り値を `JobResult<T>` に入れて完了処理へ渡す。作業が例外を投げたときは `value` を空にし、`error` に理由を入れる。戻り値なしの `Submit` で積んだ作業の例外は、ワーカーが捕まえてエラーのログに出す |
| 完了処理の実行 | `PumpMainThread(budget)` を `Application::RunFrame` の先頭で毎フレーム呼ぶ。使った時間が `budget` 以上になったら残りは次のフレームに回すが、最低 1 件は実行する |
| 待ち合わせ | `WaitFor(handle)` は、その作業の完了処理が終わるまで戻らない。待つ間はメインスレッドもキューの作業を実行する。ほかの作業の完了処理は実行しない |
| 終了 | `Shutdown`（デストラクタからも呼ぶ）は、受付を止め、未着手の作業と未実行の完了処理を捨て、実行中の作業が終わるのを待って join する。`Shutdown` のあとの `Submit` は無効な `JobHandle` を返す |

作業のキュー、完了のキュー、受け付けた作業の番号の一覧は、1 つの `mutex` で守ります。ワーカーを起こす知らせ（作業が積まれた、止める）と、`WaitFor` を起こす知らせ（作業が 1 つ終わった）は、別々の `condition_variable` で送ります。

## 他の層との関係

| 項目 | 内容 |
| --- | --- |
| 規則 | 下は Foundation 層のみ。上位層（Graphics、Asset、Render、Scene、Editor）を知らない |
| 実装上の依存 | Foundation（`DebugLog`、`Vector`、`Convert`）と Platform 内部のみ。規則どおり |
| 外部 | Windows API、COM、XInput、XAudio2、MediaFoundation、comdlg32 |
| 使われ方 | `Application` が所有し、`SceneManager` が `UpdateContext` 経由でコンポーネントへ `InputManager`、`Time`、`SoundManager` を渡す。`JobSystem` は `Application::RunFrame` が `PumpMainThread` を呼ぶ（作業を投げる側はフェーズ 2 の AssetManager から加わる） |

## 守るべき規則

### 時間

- 経過時間は `Time` 1 箇所でしか測らない。`InputManager` や `PerformanceProfiler` も自前で測らず、`Time` から受け取る。
- `Time::Tick` はフレーム先頭で 1 回だけ呼ぶ。`Time::WaitForTargetFrame` はフレーム末尾（Present の後）で 1 回だけ呼ぶ。どちらも `Platform` の `BeginFrame` と `EndFrame` が担当する。
- ゲームロジックは scaled、エディタカメラや UI は unscaled を使う。
- `deltaTime` は `kMaxDeltaTime` で頭打ちにし、捨てた分は取り戻さない。
- 一時停止と `timeScale` の変更は次の `Tick` から効く。呼んだフレームの経過時間は変わらない。

### 入力

- ゲーム側は `GetGameKeyState`（または `UpdateContext` の `GetKey` 系）を使う。`GetRawKeyState` はエディタ専用。
- 入力デバイスはフォーカスを失ったとき `Clear` する。

### ファイルダイアログ

- 戻り値は必ず相対パスで `/` 区切り。`OFN_NOCHANGEDIR` を必ず指定する。
- Windows 専用で、移植時はこのファイルを差し替える。

### ジョブ

- ワーカーの処理では、アセットの格納基盤・GPU・ImGui・シーンに触らない。触るのは完了処理の中だけ。
- `Initialize`、`Shutdown`、`PumpMainThread`、`WaitFor` はメインスレッド専用。`PumpMainThread` と `WaitFor` は Debug ビルドで `assert` が確かめる。
- `WaitFor` はエディタの道具と起動時に必要なアセット用。ゲームのフレーム中には使わない。
- `JobSystem` は `DebugLog` より先に止める。`JobSystem` は `Platform` のメンバなので `Application` と一緒に破棄され、`DebugLog` は関数内 static なので `main` を抜けた後に消える。この順序は `Application` が `WinMain` のローカル変数であることで守られている。

## 未実装・要確認

- `SoundManager` は `SoundData` を生ポインタで返し、解放は呼び出し側が `SoundUnload` で行います。エンジン内で `SoundManager` を使っている箇所は、`UpdateContext` へ参照を載せている部分のみです。`SoundPlay` が作る `IXAudio2SourceVoice` は破棄されません。
- `SoundManager` はコンストラクタで MediaFoundation と XAudio2 を初期化します。ほかの部品のように `Initialize` を持たないため、`Platform` のメンバとして構築された時点で動き始めます。
- `Time::targetFrameRate_` は 60 で固定です。変える関数も `ProjectSettings` の項目も無く、`std::optional` の「未設定なら待たない」経路は外から選べません。
- `Time::initialTime_` は `Initialize` で受け取って保持していますが、使っている箇所はありません。
- `JobSystem::PumpMainThread` に渡す時間の上限は、`Application.cpp` の `kJobCompletionBudget`（2 ms）です。仮の値で、[docs/plans/AssetSystem.md](../../plans/AssetSystem.md) のフェーズ 1 の手順 9 で計測して決めます。
- `JobSystem` の完了処理が例外を投げた場合は捕まえません。`PumpMainThread` と `WaitFor` の外へ抜け、その作業の番号は `live_` に残ります。作業（ワーカー側）の例外だけを捕まえる設計です。
- `JobSystem::Initialize` は `Shutdown` を挟まずに 2 回呼ぶとワーカーが追加されます。呼び出し元は `Platform::Initialize` の 1 回だけです。
- Tests で `JobSystem::Initialize` を呼ぶと `DebugLog` が起動し、カレントディレクトリに `logs/<日付>/` が作られます。`.gitignore` が除外するのは `project/logs/` だけで、ほかの場所に作られた `logs/` は CI の `CheckUnwantedFiles` が検出します。
