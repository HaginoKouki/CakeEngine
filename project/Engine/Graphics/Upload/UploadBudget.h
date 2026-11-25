#pragma once
/*====================================
 *
 * 1フレームに GPU へ転送する量の上限と、何件を転送するかを決める関数
 * D3D12 を知らない.
 *
 * ====================================*/
#include <cstdint>
#include <cstddef>
#include <span>

namespace Cake {

// 1フレームに転送する量の上限.
// 仮の値。手順9で計測して見直す.
inline constexpr uint64_t kUploadBytesPerFrame = 16ull * 1024 * 1024;

// sizes の先頭から何件を、このフレームで転送するかを返す.
// 合計が budget を超えない所までを取る。ただし1件目は、budget より大きくても必ず取る.
size_t CountUploadsThisFrame(std::span<const uint64_t> sizes, uint64_t budget);

} // namespace Cake
