/*====================================
 *
 * Tests の入口。コンソールを UTF-8 にしてから doctest を実行する。
 * テストは各フォルダの *Tests.cpp に書き、ここには何も足さない。
 *
 * ====================================*/
#define DOCTEST_CONFIG_IMPLEMENT // doctest の中身だけを作り、main は自分で書く.
#include "externals/doctest/doctest.h"

#include <Windows.h>

int main(int argc, char** argv) {
	// テストの名前（UTF-8）を、コンソールが正しく表示できるようにする.
	SetConsoleOutputCP(CP_UTF8);

	doctest::Context context(argc, argv);
	return context.run(); // 失敗したテストがあれば 0 以外を返す（CI はこれで失敗を知る）.
}
