#include "CommandLine.h"

#include <algorithm>
#include <Windows.h>
#include <shellapi.h>

namespace Cake {
void CommandLineArgs::Initialize() {
	args_.clear();

	// 引数の数を書いてもらう変数を用意する.
	int argc = 0;

	// 引数の配列を受け取る.
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == nullptr) {
		return;
	}

	// argv[0] は exe の名前なので、argv[1] から入れる.
	for (int i = 1; i < argc; ++i) {
		args_.emplace_back(argv[i]);
	}

	// Windows が確保した配列を返す.
	LocalFree(argv);
}

bool CommandLineArgs::Has(std::wstring_view name) const {
	return std::find(args_.begin(), args_.end(), name) != args_.end();
}
} // namespace Cake
