#include "raymath_typecast.h"
#include "raymath.h"
__m128 DQ::quaternion_to_m128(const Quaternion& q) noexcept
{
    return _mm_set_ps(q.z, q.y, q.x, q.w);
}

DQ::BoneTransform DQ::RaylibTransformToBoneTransform(const Transform& t) noexcept
{
    kln::rotor rotor(quaternion_to_m128(t.rotation));


    Vector4 position = { 0,t.translation.x, t.translation.y, t.translation.z };
    position /= 2;
    kln::translator translator{};
    translator.load_normalized(reinterpret_cast<float*>(&position));
    return rotor * translator;
}