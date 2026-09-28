#pragma once
/*====================================
 *
 * シーン内の GameObject を指す軽量な識別子。
 * 既存の各種ハンドル（ModelHandle 等）と同じ index + generation 方式。
 *
 *   index      : Scene 内プール（vector）の添字。実体の場所。
 *   generation : そのスロットが何代目か。破棄されたスロットが再利用されたとき、
 *                古い ID を検出するために使う。
 *
 * 生ポインタの代わりにこれを持ち回ることで、プールの再確保やオブジェクトの破棄で
 * 参照が壊れないようにする。
 *
 * 【注意】この ID はシーンファイル内でのみ一意。プロジェクト全体で一意な GUID とは別物。
 * ただし一度払い出した ID は保存後も変えないこと（Prefab の override が壊れるため）。
 *
 * ====================================*/
#include <compare>
#include <cstdint>

namespace Cake {

struct GameObjectId {
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	uint32_t index = kInvalidIndex;
	uint32_t generation = 0;

	bool IsValid() const { return index != kInvalidIndex; }

	auto operator<=>(const GameObjectId&) const = default;
	bool operator==(const GameObjectId&) const = default;
};

} // namespace Cake
