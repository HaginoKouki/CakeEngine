#pragma once
/*====================================
 *
 * DXCコンパイラを用いてHLSLファイルをコンパイルし、シェーダーバイナリを生成するクラス。
 * シェーダーリフレクションにより、パラメータレイアウトやテクスチャスロット情報を抽出できる。
 * エンジン起動時および開発時のシェーダー再読み込み時に使用される。
 *
 * ====================================*/
#include <string>
#include <wrl.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include <d3d12shader.h>

namespace Cake {

class ShaderCompiler {
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

public:
	void Initialize();
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(
		const std::wstring& filePath,
		const wchar_t* profile,
		Microsoft::WRL::ComPtr<ID3D12ShaderReflection>* outReflection = nullptr
	);
};

} // namespace Cake
