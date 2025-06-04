#version 330

//=============================================================================================================

#ifndef KLEIN_GUARD
#define KLEIN_GUARD

// p0 -> (e0, e1, e2, e3)
// p1 -> (1, e23, e31, e12)
// p2 -> (e0123, e01, e02, e03)
// p3 -> (e123, e032, e013, e021)

struct kln_plane
{
    vec4 p0;
};

struct kln_line
{
    vec4 p1;
    vec4 p2;
};

// If integrating this library with other code, remember that the point layout
// here has the homogeneous component in p3[0] and not p3[3]. The swizzle to
// get the vec3 Cartesian representation is p3.yzw
struct kln_point
{
    vec4 p3;
};

struct kln_rotor
{
    vec4 p1;
};

struct kln_translator
{
    vec4 p2;
};

struct kln_motor
{
    vec4 p1;
    vec4 p2;
};

kln_rotor kln_mul(in kln_rotor a, in kln_rotor b)
{
    kln_rotor c;
    c.p1 = a.p1.x * b.p1;
    c.p1 -= a.p1.yzwy * b.p1.ywyz;

    vec4 t = a.p1.zyzw * b.p1.zxxx;
    t += a.p1.wwyz * b.p1.wzwy;
    t.x = -t.x;

    c.p1 += t;
    return c;
}

kln_translator kln_mul(in kln_translator a, in kln_translator b)
{
    // (1 + a.p2) * (1 + b.p2) = 1 + a.p2 + b.p2
    kln_translator c;
    c.p2 = a.p2 + b.p2;
    return c;
}

kln_motor kln_add(in kln_motor a, in kln_motor b)
{
    kln_motor c;
    c.p1 = a.p1 + b.p1;
    c.p2 = a.p2 + b.p2;

    return c;
}

kln_motor kln_mul(in kln_motor a, in float b)
{
    kln_motor c;
    c.p1 = a.p1 * b;
    c.p2 = a.p2 * b;

    return c;
}

kln_motor kln_mul(in kln_motor a, in kln_motor b)
{
    kln_motor c;
    vec4 a_zyzw = a.p1.zyzw;
    vec4 a_ywyz = a.p1.ywyz;
    vec4 a_wzwy = a.p1.wzwy;
    vec4 c_wwyz = b.p1.wwyz;
    vec4 c_yzwy = b.p1.yzwy;

    c.p1 = a.p1.x * b.p1;
    vec4 t = a_ywyz * c_yzwy;
    t += a_zyzw * b.p1.zxxx;
    t.x = -t.x;
    c.p1 += t;
    c.p1 -= a_wzwy * c_wwyz;

    c.p2 = a.p1.x * b.p2;
    c.p2 += a.p2 * b.p1.x;
    c.p2 += a_ywyz * b.p2.yzwy;
    c.p2 += a.p2.ywyz * c_yzwy;
    t = a_zyzw * b.p2.zxxx;
    t += a_wzwy * b.p2.wwyz;
    t += a.p2.zxxx * b.p1.zyzw;
    t += a.p2.wzwy * c_wwyz;
    t.x = -t.x;
    c.p2 -= t;
    return c;
}

// Inverse of a motor (conjugate for normalized dual quaternions)
kln_motor kln_inverse(in kln_motor m)
{
    kln_motor inv;
    // For normalized dual quaternions, inverse is conjugate
    // Conjugate of dual quaternion: (p1, p2) -> (p1*, -p2*)
    // Where p1* means conjugate of quaternion p1
    inv.p1 = vec4(m.p1.x, -m.p1.yzw);
    inv.p2 = vec4(-m.p2.x, m.p2.yzw);
    return inv;
}

kln_plane kln_apply(in kln_rotor r, in kln_plane p)
{
    vec4 dc_scale = vec4(1.0, 2.0, 2.0, 2.0);
    vec4 neg_low = vec4(-1.0, 1.0, 1.0, 1.0);

    vec4 t1 = r.p1.zxxx * r.p1.zwyz;
    t1 += r.p1.yzwy * r.p1.yyzw;
    t1 *= dc_scale;

    vec4 t2 = r.p1 * r.p1.xwyz;
    t2 -= (r.p1.wxxx * r.p1.wzwy) * neg_low;
    t2 *= dc_scale;

    vec4 t3 = r.p1 * r.p1;
    t3 -= r.p1.xwyz * r.p1.xwyz;
    t3 += r.p1.xxxx * r.p1.xxxx;
    t3 -= r.p1.xzwy * r.p1.xzwy;

    // TODO: provide variadic rotor-plane application
    kln_plane q;
    q.p0 = t1 * p.p0.xzwy;
    q.p0 += t2 * p.p0.xwyz;
    q.p0 += t3 * p.p0;
    return q;
}

