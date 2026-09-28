#include "ShaderDefinition.h"

namespace Cake {

uint32_t Cake::GetShaderParamSize(ShaderParamType type) {
	switch (type) {
		case ShaderParamType::Float:
			return 4;
		case ShaderParamType::Float2:
			return 8;
		case ShaderParamType::Float3:
			return 12;
		case ShaderParamType::Float4:
			return 16;
		case ShaderParamType::Color:
			return 16; // Float4と同じ.
		case ShaderParamType::Int:
			return 4;
	}
	return 4;
}

void ShaderDefinition::ComputeLayout() {
	uint32_t cursor = 0;

	for (auto& param : params) {
		uint32_t size = GetShaderParamSize(param.type);

		// HLSLのcbuffer規則: 1つの要素は16バイト境界をまたいではいけない.
		// 今の位置から置くと境界をまたぐ場合は、次の16バイト境界まで送る.
		uint32_t posInRegister = cursor % 16;
		if (posInRegister != 0 && posInRegister + size > 16) {
			cursor += 16 - posInRegister;
		}

		param.offset = cursor;
		cursor += size;
	}

	// cbuffer全体は16バイト単位に切り上げる.
	cbufferSize = (cursor + 15) & ~15u;
}

} // namespace Cake
