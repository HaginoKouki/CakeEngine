#pragma once
/*====================================
 *
 * シェーダーパラメータ（色、浮動小数点値等）とテクスチャスロットを管理する構造体。
 * マテリアルは特定のシェーダー定義を参照し、その定義に従ってパラメータを保持する。
 * CPU側のパラメータ値とGPU側の定数バッファを同期する仕組みを提供する。
 *
 * 【出自（MaterialOrigin）】
 * 「このマテリアルがどこから来たか」を持つ。これで編集可否と保存先が決まる。
 *   Runtime  … コードが作った一時マテリアル。保存先を持たない.
 *   Asset    … .mat ファイル由来。assetPath_ へ上書き保存できる.
 *   Embedded … モデル(.mtl)に埋め込まれた既定マテリアル。読み取り専用.
 * Embedded を編集不可にしているのは、モデルの既定を書き換えると
 * そのモデルを使う全オブジェクトへ黙って波及し、しかも .obj/.mtl 側へは
 * 書き戻せないため（保存先が無い変更は次回起動で消える）。
 *
 * 【dirty】
 * 未保存の編集があるかを示す。Ctrl+S（MaterialManager::SaveDirtyMaterials）で
 * まとめて書き戻し、クリアされる。Embedded は編集できないので常に false。
 *
 * ====================================*/

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include <wrl.h>
#include <d3d12.h>

#include "Engine/Graphics/Shader/ShaderDefinition.h"
#include "Engine/Asset/Texture/TextureManager.h"

#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

// マテリアルの出自。編集可否と保存先の決定に使う.
enum class MaterialOrigin : uint8_t {
	Runtime,  // コードが作った一時マテリアル。保存先を持たない.
	Asset,    // .mat ファイル由来。assetPath_ へ上書き保存できる.
	Embedded, // モデル(.mtl)に埋め込まれた既定マテリアル。読み取り専用.
};

class Material {
private:
	std::string name_{};

	const ShaderDefinition* shader_ = nullptr; // どのシェーダーを使うか（非所有）.

	std::vector<uint8_t> cpuParams_;                // cbufferの中身（CPU側の生バイト列）.
	std::map<std::string, TextureHandle> textures_; // スロット名 → テクスチャ.

	Microsoft::WRL::ComPtr<ID3D12Resource> cb_ = nullptr; // GPU側のCB.
	void* mappedCB_ = nullptr;                            // Map済みポインタ.

	MaterialOrigin origin_ = MaterialOrigin::Runtime;
	std::string assetPath_; // origin_ == Asset のときの .mat パス（スラッシュ区切り）.
	bool dirty_ = false;    // 未保存の編集があるか.

public:
	Material() = default;
	Material(const Material&) = delete;            // コピー禁止.
	Material& operator=(const Material&) = delete; // コピー禁止.
	Material(Material&&) = default;                // ムーブOK.
	Material& operator=(Material&&) = default;     // ムーブOK.

	// シェーダーを割り当て、CPUバッファとGPU CBを確保し、デフォルト値で初期化する.
	void Initialize(ID3D12Device* device, const ShaderDefinition* shader);

	// 使用シェーダーを切り替える（cbufferを作り直す。テクスチャは保持）.
	void SetShader(ID3D12Device* device, const ShaderDefinition* shader);

	/* --- パラメータ書き込み（cpuParams_ に書く。GPUへの反映は Apply）---*/
	void SetFloat(const std::string& name, float v);
	void SetFloat2(const std::string& name, const Cake::Vector2& v);
	void SetFloat3(const std::string& name, const Cake::Vector3& v);
	void SetFloat4(const std::string& name, const Cake::Vector4& v);
	void SetColor(const std::string& name, const Cake::Vector4& v); // 中身はFloat4と同じ.
	void SetInt(const std::string& name, int32_t v);

	/* --- テクスチャ ---*/
	void SetTexture(const std::string& slot, const TextureHandle& handle);
	TextureHandle GetTexture(const std::string& slot) const;

	// cpuParams_ を GPU CB にコピーする（初期化時・パラメータ変更時に呼ぶ）.
	void Apply();

	// ImGui用：name のパラメータの先頭ポインタを返す（無ければ nullptr）.
	void* GetParamPtr(const std::string& name);

	/* --- mtl 読み込み（既存）---*/
	static std::vector<Material> LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename, TextureManager* textureManager);
	static Material& FindMaterialByName(std::vector<Material>& materials, const std::string& name);
	static const Material& FindMaterialByName(const std::vector<Material>& materials, const std::string& name);

	/* --- 出自・保存状態 ---　*/
	// インスペクタから触ってよいか.
	bool IsEditable() const { return origin_ != MaterialOrigin::Embedded; }
	// 上書き保存先を持っているか.
	bool HasAssetPath() const { return origin_ == MaterialOrigin::Asset && !assetPath_.empty(); }

	// 編集不可なマテリアルは dirty にならない（保存先が無いので溜めても無駄）.
	void MarkDirty() {
		if (IsEditable()) {
			dirty_ = true;
		}
	}
	void ClearDirty() { dirty_ = false; }
	bool IsDirty() const { return dirty_; }

	MaterialOrigin GetOrigin() const { return origin_; }
	const std::string& GetAssetPath() const { return assetPath_; }
	void SetOrigin(MaterialOrigin origin) { origin_ = origin; }
	void SetAssetPath(const std::string& path) { assetPath_ = path; }

	// 値（cbufferの中身とテクスチャ割り当て）だけを複製する。
	// 名前・出自・保存先は引き継がない（別アセットとして切り出すため）.
	// 同じシェーダーであることが前提。違う場合は先に SetShader しておくこと.
	void CopyValuesFrom(const Material& other);

	// Getter.
	const std::string& GetName() const { return name_; }
	const ShaderDefinition* GetShader() const { return shader_; }
	ID3D12Resource* GetCB() const { return cb_.Get(); }

	// Setter.
	void SetName(const std::string& newName) { name_ = newName; }

private:
	// name から ShaderParamDesc を引く（見つからなければ nullptr）.
	const ShaderParamDesc* FindParam(const std::string& name) const;
};

} // namespace Cake