kln_plane kln_apply(in kln_motor m, in kln_plane p)
{
    vec4 dc_scale = vec4(1.0, 2.0, 2.0, 2.0);
    vec4 neg_low = vec4(-1.0, 1.0, 1.0, 1.0);

    vec4 t1 = m.p1.zxxx * m.p1.zwyz;
    t1 += m.p1.yzwy * m.p1.yyzw;
    t1 *= dc_scale;

    vec4 t2 = m.p1 * m.p1.xwyz;
    t2 -= (m.p1.wxxx * m.p1.wzwy) * neg_low;
    t2 *= dc_scale;

    vec4 t3 = m.p1 * m.p1;
    t3 -= m.p1.xwyz * m.p1.xwyz;
    t3 += m.p1.xxxx * m.p1.xxxx;
    t3 -= m.p1.xzwy * m.p1.xzwy;

    vec4 t4 = m.p1.x * m.p2;
    t4 += m.p1.xzwy * m.p2.xwyz;
    t4 += m.p1 * m.p2.x;
    t4 -= m.p1.xwyz * m.p2.xzwy;
    t4 *= vec4(0.0, 2.0, 2.0, 2.0);

    // TODO: provide variadic motor-plane application
    kln_plane q;
    q.p0 = t1 * p.p0.xzwy;
    q.p0 += t2 * p.p0.xwyz;
    q.p0 += t3 * p.p0;
    q.p0 += vec4(dot(t4, p.p0), 0.0, 0.0, 0.0);
    return q;
}

kln_point kln_apply(in kln_rotor r, in kln_point p)
{
    vec4 scale = vec4(0, 2, 2, 2);

    vec4 t1 = r.p1 * r.p1.xwyz;
    t1 -= r.p1.x * r.p1.xzwy;
    t1 *= scale;

    vec4 t2 = r.p1.x * r.p1.xwyz;
    t2 += r.p1.xzwy * r.p1;
    t2 *= scale;

    vec4 t3 = r.p1 * r.p1;
    t3 += r.p1.yxxx * r.p1.yxxx;
    vec4 t4 = r.p1.zwyz * r.p1.zwyz;
    t4 += r.p1.wzwy * r.p1.wzwy;
    t3 -= t4 * vec4(-1.0, 1.0, 1.0, 1.0);

    // TODO: provide variadic rotor-point application
    kln_point q;
    q.p3 = t1 * p.p3.xwyz;
    q.p3 += t2 * p.p3.xzwy;
    q.p3 += t3 * p.p3;
    return  q;
}
// THIS IS THE SANDWICH PRODUCT MP~M WHICH IS NOT WHAT WE WANT.
// INSTEAD WE WANT ~MPM WHICH WILL RESULT IN CLOCKWISE ROTATION INSTEAD OF THE CURRENT COUNTERCLOCKWISE
// WE FIX THIS BY INVERTING THE DUAL QUATERNION BEFORE SENDING IT TO THE GPU
kln_point kln_apply(in kln_motor m, in kln_point p)
{
    vec4 scale = vec4(0, 2, 2, 2);

    vec4 t1 = m.p1 * m.p1.xwyz;
    t1 -= m.p1.x * m.p1.xzwy;
    t1 *= scale;

    vec4 t2 = m.p1.x * m.p1.xwyz;
    t2 += m.p1.xzwy * m.p1;
    t2 *= scale;

    vec4 t3 = m.p1 * m.p1;
    t3 += m.p1.yxxx * m.p1.yxxx;
    vec4 t4 = m.p1.zwyz * m.p1.zwyz;
    t4 += m.p1.wzwy * m.p1.wzwy;
    t3 -= t4 * vec4(-1.0, 1.0, 1.0, 1.0);

    t4 = m.p1.xzwy * m.p2.xwyz;
    t4 -= m.p1.x * m.p2;
    t4 -= m.p1.xwyz * m.p2.xzwy;
    t4 -= m.p1 * m.p2.x;
    t4 *= scale;

    // TODO: provide variadic motor-point application
    kln_point q;
    q.p3 = t1 * p.p3.xwyz;
    q.p3 += t2 * p.p3.xzwy;
    q.p3 += t3 * p.p3;
    q.p3 += t4 * p.p3.x;
    return  q;
}

// If no entity is provided as the second argument, the motor is
// applied to the origin.
// NOTE: The motor MUST be normalized for the result of this operation to be
// well defined.
kln_point kln_apply(in kln_motor m)
{
    kln_point p;
    p.p3 = m.p1 * m.p2.x;
    p.p3 += m.p1.x * m.p2;
    p.p3 += m.p1.xwyz * m.p2.xzwy;
    p.p3 = m.p1.xzwy * m.p2.xwyz - p.p3;
    p.p3 *= vec4(0.0, 2.0, 2.0, 2.0);
    p.p3.x = 1.0;
    return p;
}

