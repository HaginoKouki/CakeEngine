#include "SpriteDrawList.h"

#include <algorithm>

namespace Cake {

void SpriteDrawList::Add(const SpriteDrawItem& item, int sortingOrder) {
	entries_.push_back({sortingOrder, item});
}
void SpriteDrawList::SortByOrder() {
	std::stable_sort(entries_.begin(), entries_.end(), [](const Entry& a, const Entry& b) {
		return a.sortingOrder < b.sortingOrder;
	});
}

} // namespace Cake
