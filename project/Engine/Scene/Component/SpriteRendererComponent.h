#pragma once

#include "Engine/Foundation/Reflection/ReflectMacros.h"

namespace Cake {

struct SpriteRendererComponent {
	bool visible = true;
	int sortingOrder = 0;
};

CAKE_REFLECT(SpriteRendererComponent)
CAKE_PROPERTY(visible, "Visible")
CAKE_PROPERTY(sortingOrder, "Sorting Order")
CAKE_REFLECT_END()

} // namespace Cake
