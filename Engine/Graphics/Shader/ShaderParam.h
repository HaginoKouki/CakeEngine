#pragma once

#include <cstdint>
#include <string>

#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

// マテリアルパラメータの型.
enum class ShaderParamType {
	Float,
	Float2,
	Float3,
	Float4,
	Color,	// 中身はFloat4だが、ImGuiでColorEditを出すために区別する.
	Int,
};

// パラメータ1つ分のサイズ(バイト)を返す.
uint32_t GetShaderParamSize(ShaderParamType type);

// シェーダーが公開するcbufferパラメータ1つの定義(スキーマ).
struct ShaderParamDesc {
	std::string name;				// cbuffer内の変数名と一致させる.
	ShaderParamType type = ShaderParamType::Float;
	Cake::Vector4 defaultValue{};	// 初期値(型に応じて必要な成分だけ使う).
	float uiMin = 0.0f;				// ImGui用の下限.
	float uiMax = 1.0f;				// ImGui用の上限.
	uint32_t offset = 0;			// cbuffer先頭からのバイトオフセット(ComputeLayoutで自動計算).
};

// テクスチャスロット1つの定義.
struct TextureSlotDesc {
	std::string name;			// "albedo"など、マテリアル側で指定する名前.
	uint32_t registerIndex = 0;	// t0, t1, ... のレジスタ番号.
};

}	// namespace Cake
