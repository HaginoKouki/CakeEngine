#pragma once
/*====================================
 *
 * 型のメンバ構成を実行時に読めるデータとして表す。
 * これ1つあれば、インスペクタの描画・JSONへの保存・JSONからの読み込みを
 * 「プロパティ一覧を回すだけの汎用コード」で書ける。
 * コンポーネントを1つ足したときに触る箇所が、その型の定義だけで済むようになる。
 *
 * 【offset について】
 * メンバの位置をバイトオフセットで持ち、実体へは (uint8_t*)instance + offset で到達する。
 * これが成立するのはコンポーネントが仮想関数を持たない素の構造体だから
 * （ComponentPool のコメントを参照）。基底クラスを付けるとここが崩れる。
 *
 * 【name と label】
 * name はシリアライズのキー。変更すると既存のシーンファイルが読めなくなる。
 * label はインスペクタの表示名で、いつ変えても安全。
 *
 * このヘッダは Scene に依存しない（純粋な型情報のみ）。
 * コンポーネントの生成・取得は TypeRegistry 側が担当する。
 *
 * ====================================*/
#include <cstddef>
#include <cfloat>
#include <cstdint>
#include <string>
#include <vector>

#include "Engine/Foundation/Math/Transform.h"
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Asset/Database/AssetRef.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Asset/Texture/TextureHandle.h"

#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

// プロパティとして扱える型の一覧.
// 【重要】ここに追加したら、インスペクタとシリアライザの switch にも必ず追加すること.
enum class PropertyType : uint8_t {
	Unknown,

	Bool,
	Int,
	Float,

	Vector2,
	Vector3,
	Vector4,
	Color, // レイアウトは Vector4 と同じだが、UIはカラーピッカーになる.

	String,
	Transform,

	AssetRefModel,
	AssetRefTexture,
	AssetRefMaterial,

	EntityRef, // GameObjectId。同じシーン内の他オブジェクトへの参照.
};

// 数値プロパティをどのウィジェットで編集するか.
// Slider は両端が必須（範囲が位置に対応するため）、Drag は片側だけでも成立する.
enum class NumericWidget {
	Drag,
	Slider,
};

// C++ の型から PropertyType を引く。CAKE_PROPERTY が型を自動判別するのに使う.
// 未対応の型はここに特殊化が無いため Unknown となり、マクロ側の static_assert で弾かれる.
template <class T>
struct PropertyTypeOf {
	static constexpr PropertyType value = PropertyType::Unknown;
};

template <> struct PropertyTypeOf<bool> { static constexpr PropertyType value = PropertyType::Bool; };
template <> struct PropertyTypeOf<int> { static constexpr PropertyType value = PropertyType::Int; };
template <> struct PropertyTypeOf<float> { static constexpr PropertyType value = PropertyType::Float; };
template <> struct PropertyTypeOf<Vector2> { static constexpr PropertyType value = PropertyType::Vector2; };
template <> struct PropertyTypeOf<Vector3> { static constexpr PropertyType value = PropertyType::Vector3; };
template <> struct PropertyTypeOf<Vector4> { static constexpr PropertyType value = PropertyType::Vector4; };
template <> struct PropertyTypeOf<std::string> { static constexpr PropertyType value = PropertyType::String; };
template <> struct PropertyTypeOf<Transform> { static constexpr PropertyType value = PropertyType::Transform; };
template <> struct PropertyTypeOf<GameObjectId> { static constexpr PropertyType value = PropertyType::EntityRef; };
template <> struct PropertyTypeOf<AssetRef<ModelHandle>> { static constexpr PropertyType value = PropertyType::AssetRefModel; };
template <> struct PropertyTypeOf<AssetRef<TextureHandle>> { static constexpr PropertyType value = PropertyType::AssetRefTexture; };
template <> struct PropertyTypeOf<AssetRef<MaterialHandle>> { static constexpr PropertyType value = PropertyType::AssetRefMaterial; };

// 下限を指定しないことを示す（-FLT_MAX）.
constexpr const float kUnboundedMin = -FLT_MAX;
// 上限を指定しないことを示す（+FLT_MAX）.
constexpr const float kUnboundedMax = FLT_MAX;

struct PropertyDesc {
	// どちらもマクロが渡す文字列リテラルなので、寿命はプログラム終了まで.
	const char* name = "";  // シリアライズのキー（C++のメンバ名）.
	const char* label = ""; // インスペクタの表示名.

	PropertyType type = PropertyType::Unknown;
	size_t offset = 0;

	NumericWidget widget = NumericWidget::Drag;

	float uiMin = kUnboundedMin;
	float uiMax = kUnboundedMax;

	// Drag のときの1ピクセルあたりの変化量.
	float dragSpeed = 0.01f;
};

struct TypeInfo {
	const char* name = ""; // 型名。シリアライズのキーになる.
	size_t size = 0;
	std::vector<PropertyDesc> properties;

	const PropertyDesc* FindProperty(const std::string& propertyName) const {
		for (const PropertyDesc& desc : properties) {
			if (propertyName == desc.name) {
				return &desc;
			}
		}
		return nullptr;
	}
};

// インスタンスの先頭アドレスから、指定プロパティの実体へ到達する.
inline void* GetPropertyPtr(void* instance, const PropertyDesc& desc) {
	return static_cast<uint8_t*>(instance) + desc.offset;
}
inline const void* GetPropertyPtr(const void* instance, const PropertyDesc& desc) {
	return static_cast<const uint8_t*>(instance) + desc.offset;
}

// CAKE_REFLECT が特殊化する。未定義の型を登録しようとするとコンパイルエラーになる.
template <class T>
struct Reflect;

} // namespace Cake
