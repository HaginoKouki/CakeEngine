#pragma once
/*====================================
 *
 * 「当たった相手を減らす側」であることを示すだけの印。
 *
 * 弾にも近接判定にも同じものを付ける。受け側は種類を問わず
 * これを探すだけで済むので、攻撃手段が増えても受け側を触らずに済む。
 *
 * ====================================*/
#include "Engine/Reflection/ReflectMacros.h"

struct DamageDealerComponent {
	int damage = 1;
};

namespace Cake {
CAKE_REFLECT(DamageDealerComponent)
CAKE_PROPERTY(damage, "Damage")
CAKE_REFLECT_END()
} // namespace Cake
