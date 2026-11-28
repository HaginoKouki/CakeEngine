#pragma once
/*====================================
 *
 * 起動時のコマンドライン引数を読み、指定された引数があるかを答える.
 * Windows の API（GetCommandLineW、CommandLineToArgvW）を使うので Platform 層に置く.
 *
 * ====================================*/
#include <string>
#include <vector>

namespace Cake {

class CommandLineArgs {
private:
	std::vector<std::wstring> args_; // 引数（exe の名前は含めない）.

public:
	void Initialize(); // 起動時のコマンドラインを読んで、args_ に入れる.

	// name と同じ引数があれば true.
	//    例：Has(L"--d3d-debug").
	bool Has(std::wstring_view name) const;
};

} // namespace Cake
