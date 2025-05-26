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






//=============================================================================================================


#define MAX_BONE_NUM 128

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec3 vertexNormal;
in vec4 vertexBoneIds;
in vec4 vertexBoneWeights;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matNormal;
uniform mat4 boneMatrices[MAX_BONE_NUM];
uniform vec4 boneQuats[MAX_BONE_NUM*2];

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    int boneIndex0 = int(vertexBoneIds.x);
    int boneIndex1 = int(vertexBoneIds.y);
    int boneIndex2 = int(vertexBoneIds.z);
    int boneIndex3 = int(vertexBoneIds.w);

    kln_motor m1;
        m1.p1 = boneQuats[boneIndex0*2];
        m1.p2 = boneQuats[boneIndex0*2+1];
    kln_motor m2;
        m2.p1 = boneQuats[boneIndex1*2];
        m2.p2 = boneQuats[boneIndex1*2+1];
    kln_motor m3;
        m3.p1 = boneQuats[boneIndex2*2];
        m3.p2 = boneQuats[boneIndex2*2+1];
    kln_motor m4;
        m4.p1 = boneQuats[boneIndex3*2];
        m4.p2 = boneQuats[boneIndex3*2+1];
    
    kln_motor interpolatedMotor = 
    kln_add(kln_mul(m1, vertexBoneWeights.x)
    ,kln_add(kln_mul(m2, vertexBoneWeights.y)
    ,kln_add(kln_mul(m3, vertexBoneWeights.z)
    ,kln_mul(m4, vertexBoneWeights.w))));
 
    kln_point originalPosition = kln_point(vec4(1.0,vertexPosition));
    kln_point skinnedPosition = kln_apply(interpolatedMotor, originalPosition);

    vec4 skinnedNormal =
        vertexBoneWeights.x*(boneMatrices[boneIndex0]*vec4(vertexNormal, 0.0)) +
        vertexBoneWeights.y*(boneMatrices[boneIndex1]*vec4(vertexNormal, 0.0)) + 
        vertexBoneWeights.z*(boneMatrices[boneIndex2]*vec4(vertexNormal, 0.0)) + 
        vertexBoneWeights.w*(boneMatrices[boneIndex3]*vec4(vertexNormal, 0.0));
    skinnedNormal.w = 0.0;

    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    fragNormal = normalize(vec3(matNormal*skinnedNormal));

    gl_Position = mvp*vec4(skinnedPosition.p3.yzw, skinnedPosition.p3.x);
}