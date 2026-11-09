/*====================================
 *
 * Foundation 層の識別子（Guid、LocalId、AssetId）のテスト。
 *
 * ====================================*/
#include "externals/doctest/doctest.h"

#include <string>
#include <unordered_set>

#include "Engine/Foundation/Identity/AssetId.h"
#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Identity/LocalId.h"

using namespace Cake;

TEST_CASE("Guid: 既定値は無効") {
	CHECK_FALSE(Guid{}.IsValid());
	CHECK(Guid::Invalid() == Guid{});
}

TEST_CASE("Guid: Generate は有効な値を返し、毎回違う") {
	const Guid a = Guid::Generate();
	const Guid b = Guid::Generate();
	CHECK(a.IsValid());
	CHECK(b.IsValid());
	CHECK(a != b);
}

TEST_CASE("Guid: ToString と TryParse で元に戻る") {
	const Guid guid{0x0123456789abcdefull, 0xfedcba9876543210ull};
	const std::string text = guid.ToString();
	CHECK(text == "0123456789abcdeffedcba9876543210");
	CHECK(text.size() == Guid::kStringLength);

	Guid parsed;
	REQUIRE(Guid::TryParse(text, parsed));
	CHECK(parsed == guid);
}

TEST_CASE("Guid: TryParse は不正な文字列を受け付けない") {
	Guid out;
	CHECK_FALSE(Guid::TryParse("", out));
	CHECK_FALSE(Guid::TryParse("0123456789abcdef", out));                  // 短い.
	CHECK_FALSE(Guid::TryParse("0123456789abcdeffedcba98765432100", out)); // 長い.
	CHECK_FALSE(Guid::TryParse("0123456789abcdeffedcba987654321g", out));  // 16進数でない文字.
	CHECK_FALSE(Guid::TryParse("0x23456789abcdeffedcba9876543210", out));  // 0x は付けない.
}

TEST_CASE("LocalId: 同じ種別と名前なら同じ値になる") {
	static_assert(MakeTaggedLocalId(1u, "Cube") == MakeTaggedLocalId(1u, "Cube"), "コンパイル時にも計算できること");
	CHECK(MakeTaggedLocalId(1u, "Cube") == MakeTaggedLocalId(1u, "Cube"));
}

TEST_CASE("LocalId: 種別が違えば、同じ名前でも違う値になる") {
	CHECK(MakeTaggedLocalId(1u, "Cube") != MakeTaggedLocalId(2u, "Cube"));
}

TEST_CASE("LocalId: 主オブジェクトの値（0）にはならない") {
	CHECK(MakeTaggedLocalId(1u, "") != kSelfLocalId);
	CHECK(MakeTaggedLocalId(1u, "Cube") != kSelfLocalId);
}

TEST_CASE("AssetId: 既定値は無効") {
	CHECK_FALSE(AssetId{}.IsValid());
	CHECK(AssetId::Invalid() == AssetId{});
}

TEST_CASE("AssetId: MainOf は主オブジェクトを指す") {
	const Guid guid{1, 2};
	const AssetId id = AssetId::MainOf(guid);
	CHECK(id.IsValid());
	CHECK(id.IsMainObject());
	CHECK(id.guid == guid);
}

TEST_CASE("AssetId: LocalId が違えば別のものとして扱う") {
	const Guid guid{1, 2};
	const AssetId main = AssetId::MainOf(guid);
	const AssetId sub{guid, MakeTaggedLocalId(1u, "Cube")};
	CHECK(main != sub);
	CHECK_FALSE(sub.IsMainObject());

	std::unordered_set<AssetId> set{main, sub};
	CHECK(set.size() == 2);
}

TEST_CASE("AssetId: ToString は GUID と LocalId をコロンでつなぐ") {
	const AssetId id{Guid{0x0123456789abcdefull, 0xfedcba9876543210ull}, 0x1ull};
	CHECK(id.ToString() == "0123456789abcdeffedcba9876543210:0000000000000001");
}
