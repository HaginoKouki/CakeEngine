#include "ShaderCompiler.h"
#include <cassert>
#include <format>
#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Foundation/Utility/Convert.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "ShaderCompiler";
}

void ShaderCompiler::Initialize() {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	HRESULT hr;

	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	AssertHRESULT(hr, "DXCompilerのユーティリティ生成");

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	AssertHRESULT(hr, "DXCompilerのコンパイラ生成");

	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	AssertHRESULT(hr, "DXCompilerのIncludeハンドラ生成");

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(
	const std::wstring& filePath,
	const wchar_t* profile,
	Microsoft::WRL::ComPtr<ID3D12ShaderReflection>* outReflection
) {
	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, ConvertString(std::format(L"Begin CompileShader, path:{}, profile:{}", filePath, profile)));

	// 1. hlslファイルを読む.
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	AssertHRESULT(hr, "HLSLファイルの読み込み");

	DxcBuffer shaderSourceBuffer{};
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	// 2. コンパイル.
	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",
		L"main",
		L"-T",
		profile,
		L"-Zi",
		L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
	hr = dxcCompiler_->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler_.Get(),
		IID_PPV_ARGS(&shaderResult)
	);
	AssertHRESULT(hr, "Shaderのコンパイル");

	// 3. エラーチェック.
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		DebugLog::GetInstance().Log(
			LogLevel::Error,
			kLogCategory,
			shaderError->GetStringPointer()
		);
		assert(false);
	}

	// 4. バイナリを取得して返す.
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	AssertHRESULT(hr, "Shaderのバイナリ取得");

	// 5. リフレクション（要求された場合のみ）.
	if (outReflection != nullptr) {
		Microsoft::WRL::ComPtr<IDxcBlob> reflectionData;
		shaderResult->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionData), nullptr);
		if (reflectionData != nullptr) {
			DxcBuffer reflectionBuffer{};
			reflectionBuffer.Ptr = reflectionData->GetBufferPointer();
			reflectionBuffer.Size = reflectionData->GetBufferSize();
			reflectionBuffer.Encoding = DXC_CP_ACP;
			dxcUtils_->CreateReflection(
				&reflectionBuffer,
				IID_PPV_ARGS(outReflection->ReleaseAndGetAddressOf())
			);
		}
	}

	DebugLog::GetInstance().Log(LogLevel::Info, kLogCategory, ConvertString(std::format(L"Compile succeeded, path:{}, profile:{}\n", filePath, profile)));
	return shaderBlob;
}

} // namespace Cake
