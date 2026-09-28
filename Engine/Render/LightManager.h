#pragma once

#include <wrl.h>
#include <d3d12.h>

#include "Engine/Render/Light.h"

namespace Cake {

class LightManager {
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalResource_;
	DirectionalLight* directionalData_ = nullptr;

public:
	void Initialize(ID3D12Device* device);

	// 値を書き込むだけ（バインドは Renderer 側）
	void SetDirectional(const DirectionalLight& light) {
		*directionalData_ = light;
	}

	ID3D12Resource* GetDirectionalResource() const {
		return directionalResource_.Get();
	}
};

} // namespace Cake