#endif // KLEIN_GUARD

//=============================================================================================================

#define MAX_BONE_NUM 128

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 preSkinnedFragPosition;
in vec3 fragWorldPosition;
in vec4 fragBoneWeights;
in vec4 fragBoneIds;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler3D bindPose3DTextureSDF;
uniform vec4 colDiffuse;

uniform vec3 minBounds3DTextureSDF;
uniform vec3 maxBounds3DTextureSDF;

uniform vec3 clippingVolumePosition;
uniform vec3 clippingVolumeScale;
uniform vec3 cameraPosition;
uniform float time;

// Bone dual quaternions for inverse transformation
uniform vec4 boneDualQuaternions[MAX_BONE_NUM*2];

float ellipsoidSDF( vec3 pos, vec3 center, vec3 r )
{
    vec3 p = pos - center;
    float k1 = length(p/r);
    return (k1-1.0)*min(min(r.x,r.y),r.z);
}

// Remap world position to SDF texture coordinates (0-1 range)
vec3 worldToSDFCoords(vec3 worldPos) {
    return (worldPos - minBounds3DTextureSDF) / (maxBounds3DTextureSDF - minBounds3DTextureSDF);
}

kln_motor GetMotor(int boneIndex)
{
    kln_motor m;
    m.p1 = boneDualQuaternions[boneIndex*2];
    m.p2 = boneDualQuaternions[boneIndex*2+1];
    return m;
}

void main() {
    // SDF: <0 inside ellipsoid
    float woundSDF = ellipsoidSDF(preSkinnedFragPosition, clippingVolumePosition, clippingVolumeScale);
    
    if (woundSDF > 0.0) {
        // Outside the wound: render as normal
        vec4 texelColor = texture(texture0, fragTexCoord);
        finalColor = texelColor * colDiffuse * fragColor;
        return;
    }

    // Inside the wound: raymarch through ellipsoid and mesh SDF
    vec3 rayOrigin = preSkinnedFragPosition;
    vec3 rayDir = normalize(fragWorldPosition - cameraPosition);

    // Create interpolated motor from bone weights for inverse transformation
    int boneIndex0 = int(fragBoneIds.x);
    int boneIndex1 = int(fragBoneIds.y);
    int boneIndex2 = int(fragBoneIds.z);
    int boneIndex3 = int(fragBoneIds.w);
    
    kln_motor interpolatedMotor = 
    kln_add(kln_mul(GetMotor(boneIndex0), fragBoneWeights.x)
    ,kln_add(kln_mul(GetMotor(boneIndex1), fragBoneWeights.y)
    ,kln_add(kln_mul(GetMotor(boneIndex2), fragBoneWeights.z)
    ,kln_mul(GetMotor(boneIndex3), fragBoneWeights.w))));
    
    // Get the inverse motor and extract the rotational part (rotor) for direction transformation
    kln_motor inverseMotor = kln_inverse(interpolatedMotor);
    kln_rotor inverseRotor;
    inverseRotor.p1 = inverseMotor.p1; // Extract rotational part only
    
    // Transform ray direction to T-pose space using inverse rotor
    kln_point rayDirPoint = kln_point(vec4(0.0, rayDir)); // Direction vector (w=0)
    kln_point transformedRayDirPoint = kln_apply(inverseRotor, rayDirPoint);
    vec3 transformedRayDir = transformedRayDirPoint.p3.yzw;

    bool hitEllipsoidBorder = false;
    const int maxSteps = 128;
    const float minStep = 0.001;

    float depth = 0.0;

    for (int i = 0; i < maxSteps; ++i) {
        vec3 pos = rayOrigin + transformedRayDir * depth;
        float ellipsoidDist = ellipsoidSDF(pos, clippingVolumePosition, clippingVolumeScale);

        const float EPSILON = 0.0001;
        const float MAX_DIST = 200.;
        if (ellipsoidDist >= 0) 
        {
            hitEllipsoidBorder = true;
            break;
        }
        depth += max(abs(ellipsoidDist),minStep);

        if(depth >= MAX_DIST) break;
    }

    if (hitEllipsoidBorder) {
        vec3 hitPosition = rayOrigin + transformedRayDir * depth;
        
        // Sample the SDF texture at the hit position (already in T-pose space)
        float sampledDepth = texture(bindPose3DTextureSDF, worldToSDFCoords(hitPosition)).r;

        if(sampledDepth > 0.5025)
            discard;
        // Ray hit ellipsoid border while still inside mesh - render wound surface
        finalColor = vec4(0.8, 0.1, 0.1, 1.0); // Red wound color
    } else {
        // Ray didn't hit anything definitive - fallback
        finalColor = vec4(1,0,1, 1.0); // pink error color
    }
}