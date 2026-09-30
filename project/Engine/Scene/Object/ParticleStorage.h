#pragma once
/*====================================
 *
 * パーティクルの粒の状態を、エミッター（ParticleSystemComponent を持つ GameObject）ごとに持つ置き場。
 * Unity でいうと、ParticleSystem の内部（C++側）が持っている粒のバッファにあたる。
 *
 * 【なぜコンポーネントの外に置くか】
 * コンポーネントは standard-layout を保つ決まりがあり（ComponentPool.h を参照）、
 * 数が増減する配列を直接持たせられないため。
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
};

class ParticleStorage {
private:
	// GameObjectId は世代番号を持つので、空きスロットが再利用されても取り違えない.
	std::map<GameObjectId, std::vector<Particle>> emitters_;

public:
	// 無ければ空の配列を作って返す.
	std::vector<Particle>& GetOrCreate(GameObjectId owner) { return emitters_[owner]; }

	// 無ければ nullptr.
	const std::vector<Particle>* Find(GameObjectId owner) const {
		auto it = emitters_.find(owner);
		return (it != emitters_.end()) ? &it->second : nullptr;
	}

	// shouldRemove(owner) が true を返したエミッターの粒を捨てる.
	template <class Predicate>
	void RemoveIf(Predicate shouldRemove) {
		std::erase_if(emitters_, [&](const auto& entry) { return shouldRemove(entry.first); });
	}

	void Clear() { emitters_.clear(); }
};

} // namespace Cake
