#pragma once
/*====================================
 *
 * SphereColliderComponent の layer / mask に入れる値をまとめたもの。
 *
 * インスペクタでは数値で入るので、コードから設定する側だけがここを使う。
 * 対応表を1箇所に置いておかないと、当たらない原因が数字の取り違えになる。
 *
 * ====================================*/
namespace GameLayer {

constexpr int kPlayer = 1 << 0;       // 1.
constexpr int kPlayerAttack = 1 << 1; // 2.
constexpr int kEnemy = 1 << 2;        // 4.
constexpr int kEnemyBullet = 1 << 3;  // 8.
constexpr int kPlayerGuard = 1 << 4;  // 16.

} // namespace GameLayer
