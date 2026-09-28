#pragma once
/*====================================
 *
 * GameObject を子孫ごと複製する。
 *
 * コンポーネントの型を一切知らないまま複製するため、TypeRegistry の
 * ComponentOps::copy を使う。コンポーネントを追加してもこのファイルは触らない。
 *
 * 【EntityRef は張り替えない】
 * 複製元が他のオブジェクトを参照している場合、複製後もそのまま元の相手を指す。
 * 「複製した部分木の中で閉じた参照だけを新しい方へ向け直す」処理は入れていない。
 * 必要になったら、複製で作った旧ID→新IDの対応表を使って後段で解決すること。
 *
 * 【将来】
 * Prefab の実体化もこの仕組みの上に載る。複製元がシーン内のオブジェクトか
 * ファイルから読んだテンプレートかが違うだけで、やることは同じ。
 *
 * ====================================*/
#include "Engine/Scene/Object/GameObjectId.h"

namespace Cake {

class Scene;

// source を子孫ごと複製し、parent の子として追加する。
// parent が無効ならルート直下に作る。
// 戻り値は複製されたオブジェクトのID。source が無効なら無効なIDを返す.
GameObjectId DuplicateGameObject(Scene& scene, GameObjectId source, GameObjectId parent);

} // namespace Cake
