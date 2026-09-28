#include "SoundManager.h"

#include <cassert>
#include <vector>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"

namespace Cake {

SoundManager::SoundManager() {
	DebugLog::GetInstance().Log(
		LogLevel::Info,
		"SoundManager",
		"Initializing..."
	);

	// MediaFoundation初期化.
	HRESULT hr = MFStartup(MF_VERSION);
	AssertHRESULT(hr, "Media Foundationの初期化");

	// XAudioエンジンのインスタンスを生成.
	hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
	AssertHRESULT(hr, "XAudio2エンジンの生成");

	// マスターボイスを生成.
	hr = xAudio2_->CreateMasteringVoice(&masteringVoice_);
	AssertHRESULT(hr, "マスターボイスの生成");

	DebugLog::GetInstance().Log(
		LogLevel::Info,
		"SoundManager",
		"Initialized\n"
	);
}
SoundManager::~SoundManager() {
	// XAudio2解放.
	xAudio2_.Reset();

	// Media Foundation 終了
	MFShutdown();
}

SoundData SoundManager::SoundLoad(const std::string& filename) {
	// SourceReaderを生成する.
	Microsoft::WRL::ComPtr<IMFSourceReader> reader;
	HRESULT hr = MFCreateSourceReaderFromURL(ConvertString(filename).c_str(), nullptr, &reader);
	AssertHRESULT(hr, "SourceReaderの生成");

	// 出力フォーマットをPCMに指定する
	Microsoft::WRL::ComPtr<IMFMediaType> pcmType;
	hr = MFCreateMediaType(&pcmType);
	AssertHRESULT(hr, "MediaTypeの生成");

	pcmType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pcmType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);

	hr = reader->SetCurrentMediaType(
		MF_SOURCE_READER_FIRST_AUDIO_STREAM,
		nullptr,
		pcmType.Get()
	);
	AssertHRESULT(hr, "PCM出力フォーマットの設定");

	// 実際に出力される確定値を取得.
	Microsoft::WRL::ComPtr<IMFMediaType> outType;
	reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &outType);

	// outType から WAVEFORMATEX を取得する
	WAVEFORMATEX* pwfx = nullptr;
	UINT32 wfxSize = 0;
	hr = MFCreateWaveFormatExFromMFMediaType(outType.Get(), &pwfx, &wfxSize);
	AssertHRESULT(hr, "WaveFormatExの変換");

	// 中身をコピーしてから解放する
	WAVEFORMATEX wfex = *pwfx;
	CoTaskMemFree(pwfx);

	// PCMデータを格納するバッファ.
	std::vector<BYTE> pcmData;

	// ReadSampleをループして全サンプルを読み出す.
	while (true) {
		DWORD streamFlags = 0;
		Microsoft::WRL::ComPtr<IMFSample> sample;

		hr = reader->ReadSample(
			MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			nullptr,
			&streamFlags,
			nullptr,
			&sample
		);

		// 終端チェック.
		if (FAILED(hr))
			break;
		if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;
		if (!sample)
			continue;

		// IMFSample → IMFMediaBuffer に変換
		Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
		hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
		if (FAILED(hr))
			continue;

		// バッファからバイト列を取り出して pcmData に追加
		BYTE* audioData = nullptr;
		DWORD audioLength = 0;

		hr = mediaBuffer->Lock(&audioData, nullptr, &audioLength);
		if (SUCCEEDED(hr)) {
			pcmData.insert(pcmData.end(), audioData, audioData + audioLength);
			mediaBuffer->Unlock();
		}
	}

	// returnするための音声データ.
	SoundData soundData = {};
	soundData.wfex = wfex;
	soundData.pBuffer = new BYTE[pcmData.size()];
	std::memcpy(soundData.pBuffer, pcmData.data(), pcmData.size());
	soundData.bufferSize = static_cast<unsigned int>(pcmData.size());

	return soundData;
}
// 音声データ解放.
void SoundManager::SoundUnload(SoundData* soundData) {
	// バッファのメモリを解放.
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}
// 音声再生.
void SoundManager::SoundPlay(const SoundData& soundData) {
	HRESULT result;

	// 波形フォーマットを元にSourceVoiceの生成.
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2_->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	AssertHRESULT(result, "SourceVoiceの生成");

	// 再生する波形データの設定.
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = soundData.bufferSize; // 波形データのサイズ.
	buffer.pAudioData = soundData.pBuffer;    // 波形データへのポインタ.
	buffer.Flags = XAUDIO2_END_OF_STREAM;     // 最後のバッファであることを示すフラグ.

	// 波形データの再生.
	result = pSourceVoice->SubmitSourceBuffer(&buffer);
	result = pSourceVoice->Start();
}

} // namespace Cake
