#include "externals/doctest/doctest.h"

#include "Engine/Graphics/Upload/UploadBudget.h"

using namespace Cake;

TEST_CASE("UploadBudget: 空なら0件") {
	std::span<const uint64_t> sizes{};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes, budget) == 0);
}
TEST_CASE("UploadBudget: 上限に収まれば全部取る") {
	const uint64_t sizes_arr[] = {10, 20, 30};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 3);
}
TEST_CASE("UploadBudget: 上限を超える手前で取る") {
	const uint64_t sizes_arr[] = {40, 40, 40};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 2);
}
TEST_CASE("UploadBudget: 合計がちょうど上限なら取る") {
	const uint64_t sizes_arr[] = {50, 50};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 2);
}
TEST_CASE("UploadBudget: 1件目が上限より大きくても1件は取る") {
	const uint64_t sizes_arr[] = {200, 10};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 1);
}
TEST_CASE("UploadBudget: 途中に大きいものがあると、その手前で止まる") {
	const uint64_t sizes_arr[] = {10, 200, 10};
	uint64_t budget = 100;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 1);
}
TEST_CASE("UploadBudget: 上限が0でも1件は取る") {
	const uint64_t sizes_arr[] = {10, 10};
	uint64_t budget = 0;
	CHECK(CountUploadsThisFrame(sizes_arr, budget) == 1);
}
