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

// Normalizes motor such that m * ~m = 1
kln_motor kln_normalize(in kln_motor m)
{
    kln_motor result;
    
    // Compute |b|^2 = dot(p1, p1)
    float b2 = dot(m.p1, m.p1);
    
    // Compute 1/sqrt(|b|^2) = 1/|b|
    float s = inversesqrt(b2);
    
    // Compute b0*c0 - b1*c1 - b2*c2 - b3*c3
    float bc = m.p1.x * m.p2.x - dot(m.p1.yzw, m.p2.yzw);
    
    // t = bc / |b|^3 = bc / (|b|^2 * |b|) = bc * s / b2
    float t = bc * s / b2;
    
    // Apply normalization
    result.p1 = m.p1 * s;
    result.p2 = m.p2 * s - m.p1 * t;
    result.p2.x = m.p2.x * s + m.p1.x * t;
    
    return result;
}

// Constrains the motor to traverse the shortest arc
kln_motor kln_constrain(in kln_motor m)
{
    kln_motor result;
    
    // Check if the scalar part (first component) is negative
    if (m.p1.x < 0.0) {
        // Flip both p1 and p2 to ensure shortest arc
        result.p1 = -m.p1;
        result.p2 = -m.p2;
    } else {
        result.p1 = m.p1;
        result.p2 = m.p2;
    }
    
    return result;
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
in vec3 rayDirVertex;

out vec4 finalColor;

uniform sampler2D texture0;
uniform sampler3D bindPose3DTextureSDF;
uniform sampler3D bindPose3DTextureBoneWeight;
uniform sampler3D bindPose3DTextureBoneIndex;
uniform vec4 colDiffuse;

uniform vec3 minBounds3DTextureSDF;
uniform vec3 maxBounds3DTextureSDF;
uniform vec3 minBounds3DTextureWeight;
uniform vec3 maxBounds3DTextureWeight;

uniform vec3 clippingVolumePosition;
uniform vec3 clippingVolumeScale;
uniform vec3 cameraPosition;

uniform vec3 ellipsoidLightPosition;
uniform vec3 ellipsoidLightColor = vec3(1,1,1);  // Light color/intensity, e.g., vec3(1,1,1)

uniform float time;

// Rendering mode control (0=shaded, 1=checker debug, 2=ray heatmap)
uniform int renderingMode;

// Bone dual quaternions for inverse transformation
uniform vec4 boneDualQuaternions[MAX_BONE_NUM*2];

float ellipsoidSDF( vec3 pos, vec3 center, vec3 r )
{
    vec3 p = pos - center;
    float k1 = length(p/r);
    return (k1-1.0)*min(min(r.x,r.y),r.z);
}
vec3 ellipsoidNormal(vec3 pos, vec3 center, vec3 radii) {
    return -normalize((pos - center) / (radii * radii)); // negative for interior surface
}


// Remap world position to SDF texture coordinates (0-1 range)
vec3 worldToSDFCoords(vec3 worldPos) {
    return (worldPos - minBounds3DTextureSDF) / (maxBounds3DTextureSDF - minBounds3DTextureSDF);
}
vec3 worldToSDFCoordsWeight(vec3 worldPos) {
    return (worldPos - minBounds3DTextureWeight) / (maxBounds3DTextureWeight - minBounds3DTextureWeight);
}

kln_motor GetMotor(int boneIndex)
{
    kln_motor m;
    m.p1 = boneDualQuaternions[boneIndex*2];
    m.p2 = boneDualQuaternions[boneIndex*2+1];
    return m;
}

// Inputs: indices (ivec4), weights (vec4), current rayDir
vec3 TransformRayDirInverseSkinning(ivec4 boneIndices, vec4 boneWeights, vec3 rayDir) {
    kln_motor interpolatedMotor = 
        kln_add(kln_mul(GetMotor(boneIndices.x), boneWeights.x),
        kln_add(kln_mul(GetMotor(boneIndices.y), boneWeights.y),
        kln_add(kln_mul(GetMotor(boneIndices.z), boneWeights.z),
                kln_mul(GetMotor(boneIndices.w), boneWeights.w))));

    interpolatedMotor = kln_constrain(interpolatedMotor);
    interpolatedMotor = kln_normalize(interpolatedMotor);

    kln_motor inverseMotor = kln_inverse(interpolatedMotor);
    kln_rotor inverseRotor;
    inverseRotor.p1 = inverseMotor.p1;

    kln_point rayDirPoint = kln_point(vec4(0.0, rayDir));
    kln_point transformedRayDirPoint = kln_apply(inverseRotor, rayDirPoint);
    return normalize(transformedRayDirPoint.p3.yzw);
}

//
// debugEllipsoidColor: given a point on the ellipsoid surface (hitPos),
// plus the ellipsoid's center & radii, spit out a distinct RGBA color
// so you can visually see exactly WHERE on the ellipse you hit.
//
// In this example it builds a simple 3D checker pattern (8x8x8 cells),
// but you can tweak checkerCount or swap in any other mapping.
//
//vec4 debugEllipsoidColor(in vec3 hitPos,
//                         in vec3 ellCenter,
//                         in vec3 ellRadii)
//{
//    // 1) Transform into unit-sphere 'param space'
//    vec3 pLocal = (hitPos - ellCenter) / ellRadii;
//    // Now pLocal should be 1 on the surface.
//
//    // 2) Build a 3D checker: divide each coordinate into [0..checkerCount) cells
//    const float checkerCount = 8.0;
//    float fx = floor((pLocal.x + 1.0) * 0.5 * checkerCount);
//    float fy = floor((pLocal.y + 1.0) * 0.5 * checkerCount);
//    float fz = floor((pLocal.z + 1.0) * 0.5 * checkerCount);
//
//    // 3) Parity bit: if (fx+fy+fz) is even, one color; if odd, the other.
//    float parity = mod(fx + fy + fz, 2.0);
//    vec3 colorEven = vec3(0.9, 0.9, 0.9); // pale
//    vec3 colorOdd  = vec3(0.2, 0.2, 0.2); // dark
//    vec3 baseColor = mix(colorEven, colorOdd, parity);
//
//    return vec4(baseColor, 1.0);
//}
// Rendering mode 1: Checker pattern debug
vec4 renderCheckerDebug(vec3 hitPosition) {
        // 1) Transform into unit-sphere 'param space'
    vec3 pLocal = (hitPosition - clippingVolumePosition);
//    vec3 pLocal = (hitPosition - clippingVolumePosition) / clippingVolumeScale;
    // Now pLocal should be 1 on the surface.

    // 2) Build a 3D checker: divide each coordinate into [0..checkerCount) cells
    const float checkerCount = 40.0;
    float fx = floor((pLocal.x + 1.0) * 0.5 * checkerCount);
    float fy = floor((pLocal.y + 1.0) * 0.5 * checkerCount);
    float fz = floor((pLocal.z + 1.0) * 0.5 * checkerCount);

    // 3) Parity bit: if (fx+fy+fz) is even, one color; if odd, the other.
    float parity = mod(fx + fy + fz, 2.0);
    vec3 colorEven = vec3(0.9, 0.9, 0.9); // pale
    vec3 colorOdd  = vec3(0.2, 0.2, 0.2); // dark
    vec3 baseColor = mix(colorEven, colorOdd, parity);

    return vec4(baseColor, 1.0);
}


// Rendering mode 0: Shaded with lighting
vec4 renderShaded(vec3 hitPosition) {
    // Compute analytic normal (inward-facing)
    vec3 normal = ellipsoidNormal(hitPosition, clippingVolumePosition, clippingVolumeScale);
    vec3 toLight = normalize(ellipsoidLightPosition - hitPosition);
    
    float NdotL = max(dot(normal, toLight), 0.0);
    
    // Optional: add ambient term
    float ambient = 0.1;
    float lighting = ambient + (1.0 - ambient) * NdotL;
    
    // Example base color (choose what fits your effect)
    vec3 baseColor = vec3(0.56, 0.2, 0.2);
    // Final color
    vec3 shaded = baseColor * lighting * ellipsoidLightColor;
    
    return vec4(shaded, 1.0);
}

// Rendering mode 1: Checker pattern debug
//vec4 renderCheckerDebug(vec3 hitPosition) {
//    return debugEllipsoidColor(hitPosition, clippingVolumePosition, clippingVolumeScale);
//}

// Rendering mode 2: Ray iterations heatmap
vec4 renderRayHeatmap(int steps) {
    vec3 heatmapColor;
    if (steps <= 5) {
        // Blue: 1-5 steps (very efficient)
        heatmapColor = vec3(0.0, 0.0, 1.0);
    } else if (steps <= 10) {
        // Blue to Cyan: 6-10 steps
        float t = (steps - 5) / 5.0;
        heatmapColor = mix(vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 1.0), t);
    } else if (steps <= 20) {
        // Cyan to Green: 11-20 steps
        float t = (steps - 10) / 10.0;
        heatmapColor = mix(vec3(0.0, 1.0, 1.0), vec3(0.0, 1.0, 0.0), t);
    } else if (steps <= 40) {
        // Green to Yellow: 21-40 steps
        float t = (steps - 20) / 20.0;
        heatmapColor = mix(vec3(0.0, 1.0, 0.0), vec3(1.0, 1.0, 0.0), t);
    } else if (steps <= 80) {
        // Yellow to Red: 41-80 steps
        float t = (steps - 40) / 40.0;
        heatmapColor = mix(vec3(1.0, 1.0, 0.0), vec3(1.0, 0.0, 0.0), t);
    } else {
        // Pure Red: 81+ steps (inefficient)
        heatmapColor = vec3(1.0, 0.0, 0.0);
    }
    
    return vec4(heatmapColor, 1.0);
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

    ivec4 boneIndices = ivec4(fragBoneIds);
    vec4 boneWeights = fragBoneWeights;
    // Transform ray direction to T-pose space using inverse rotor
    vec3 transformedRayDir = rayDirVertex;


    vec3 currentRayOrigin = rayOrigin;
    vec3 currentRayDir = transformedRayDir;
    ivec4 currentBoneIndices = boneIndices;
    vec4 currentBoneWeights = boneWeights;

    bool hitEllipsoidBorder = false;
    int maxSteps = int(max(max(clippingVolumeScale.x,clippingVolumeScale.y),clippingVolumeScale.z) * 100.0)*3;
    const float minStep = 0.00001;

    float stepdepth = 0.0;
    float depth = 0.0;
    int steps = 0;
//    for (int i = 0; i < maxSteps; ++i) {
//        vec3 pos = rayOrigin + transformedRayDir * depth;
//        float ellipsoidDist = ellipsoidSDF(pos, clippingVolumePosition, clippingVolumeScale);
//
//        const float EPSILON = 0.00001;
//        const float MAX_DIST = 200.;
//        if (ellipsoidDist >= 0) 
//        {
//            hitEllipsoidBorder = true;
//            break;
//        }
//        depth += max(abs(ellipsoidDist),minStep);
//        steps++;
//
//        if(depth >= MAX_DIST) break;
//    }
    for (int i = 0; i < maxSteps; ++i) {
        // Position now advances along currentRayDir, which can change per step
        vec3 pos = currentRayOrigin + currentRayDir * stepdepth;
         currentRayOrigin = pos; // Uncomment if ray origin moves with ray for curved marching
        float ellipsoidDist = ellipsoidSDF(pos, clippingVolumePosition, clippingVolumeScale);
    
        const float EPSILON = 0.00001;
        const float MAX_DIST = 200.0;
        if (ellipsoidDist >= 0) {
            hitEllipsoidBorder = true;
            break;
        }
        stepdepth = min(max(abs(ellipsoidDist), 0.001),0.03);
        stepdepth = 0.01;
        depth += stepdepth;
        steps++;
        if(depth >= MAX_DIST) break;
        if(i == 0) continue;
    
        // ==== Sample new bone indices and weights from textures at this position ====
        vec3 texCoord = worldToSDFCoords(pos);
        vec3 texCoordWeight = worldToSDFCoordsWeight(pos);
        vec4 boneWeightsTex = texture(bindPose3DTextureBoneWeight, texCoordWeight);
        vec4 boneIndicesTexRaw = texture(bindPose3DTextureBoneIndex, texCoordWeight);
        ivec4 boneIndicesTex = ivec4(boneIndicesTexRaw * 255.0 + 0.5); // Assuming indices are encoded in [0,1] range

        if(length(boneIndicesTexRaw) == 0.0){
        float sampledDepth = texture(bindPose3DTextureSDF, worldToSDFCoords(pos)).r;

        if(sampledDepth > 0.500){
        hitEllipsoidBorder = true;
        break;
        }
        }
        for(int fix;fix<4;fix++)
            boneIndicesTex[fix] = (boneIndicesTex[fix] > 254) ? 0 : boneIndicesTex[fix]; 
        // ==== Re-transform direction for next step ====
        currentBoneIndices = boneIndicesTex;
        currentBoneWeights = boneWeightsTex;
        currentRayDir = TransformRayDirInverseSkinning(currentBoneIndices, currentBoneWeights, rayDir);
//    currentRayDir = rayDirVertex;
        // ==== Optionally, update origin for non-linear rays (if using curved paths) ====
    }


    if (hitEllipsoidBorder) {
        vec3 hitPosition = currentRayOrigin;

        // Sample the SDF texture at the hit position (already in T-pose space)
        float sampledDepth = texture(bindPose3DTextureSDF, worldToSDFCoords(hitPosition)).r;

        if(sampledDepth > 0.500)
            discard;

        // Choose rendering mode based on uniform
        if (renderingMode == 0) {
            // Mode 0: Shaded with lighting
            finalColor = renderShaded(hitPosition);
        } else if (renderingMode == 1) {
            // Mode 1: Checker pattern debug
            finalColor = renderCheckerDebug(hitPosition);
        } else if (renderingMode == 2) {
            // Mode 2: Ray iterations heatmap
            finalColor = renderRayHeatmap(steps);
        } else {
            // Fallback to shaded mode
            finalColor = renderShaded(hitPosition);
        }
    } else {
        // Ray didn't hit anything definitive - fallback
        finalColor = vec4(1.0, 0.0, 1.0, 1.0); // pink fallback
        discard;
    }
}