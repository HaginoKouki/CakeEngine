#pragma once
/*====================================
 *
 * ParticleSystemComponent を持つ全オブジェクトの粒を、生成・移動・消滅・片付けする System。
 * 粒の状態と発生タイマーは Scene が持つ ParticleStorage に置く（コンポーネントには持たせない）。
 *
 * 【呼び出し】
 * SceneManager::Update から1フレームに1回だけ呼ぶ。
 * 描画パスで呼ぶと、ゲームビューとシーンビューの2回分動いてしまう。
 *
 * 【現状の仕様】
 * ・開始時は粒が無い。emitInterval 秒ごとに1つ生成する（1フレームで間隔を何回もまたいだら、その回数分生成する）。
 * ・生成時の位置と速度は各軸 -kSpawnRange〜kSpawnRange のランダム、色と寿命は ParticleSystemComponent から写す。
 * ・毎フレーム、経過時間を進めて velocity * deltaTime だけ動かし、寿命（lifeTime）に達した粒を消す。
 * ・エディタ停止中・一時停止中は deltaTime が 0 なので、生成・移動・消滅は起きない
 *   （コンポーネントを外した・オブジェクトを破棄した分の片付けだけ行う）。
 *
 * ====================================*/

namespace Cake {

class Scene;

void UpdateParticleSystems(Scene& scene, float deltaTime);

} // namespace Cake
