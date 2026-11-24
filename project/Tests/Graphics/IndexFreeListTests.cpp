#include "externals/doctest/doctest.h"

#include "Engine/Graphics/Descriptor/IndexFreeList.h"

using namespace Cake;

TEST_CASE("IndexFreeList: 番号は0から順に払い出される") {
	IndexFreeList indices;
	indices.Initialize(4);

	CHECK(indices.Allocate() == 0);
	CHECK(indices.Allocate() == 1);
	CHECK(indices.Allocate() == 2);
	CHECK(indices.Allocate() == 3);
}
TEST_CASE("IndexFreeList: 容量を使い切ると無効な番号を返す") {
	IndexFreeList indices;
	indices.Initialize(2);

	indices.Allocate();
	indices.Allocate();
	CHECK(indices.Allocate() == IndexFreeList::kInvalidIndex);
}
TEST_CASE("IndexFreeList: 返した番号は使いまわされる") {
	IndexFreeList indices;
	indices.Initialize(4);

	indices.Allocate();
	indices.Allocate();
	indices.Allocate();
	indices.Free(1);
	CHECK(indices.Allocate() == 1);
}
TEST_CASE("IndexFreeList: 二重解放はfalseを返す") {
	IndexFreeList indices;
	indices.Initialize(4);

	indices.Allocate();
	indices.Allocate();
	indices.Allocate();

	CHECK(indices.Free(1));
	CHECK_FALSE(indices.Free(1));
}
TEST_CASE("IndexFreeList: 払いだしていない番号の解放はfalseを返す") {
	IndexFreeList indices;
	indices.Initialize(4);

	CHECK_FALSE(indices.Free(0));
	CHECK_FALSE(indices.Free(4));
	CHECK_FALSE(indices.Free(IndexFreeList::kInvalidIndex));
}
TEST_CASE("IndexFreeList: 使用数と最大の使用数") {
	IndexFreeList indices;
	indices.Initialize(4);

	indices.Allocate();
	indices.Allocate();
	indices.Allocate();

	indices.Free(0);
	indices.Free(1);

	CHECK(indices.GetUsedCount() == 1);
	CHECK(indices.GetPeakUsedCount() == 3);
}
TEST_CASE("IndexFreeList: 作成と破棄を繰り返しても尽きない") {
	IndexFreeList indices;
	indices.Initialize(4);

	for (uint32_t i = 0; i < 10000; i++) {
		auto index = indices.Allocate();
		CHECK(index != IndexFreeList::kInvalidIndex);
		CHECK(indices.Free(index));
	}
}
