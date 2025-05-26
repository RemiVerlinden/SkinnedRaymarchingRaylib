#ifndef RAYMATH_TYPECAST_H
#define RAYMATH_TYPECAST_H
#include <klein/klein.hpp> 
#include "raylib.h"
#include "types.h"

namespace DQ 
{
	__m128 quaternion_to_m128(const Quaternion& q) noexcept;
	DQ::BoneTransform RaylibTransformToBoneTransform(const Transform& t) noexcept;
}

#endif // !RAYMATH_TYPECAST_H
