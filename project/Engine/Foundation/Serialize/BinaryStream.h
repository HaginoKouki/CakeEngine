#pragma once
/*====================================
 *
 * アーティファクト（インポート結果）をバイト列へ落とす／読み戻すための最小ストリーム。
 *
 * Library に置くデータは人が読む必要が無いので、JSON ではなく生のバイナリで扱う。
 * OBJ のテキスト解析が数百ミリ秒かかるのに対し、ここは memcpy で済む。
 * 2回目以降の起動が速くなるのはこの差が理由。
 *
 * 【対応する型】
 * 算術型・enum のような自明にコピーできる型、std::string、std::vector。
 * ポインタ・ハンドル・ComPtr は書けない（書いてはいけない）。
 * これらは実行時にしか意味を持たないため、アーティファクトに混ぜると
 * 次回起動で必ず壊れる。Artifact 構造体にこれらを持たせないこと。
 *
 * 【エンディアン】
 * 変換しない。Library は自分のマシンで作り直せるキャッシュであり、
 * 別環境へ配る成果物ではないため（配るならここに変換を足す）。
 *
 * 【読み込み側の安全性】
 * BinaryReader は壊れたデータを受け取っても落ちない。範囲外を読もうとした時点で
 * failed_ を立てて以降の読み取りを無視する。呼び出し側は最後に IsFailed() を
 * 1回見れば良い（毎回の戻り値チェックは不要）。
 *
 * ====================================*/
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace Cake {

class BinaryWriter {
private:
	std::vector<uint8_t> buffer_;

public:
	// 算術型・enum をそのまま書く.
	template <class T>
	void Write(const T& value) {
		static_assert(std::is_trivially_copyable_v<T>, "自明にコピーできない型は書けません");
		static_assert(!std::is_pointer_v<T>, "ポインタはアーティファクトへ書けません");
		const uint8_t* src = reinterpret_cast<const uint8_t*>(&value);
		buffer_.insert(buffer_.end(), src, src + sizeof(T));
	}

	// 長さ（uint32）＋中身。
	void Write(const std::string& value) {
		Write(static_cast<uint32_t>(value.size()));
		buffer_.insert(buffer_.end(), value.begin(), value.end());
	}

	// 要素数（uint32）＋各要素。要素が自明にコピーできる場合は一括で書く.
	template <class T>
	void WriteVector(const std::vector<T>& values) {
		Write(static_cast<uint32_t>(values.size()));
		if constexpr (std::is_trivially_copyable_v<T>) {
			if (!values.empty()) {
				const uint8_t* src = reinterpret_cast<const uint8_t*>(values.data());
				buffer_.insert(buffer_.end(), src, src + sizeof(T) * values.size());
			}
		} else {
			for (const T& v : values) {
				Write(v);
			}
		}
	}

	const std::vector<uint8_t>& GetBuffer() const { return buffer_; }
	size_t GetSize() const { return buffer_.size(); }

	// 見込みサイズが分かるなら先に確保しておくと再確保が減る.
	void Reserve(size_t bytes) { buffer_.reserve(bytes); }
};

class BinaryReader {
private:
	const uint8_t* data_ = nullptr;
	size_t size_ = 0;
	size_t cursor_ = 0;
	bool failed_ = false;

	// 残りが n バイト以上あるか。足りなければ failed_ を立てる.
	bool Ensure(size_t n) {
		if (failed_ || cursor_ + n > size_) {
			failed_ = true;
			return false;
		}
		return true;
	}

public:
	BinaryReader(const uint8_t* data, size_t size) : data_(data), size_(size) {}
	explicit BinaryReader(const std::vector<uint8_t>& buffer)
		: data_(buffer.data()), size_(buffer.size()) {}

	// 失敗後は値を書き換えない（呼び出し側の変数は初期値のまま残る）.
	template <class T>
	void Read(T& out) {
		static_assert(std::is_trivially_copyable_v<T>, "自明にコピーできない型は読めません");
		if (!Ensure(sizeof(T))) {
			return;
		}
		std::memcpy(&out, data_ + cursor_, sizeof(T));
		cursor_ += sizeof(T);
	}

	void Read(std::string& out) {
		uint32_t length = 0;
		Read(length);
		if (!Ensure(length)) {
			return;
		}
		out.assign(reinterpret_cast<const char*>(data_ + cursor_), length);
		cursor_ += length;
	}

	template <class T>
	void ReadVector(std::vector<T>& out) {
		uint32_t count = 0;
		Read(count);
		if (failed_) {
			return;
		}
		if constexpr (std::is_trivially_copyable_v<T>) {
			const size_t bytes = sizeof(T) * count;
			if (!Ensure(bytes)) {
				return;
			}
			out.resize(count);
			if (count > 0) {
				std::memcpy(out.data(), data_ + cursor_, bytes);
			}
			cursor_ += bytes;
		} else {
			out.clear();
			out.resize(count);
			for (uint32_t i = 0; i < count && !failed_; ++i) {
				Read(out[i]);
			}
		}
	}

	// 途中で1度でも範囲外を踏んだか。読み終わりに1回だけ見れば良い.
	bool IsFailed() const { return failed_; }
};

} // namespace Cake
