#pragma once
/*====================================
 *
 * シーンを JSON へ保存し、JSON から復元する。
 *
 * コンポーネントの型を一切知らない。TypeRegistry から型情報を引き、
 * プロパティ一覧を回して読み書きするだけ。コンポーネントを追加してもここは触らない。
 * 触るのは PropertyType を新設したときだけ（そのときは必ず両方の switch に追加すること）。
 *
 * ファイル版と文字列版がある。文字列版は Play/Stop のスナップショットに使う
 * （PlayModeController を参照）。
 *
 * 【保存されるID】
 * GameObjectId のうち index だけを保存する。読み込み時は空のシーンへ作り直すため
 * generation は一致せず、保存しても意味がない。親子と EntityRef は保存された index
 * を新しい GameObjectId へ読み替える（idMap）。
 *
 * 【階層はフラット + parent】
 * JSON をネストさせず、各オブジェクトが親の index を持つ形にしている。
 * 親を張り替えても JSON の構造が動かないため、差分が読みやすい。
 *
 * 【保存しないもの】
 * ワールド行列とダーティフラグは読み込み後に再計算されるため保存しない。
 * AssetRef の handle も実行時の値なので保存しない（guid と localId のみ）。
 *
 * ====================================*/
#include <string>

namespace Cake {
class Scene;
class AssetDatabase;

namespace SceneSerializer {

// シーンのファイル形式の版数。項目を変えたらインクリメントする.
constexpr int kSceneVersion = 1;

/* ===== ファイル ===== */

/// <summary>
/// シーンを.sceneファイルとして保存する.
/// </summary>
/// <param name="scene"> 保存するシーン </param>
/// <param name="path"> ファイルパス </param>
/// <returns> 成功でtrue </returns>
bool SaveScene(Scene& scene, const std::string& path);

/// <summary>
/// .sceneファイルからシーンを復元する.
/// </summary>
/// <param name="scene"> 復元するシーン（既存のデータは上書きされる） </param>
/// <param name="path"> ファイルパス </param>
/// <param name="assetDatabase"> アセットデータベース </param>
/// <returns> 成功でtrue </returns>
bool LoadScene(Scene& scene, const std::string& path, AssetDatabase& assetDatabase);

/* ===== 文字列（メモリ上のスナップショット用） ===== */

/// <summary>
/// シーンを文字列として保存する.
/// </summary>
/// <param name="scene"> 保存するシーン </param>
/// <returns> シーンの文字列表現 </returns>
std::string SaveSceneToString(Scene& scene);

/// <summary>
/// 文字列からシーンを復元する.
/// </summary>
/// <param name="scene"> 復元するシーン（既存のデータは上書きされる） </param>
/// <param name="text"> シーンの文字列表現 </param>
/// <param name="assetDatabase"> アセットデータベース </param>
/// <returns> 成功でtrue </returns>
bool LoadSceneFromString(Scene& scene, const std::string& text, AssetDatabase& assetDatabase);

} // namespace SceneSerializer
} // namespace Cake
