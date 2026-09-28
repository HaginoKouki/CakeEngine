#pragma once
/*====================================
 *
 * ヒットストップと画面シェイクを一手に引き受けるコンポーネント。カメラに1つだけ置く。
 *
 * 攻撃側・被弾側から RequestHitStop / RequestShake を呼ぶと、ここが実際の
 * 時間停止とカメラの揺らしを行う。演出の実装を1箇所に集めることで、
 * 「誰が時間を止めたか」を追わなくて済むようにしている。
 *
 * 【タイマーは unscaledDeltaTime で進める】
 * ヒットストップ中は deltaTime がほぼ0になるので、そちらで数えると永久に明けない。
 *
 * 【シェイクはローカル座標を直接触る】
 * 初回に構えの位置を控え、そこからのずれとして毎フレーム上書きする。
 * カメラの子（UI）も一緒に揺れるが、そのほうが画面全体が殴られたように見える。
 *
 * 【呼び出しはプール経由】
 * シーンに1つしか無い前提で、ForEach で見つけた最初の1つへ要求を渡す。
 * ID を配線しなくて済むので、要求する側は自分の位置を知らなくてよい。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"

#include "Engine/Reflection/ReflectMacros.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {
struct UpdateContext;
} // namespace Cake

struct GameFeelComponent {
	float hitStopScale = 0.04f; // 停止中のタイムスケール。0にすると復帰が不安定になる.
	float shakeDecay = 1.0f;    // 揺れが収まる速さの係数.

	// --- 実行時 ---
	bool started = false;
	float hitStopTimer = 0.0f;
	float shakeTimer = 0.0f;
	float shakeDuration = 0.0f;
	float shakePower = 0.0f;
	unsigned int randomState = 12345u; // 揺れ用の簡易乱数.
	Cake::Vector3 basePosition{};

	void Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self);

	// --- 外から呼ぶ入口。シーン中の GameFeel を探して要求を渡す ---
	static void RequestHitStop(const Cake::UpdateContext& ctx, float duration);
	static void RequestShake(const Cake::UpdateContext& ctx, float power, float duration);
};

namespace Cake {
CAKE_REFLECT(GameFeelComponent)
CAKE_PROPERTY_RANGE(hitStopScale, "Hit Stop Scale", 0.01f, 1.0f)
CAKE_PROPERTY_RANGE(shakeDecay, "Shake Decay", 0.1f, 5.0f)
CAKE_REFLECT_END()
} // namespace Cake
