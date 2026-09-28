#include "ComponentUpdateSystem.h"

#include "Engine/Scene/Component/ComponentRegistry/TypeRegistry.h"

#include "Engine/Scene/Scene.h"

namespace Cake {

void RunComponentUpdates(const UpdateContext& ctx) {
	// updateOrder_ は優先度順（同値なら登録順）にソート済み.
	for (const ComponentTypeInfo* info : TypeRegistry::GetInstance().GetUpdateOrder()) {
		info->ops.update(ctx);
	}
}

} // namespace Cake
