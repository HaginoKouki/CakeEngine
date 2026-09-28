#pragma once
/*====================================
 *
 * XAudio2とMediaFoundationを用いた音声の再生管理を行うマネージャ。
 * WAVファイル等を読み込んでSoundData（波形フォーマット＋PCMバッファ）を生成し、
 * マスターボイス経由で再生・停止制御を提供する。
 *
 * ====================================*/
#include <string>

#include <wrl.h> // Microsoft::WRL::ComPtrを使うためのヘッダ.

// XAudio2を使うためのヘッダ.
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

// MediaFoundationを使うためのヘッダ
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

namespace Cake {

// 音声データ.
struct SoundData {
	// 波形フォーマット.
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス.
	BYTE* pBuffer;
	// バッファのサイズ.
	unsigned int bufferSize;
};

class SoundManager {
private:
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masteringVoice_ = nullptr;

public:
	SoundManager();
	~SoundManager();

	SoundData SoundLoad(const std::string& filename);

	void SoundUnload(SoundData* soundData);
	void SoundPlay(const SoundData& soundData);

private:
};

} // namespace Cake
