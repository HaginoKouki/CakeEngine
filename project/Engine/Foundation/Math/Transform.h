#pragma once
/*====================================
 *
 * 3D空間上の位置（translate）、回転（rotate）、スケール（scale）を保持する構造体と変換行列（TransformationMatrix）を定義する構造体。
 * ゲームオブジェクトの姿勢を表現し、Rendererで行列変換に使用される。
 * 主にGameSceneのゲームオブジェクト更新時に更新される。
 *
 * ====================================*/
#include "Engine/Foundation/Math/Vector.h"
#include "Engine/Foundation/Math/Matrix.h"

namespace Cake {

struct Transform {
	Vector3 scale = Vector3::One;
	Vector3 rotate = Vector3::Zero;
	Vector3 translate = Vector3::Zero;
};

struct TransformationMatrix {
	Matrix4x4 WVP = Matrix4x4::Identity;
	Matrix4x4 World = Matrix4x4::Identity;
};

namespace Math {
// アフィン変換行列を scale / rotate / translate へ分解する。
// せん断（非一様スケール＋回転の組み合わせ）は表現できないため、その場合は近い値に丸められる.
Transform DecomposeAffine(const Matrix4x4& m);
}

}	// namespace Cake
