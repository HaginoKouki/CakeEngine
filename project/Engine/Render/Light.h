#pragma once

#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Math/Matrix.h"

namespace Cake {

struct DirectionalLight {
	Cake::Vector4 color = Cake::Vector4::One;
	Cake::Vector3 direction = Cake::Vector3::Zero;
	float intensity = 1.0f;
};

}	// namespace Cake
