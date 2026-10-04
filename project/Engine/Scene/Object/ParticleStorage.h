#pragma once
/*====================================
 *
 * パーティクルの粒の状態を、エミッター（ParticleSystemComponent を持つ GameObject）ごとに持つ置き場。
 * Unity でいうと、ParticleSystem の内部（C++側）が持っている粒のバッファにあたる。
 * 粒の配列に加えて、次の粒を出すまでの発生タイマーもエミッターごとにここで持つ。
 *
 * 【なぜコンポーネントの外に置くか】
 * コンポーネントは standard-layout を保つ決まりがあり（ComponentPool.h を参照）、
 * 数が増減する配列を直接持たせられないため。
 * 発生タイマーも実行中にだけ変わる状態なので、設定を持つコンポーネントとは分けてここに置く。
 *
 * 【寿命】
 * Scene が所有し、Scene::Clear() で一緒に空になる。
 * これにより、シーンの読み込みや Play → Stop の復元で古い粒が残らない。
 * コンポーネントを外した・オブジェクトを破棄した分は UpdateParticleSystems が片付ける。
 *
 * このヘッダは Scene に依存しない（Scene.h から include されるため）。
 *
 * ====================================*/
#include <map>
#include <vector>

#include "Engine/Foundation/Math/Transform.h"
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

// 粒1つ分の状態.
struct Particle {
	Transform transform;              // エミッター（GameObject）からの相対.
	Vector3 velocity = Vector3::Zero; // 1秒あたりの移動量.
	Vector4 color = Vector4::One;
	float lifeTime = 0.0f;    // 寿命（秒）。生成時に ParticleSystemComponent::lifeTime を写す.
	float currentTime = 0.0f; // 生成されてからの経過時間（秒）。lifeTime 以上になったら消す.
};

// エミッター1つ分の実行時の状態.
struct EmitterState {
	std::vector<Particle> particles;
	float emitTimer = 0.0f; // 前回の生成からの経過時間（秒）.
};

class ParticleStorage {
private:
	// GameObjectId は世代番号を持つので、空きスロットが再利用されても取り違えない.
	std::map<GameObjectId, EmitterState> emitters_;

public:
	// 無ければ空の状態を作って返す.
	EmitterState& GetOrCreate(GameObjectId owner) { return emitters_[owner]; }

	// 無ければ nullptr.
	const EmitterState* Find(GameObjectId owner) const {
		auto it = emitters_.find(owner);
		return (it != emitters_.end()) ? &it->second : nullptr;
	}

	// shouldRemove(owner) が true を返したエミッターの状態を捨てる.
	template <class Predicate>
	void RemoveIf(Predicate shouldRemove) {
		std::erase_if(emitters_, [&](const auto& entry) { return shouldRemove(entry.first); });
	}

	void Clear() { emitters_.clear(); }
};

} // namespace Cake
