#pragma once
/*====================================
 *
 * 全コンポーネント型を TypeRegistry へ登録する。
 * Engine の初期化中に1回だけ呼ぶこと。
 *
 * コンポーネントを新しく作ったら、.cpp に1行追加する。
 * 書き忘れるとインスペクタに出ず、シーンにも保存されない。
 *
 * ====================================*/
namespace Cake {

void RegisterAllComponents();

} // namespace Cake
