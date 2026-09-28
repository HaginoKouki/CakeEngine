#include "Guid.h"

#include <charconv>
#include <cstring>
#include <format>

#include <objbase.h> // CoCreateGuid.

#pragma comment(lib, "ole32.lib")

namespace Cake {

std::string Guid::ToString() const {
	// 上位64ビット・下位64ビットをそれぞれ16桁ゼロ埋めで並べる.
	// エンジン内部で完結する表現なので、UUID標準のバイト順とは一致しない（TryParseと対で使う）.
	return std::format("{:016x}{:016x}", high, low);
}

Guid Guid::Generate() {
	GUID native{};
	// CoCreateGuid は内部で暗号論的乱数を使い、UUID v4 相当の値を返す.
	// COM系APIだが CoInitialize は不要（この関数だけは例外的にCOMの状態に依存しない）.
	if (FAILED(CoCreateGuid(&native))) {
		return Guid::Invalid();
	}

	// 16バイトの生バイト列をそのまま uint64 2本へ詰め替える.
	// GUID構造体はメンバごとにエンディアンが異なるため、フィールド単位では扱わない.
	Guid result;
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&native);
	std::memcpy(&result.high, bytes, sizeof(uint64_t));
	std::memcpy(&result.low, bytes + sizeof(uint64_t), sizeof(uint64_t));
	return result;
}

bool Guid::TryParse(std::string_view text, Guid& out) {
	if (text.size() != kStringLength) {
		return false;
	}

	const char* begin = text.data();
	const char* middle = begin + kStringLength / 2;
	const char* end = begin + kStringLength;

	uint64_t high = 0;
	uint64_t low = 0;

	// from_chars は例外を投げず、16進数のプレフィックス（0x）や符号も受け付けない.
	// ptr が終端に達したかも見て、途中に不正文字が混ざった場合を弾く.
	const std::from_chars_result highResult = std::from_chars(begin, middle, high, 16);
	if (highResult.ec != std::errc{} || highResult.ptr != middle) {
		return false;
	}
	const std::from_chars_result lowResult = std::from_chars(middle, end, low, 16);
	if (lowResult.ec != std::errc{} || lowResult.ptr != end) {
		return false;
	}

	out.high = high;
	out.low = low;
	return true;
}

} // namespace Cake
