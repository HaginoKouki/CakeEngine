#include "UploadBudget.h"

namespace Cake {

size_t CountUploadsThisFrame(std::span<const uint64_t> sizes, uint64_t budget) {
	// sizes が空なら 0 を返す.
	if (sizes.empty()) {
		return 0;
	}

	size_t count = 0;   // 取った件数.
	uint64_t total = 0; // 取ったサイズの合計.

	for (auto size : sizes) {
		// 1件目は無条件に取る.
		if (count != 0) {
			// 2件目以降は、bugetを超えない範囲で取る.
			if (total + size > budget) {
				break;
			}
		}

		count++;
		total += size;
	}

	return count;
}

} // namespace Cake
