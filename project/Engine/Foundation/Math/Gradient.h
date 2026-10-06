#pragma once
/*====================================
 *
 * 0〜1 の位置に対して色と透明度を返すグラデーション（Unity の Gradient に相当）。
 * パーティクルの Color over Lifetime で、粒の年齢（0〜1）から色を引くのに使う。
 *
 * 【キーの持ち方】
 * 色（rgb）と透明度（a）を別々のキー列で持つ。どちらも最大 kMaxKeys 個で、
 * 先頭から colorKeyCount / alphaKeyCount 個だけを使う。
 * コンポーネントに直接持たせるため、std::vector ではなく固定長の配列にしてある
 * （コンポーネントは固定サイズの素の構造体に保つ決まり。ComponentPool.h を参照）。
 *
 * 【並び順】
 * 保存されているキーの並びは time 順とは限らない。
 * エディタで位置をドラッグしている最中に並べ替えると、触っているキーが入れ替わってしまうため。
 * Evaluate は time の昇順を前提にするので、使う側で Sorted() のコピーを作ってから呼ぶこと。
 * 粒ごとに並べ替えると無駄なので、エミッターごとに1フレーム1回 Sorted() するのが想定の使い方。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

namespace Cake {

// 色のキー.
struct ColorKey {
	float time = 0.0f; // 0〜1.
	Vector3 color = Vector3::One;
};

// 透明度のキー.
struct AlphaKey {
	float time = 0.0f; // 0〜1.
	float alpha = 1.0f;
};

struct Gradient {
	static constexpr int kMaxKeys = 8;

	// 先頭から Count 個だけ使う。既定は「白・不透明」の2キー.
	ColorKey colorKeys[kMaxKeys] = {{0.0f, Vector3::One}, {1.0f, Vector3::One}};
	AlphaKey alphaKeys[kMaxKeys] = {{0.0f, 1.0f}, {1.0f, 1.0f}};
	int colorKeyCount = 2; // 1〜kMaxKeys.
	int alphaKeyCount = 2; // 1〜kMaxKeys.

	// t（0〜1）での色。キーが time の昇順に並んでいることが前提.
	// 最初のキーより前は最初の値、最後のキーより後は最後の値、間は前後2キーの線形補間.
	Vector4 Evaluate(float t) const;

	// キーを time の昇順に並べ替えたコピーを返す。個数も 1〜kMaxKeys に収める.
	Gradient Sorted() const;

	// 個数を 1〜kMaxKeys に収める。壊れた値で配列の外を読まないための共通処理.
	static int ClampKeyCount(int count);
};

} // namespace Cake
