#pragma once
/*====================================
 *
 * シェーダー定義（ファイルパス、パラメータ記述、テクスチャスロット等）を一元管理し、
 * 登録されたシェーダーの各種情報提供やPSO（PipelineState）生成と紐づけを行うマネージャ。
 * マテリアルはこのShaderLibraryから定義を取得してシェーダーを設定する。
 *
 * ====================================*/
#include <string>
#include <unordered_map>
#include <vector>
#include "Engine/Graphics/Shader/ShaderDefinition.h"

struct ID3D12ShaderReflection;

namespace Cake {

class ShaderCompiler;
class PipelineState;

// シェーダー定義を一元管理し、PSO登録と紐づけるクラス.
class ShaderLibrary {
private:
	std::unordered_map<std::string, ShaderDefinition> definitions_;

	PipelineState* pipelineState_ = nullptr;
	ShaderCompiler* shaderCompiler_ = nullptr;

public:
	const ShaderDefinition* errorShader_ = nullptr;
	const ShaderDefinition* defaultShader_ = nullptr;

public:
	void Initialize(PipelineState* pipelineState, ShaderCompiler* shaderCompiler);

	// 定義を登録し、レイアウト計算とPSO生成まで一括で行う.
	void Register(ShaderDefinition definition);

	// 名前で定義を取得する(見つからなければnullptr).
	const ShaderDefinition* Find(const std::string& name) const;

	// 登録済みの全シェーダー名を返す(ImGuiのドロップダウン用).
	std::vector<std::string> GetAllNames() const;

private:
	// エンジンデフォルトのシェーダーを登録する.
	void RegisterDefaultShaders();
	/// <summary>
	/// HLSL の実レイアウトを読み取って、paramsに正しいバイト位置を焼き込む関数
	/// </summary>
	/// <param name="definition">シェーダーの定義</param>
	/// <param name="reflection">シェーダーのリフレクション</param>
	void ApplyReflection(ShaderDefinition& definition, ID3D12ShaderReflection* reflection);
};

} // namespace Cake
