#pragma once
/*====================================
 *
 * マテリアルへの安定参照を表す軽量ハンドル。
 * 生ポインタの代わりにこれを持ち回ることで、MaterialManager 内部の
 * 格納方法（連続配置 vector）が変わっても参照が壊れないようにする。
 *
 *   index      : MaterialManager 内部プール（vector）の添字。実体の場所。
 *   generation : そのスロットが「何代目か」。将来アンロードでスロットを
 *                再利用したとき、古いハンドルを検出するために使う。
 *                現状アンロードは無いので常に有効だが、フィールドは予約しておく。
 *
 * 空（未割り当て）ハンドルは IsValid() が false を返す。
 *
 * ====================================*/

#include <cstdint>

namespace Cake {

struct MaterialHandle {
	// kInvalidIndex を入れておくと「未割り当て」を表す.
	static constexpr uint32_t kInvalidIndex = 0xFFFFFFFFu;

	uint32_t index = kInvalidIndex; // プール内の添字（実体の場所）.
	uint32_t generation = 0;        // 世代番号（将来アンロード用。今は常に有効）.

	// 中身が入っているか（＝ CreateMaterial で払い出されたか）.
	bool IsValid() const { return index != kInvalidIndex; }

	// 同一マテリアルを指すか.
	bool operator==(const MaterialHandle& rhs) const {
		return index == rhs.index && generation == rhs.generation;
	}
	bool operator!=(const MaterialHandle& rhs) const { return !(*this == rhs); }
};

} // namespace Cake
