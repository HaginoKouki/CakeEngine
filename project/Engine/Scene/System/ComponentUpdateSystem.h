#pragma once
/*====================================
 *
 * Update を持つ全コンポーネントを、優先度順に呼び出す。
 *
 * どの型を呼ぶかは TypeRegistry が知っているので、ここは並びを取り出して回すだけ。
 * コンポーネントを追加してもこのファイルは触らない。
 *
 * この関数は Play 状態を知らない。走らせるかどうかは呼び出し側（SceneManager）が
 * deltaTime で判断する。編集中は 0 が渡るため、そもそも呼ばれない。
 *
 * 【注意】Update の中でコンポーネントの追加・削除をしないこと。
 * 走査中のプールが再確保され、処理中のコンポーネントへの参照が壊れる。
 * 必要になったら遅延キューを挟む形にする。
 *
 * ====================================*/
namespace Cake {

struct UpdateContext;

void RunComponentUpdates(const UpdateContext& ctx);

} // namespace Cake
