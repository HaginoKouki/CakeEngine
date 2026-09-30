#pragma once
/*====================================
 *
 * ParticleSystemComponent を持つ全オブジェクトの粒を、生成・移動・片付けする System。
 * 粒の状態は Scene が持つ ParticleStore に置く（コンポーネントには持たせない）。
 *
 * 【呼び出し】
 * SceneManager::Update から1フレームに1回だけ呼ぶ。
 * 描画パスで呼ぶと、ゲームビューとシーンビューの2回分動いてしまう。
 *
 * 【現状の仕様】
 * ・粒が kParticleCount 個に満たなければ補充する（初回はここで全部生まれる）。
 * ・生成時の位置と速度は各軸 -kSpawnRange〜kSpawnRange のランダム、色は ParticleSystemComponent::color。
 * ・毎フレーム velocity * deltaTime だけ動かす。寿命と消滅はまだ無い。
 * ・エディタ停止中は deltaTime が 0 なので、生成だけ行われて動かない。
 *
 * ====================================*/

namespace Cake {

class Scene;

void UpdateParticleSystems(Scene& scene, float deltaTime);

} // namespace Cake
