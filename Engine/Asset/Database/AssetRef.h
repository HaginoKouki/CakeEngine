#pragma once
/*====================================
 *
 * アセットへの参照を表す値型（Asset Reference）。
 * アセット実体は持たず、「どのアセットか」を {GUID, LocalId} で指すだけ。
 *
 * 保存されるのは guid と localId のみで、handle は実行時にだけ意味を持つ。
 * シーンやプレハブを読み込んだ直後は handle が空なので、Resolve() を呼んで埋める。
 *
 * localId が 0 ならアセット本体、非0ならそのファイルに埋め込まれたサブアセットを指す
 * （OBJ の .mtl 由来マテリアルなど）。マテリアル以外では現状 0 のみ有効。
 *
 * 全コンポーネントに埋まる型なので、サイズを膨らませないこと
 * （マネージャへのポインタは持たず、Resolve() の引数で受け取る方針）。
 *
 * 対応する型は AssetTraits の特殊化で決まる。未対応の型を使うと
 * 「AssetTraits<T> が不完全型」というコンパイルエラーになる。
 *
 * ====================================*/
#include "Engine/Foundation/Identity/Guid.h"
#include "Engine/Foundation/Identity/LocalId.h"

#include "Engine/Asset/Database/AssetDatabase.h"
#include "Engine/Asset/Material/MaterialHandle.h"
#include "Engine/Asset/Model/ModelHandle.h"
#include "Engine/Asset/Texture/TextureHandle.h"

namespace Cake {

// ハンドル型ごとの解決方法。AssetDatabase のどのメソッドを呼ぶかを対応付ける.
// 未特殊化のまま使うとコンパイルエラーになる（対応漏れを検出できる）.
template <class HandleT>
struct AssetTraits;

template <>
struct AssetTraits<ModelHandle> {
	static constexpr AssetType kType = AssetType::Model;
	static ModelHandle Load(AssetDatabase& db, const Guid& guid, LocalId localId) {
		return db.LoadModel(guid, localId);
	}
};

template <>
struct AssetTraits<TextureHandle> {
	static constexpr AssetType kType = AssetType::Texture;
	static TextureHandle Load(AssetDatabase& db, const Guid& guid, LocalId localId) {
		return db.LoadTexture(guid, localId);
	}
};

template <>
struct AssetTraits<MaterialHandle> {
	static constexpr AssetType kType = AssetType::Material;
	static MaterialHandle Load(AssetDatabase& db, const Guid& guid, LocalId localId) {
		return db.LoadMaterial(guid, localId);
	}
};

template <class HandleT>
struct AssetRef {
	Guid guid;                     // 保存される.
	LocalId localId = kSelfLocalId; // 保存される。0 なら本体.
	HandleT handle{};              // 解決済みの実体。実行時のみ有効で、保存しない.

	// 参照先が設定されていないか（未設定と解決失敗を区別する）.
	bool IsEmpty() const { return !guid.IsValid(); }
	// 解決済みで、すぐ使える状態か.
	bool IsResolved() const { return handle.IsValid(); }

	// 同じものを指しているか（handle は比較しない）.
	bool PointsTo(const Guid& otherGuid, LocalId otherLocalId) const {
		return guid == otherGuid && localId == otherLocalId;
	}

	// {GUID, LocalId} からアセットを読み込み handle を埋める。成功で true.
	// 未設定（IsEmpty）の場合は何もせず false を返す.
	bool Resolve(AssetDatabase& db) {
		if (!guid.IsValid()) {
			return false;
		}
		handle = AssetTraits<HandleT>::Load(db, guid, localId);
		return handle.IsValid();
	}

	// 参照先を差し替える。解決済みハンドルは無効化して、次の Resolve に委ねる.
	void Set(const Guid& newGuid, LocalId newLocalId = kSelfLocalId) {
		guid = newGuid;
		localId = newLocalId;
		handle = HandleT{};
	}

	void Clear() {
		guid = Guid::Invalid();
		localId = kSelfLocalId;
		handle = HandleT{};
	}

	// この参照が受け付けるアセット種別。インスペクタの選択候補を絞るのに使う.
	static constexpr AssetType Type() { return AssetTraits<HandleT>::kType; }
};

} // namespace Cake
