#pragma once
/*====================================
 *
 * この構造体にはこういうメンバがあるよ、という情報を TypeInfo として作るためのマクロ。
 * コンポーネントの構造体定義の直後、namespace Cake の中で使う。
 *
 *   CAKE_REFLECT(MeshRendererComponent)
 *       CAKE_PROPERTY(model, "Model")
 *       CAKE_PROPERTY_RANGE(intensity, "Intensity", 0.0f, 10.0f)
 *       CAKE_PROPERTY_COLOR(tint, "Tint")
 *   CAKE_REFLECT_END()
 *
 * 型はメンバの宣言から自動判別されるため、書く必要はない。
 * メンバの型を変えても記述がずれない（対応していない型ならコンパイルエラーになる）。
 *
 * 構造体の定義と同じヘッダに並べて置くこと。
 * 別ファイルに離すと、メンバを追加したときに書き足し忘れる。
 *
 * ====================================*/
#include "Engine/Foundation/Reflection/TypeInfo.h"

// 型の反映を開始する.
#define CAKE_REFLECT(TypeName)     \
	template <>                    \
	struct Reflect<TypeName> {     \
		using Self = TypeName;     \
		static TypeInfo Build() {  \
			TypeInfo info;         \
			info.name = #TypeName; \
			info.size = sizeof(Self);

// 実体。他のマクロはこれを呼ぶ.
#define CAKE_PROPERTY_IMPL(member, displayLabel, propertyType, numericWidget, minValue, maxValue, speed) \
	{                                                                                                    \
		static_assert(                                                                                   \
			(propertyType) != ::Cake::PropertyType::Unknown,                                             \
			"対応していないメンバ型です。PropertyType と PropertyTypeOf に追加してください"              \
		);                                                                                               \
		::Cake::PropertyDesc desc;                                                                       \
		desc.name = #member;                                                                             \
		desc.label = displayLabel;                                                                       \
		desc.type = (propertyType);                                                                      \
		desc.offset = offsetof(Self, member);                                                            \
		desc.widget = (numericWidget);                                                                   \
		desc.uiMin = (minValue);                                                                         \
		desc.uiMax = (maxValue);                                                                         \
		desc.dragSpeed = (speed);                                                                        \
		info.properties.push_back(desc);                                                                 \
	}

// 型を自動判別して登録する。通常はこれを使う.
#define CAKE_PROPERTY(member, displayLabel)                    \
	CAKE_PROPERTY_IMPL(                                        \
		member, displayLabel,                                  \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value, \
		::Cake::NumericWidget::Drag, kUnboundedMin, kUnboundedMax, 0.01f  \
	)

// 下限だけ縛る Drag。上限が決められない値に使う.
#define CAKE_PROPERTY_MIN(member, displayLabel, minValue)      \
	CAKE_PROPERTY_IMPL(                                        \
		member, displayLabel,                                  \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value, \
		::Cake::NumericWidget::Drag, minValue, kUnboundedMax, 0.01f  \
	)

// 上限だけ縛る Drag。下限が決められない値に使う.
#define CAKE_PROPERTY_MAX(member, displayLabel, maxValue)      \
	CAKE_PROPERTY_IMPL(                                        \
		member, displayLabel,                                  \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value, \
		::Cake::NumericWidget::Drag, kUnboundedMin, maxValue, 0.01f \
	)

// 両側を縛る Drag。範囲は広いがクランプはしたい値に使う.
#define CAKE_PROPERTY_CLAMP(member, displayLabel, minValue, maxValue) \
	CAKE_PROPERTY_IMPL(                                               \
		member, displayLabel,                                         \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value,        \
		::Cake::NumericWidget::Drag, minValue, maxValue, 0.01f        \
	)

// Slider（両端必須）。範囲が狭く、端から端まで直感的に動かしたい値に使う.
#define CAKE_PROPERTY_RANGE(member, displayLabel, minValue, maxValue) \
	CAKE_PROPERTY_IMPL(                                               \
		member, displayLabel,                                         \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value,        \
		::Cake::NumericWidget::Slider, minValue, maxValue, 0.01f      \
	)

// ドラッグ速度まで指定する Drag。farClip のように桁が大きい値に使う.
#define CAKE_PROPERTY_DRAG(member, displayLabel, minValue, maxValue, speed) \
	CAKE_PROPERTY_IMPL(                                                     \
		member, displayLabel,                                               \
		::Cake::PropertyTypeOf<decltype(Self::member)>::value,              \
		::Cake::NumericWidget::Drag, minValue, maxValue, speed              \
	)

// Vector4 をカラーピッカーとして扱う。レイアウトが同じで UI だけ違うケース.
#define CAKE_PROPERTY_COLOR(member, displayLabel)          \
	CAKE_PROPERTY_IMPL(                                    \
		member, displayLabel, ::Cake::PropertyType::Color, \
		::Cake::NumericWidget::Drag, 0.0f, 1.0f, 0.01f     \
	)

// 反映を終了する.
#define CAKE_REFLECT_END() \
	return info;           \
	}                      \
	}                      \
	;
