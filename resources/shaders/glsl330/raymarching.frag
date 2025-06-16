#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragWorldPosition;

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

//float ellipsoidSDF(vec3 pos, vec3 center, vec3 radii) {
//    return length((pos - center) / radii) - 1.0;
//}

//float ellipsoidSDF( vec3 pos, vec3 center, vec3 r )
//{
//    vec3 p = pos - center;
//    float k1 = length(p/r);
//    float k2 = length(p/(r*r));
//    return k1*(k1-1.0)/k2;
//}

vec3 ellipsoidNormal(vec3 pos, vec3 center, vec3 radii) {
    return -normalize((pos - center) / (radii * radii)); // negative for interior surface
}


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
    vec3 toLight = normalize(vec3(1,1,1) - hitPosition);
    
    float NdotL = max(dot(normal, toLight), 0.0);
    
    // Optional: add ambient term
    float ambient = 0.1;
    float lighting = ambient + (1.0 - ambient) * NdotL;
    
    // Example base color (choose what fits your effect)
    vec3 baseColor = vec3(0.56, 0.2, 0.2);
    // Final color
    vec3 shaded = renderCheckerDebug(hitPosition).xyz * lighting;
    
    return vec4(shaded, 1.0);
}


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

void main() {
    // SDF: <0 inside ellipsoid
    float woundSDF = ellipsoidSDF(fragWorldPosition, clippingVolumePosition, clippingVolumeScale);
    
    if (woundSDF > 0.0) {
        // Outside the wound: render as normal
        vec4 texelColor = texture(texture0, fragTexCoord);
        finalColor = texelColor * colDiffuse * fragColor;
        return;
    }

    // Inside the wound: raymarch through ellipsoid and mesh SDF
    vec3 rayOrigin = fragWorldPosition;
    vec3 rayDir = normalize(fragWorldPosition - cameraPosition);

    bool hitEllipsoidBorder = false;
    const int maxSteps = 128;
    const float minStep = 0.001;

    float depth = 0.0;

    for (int i = 0; i < maxSteps; ++i) {
        vec3 pos = rayOrigin + rayDir * depth;
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
        vec3 hitPosition =  rayOrigin + rayDir * depth;
        float sampledDepth = vec4(texture(bindPose3DTextureSDF, worldToSDFCoords(hitPosition)).xyz, 1.0).r; // Red wound color


        if(sampledDepth > 0.50){
                            finalColor = vec4(sampledDepth,sampledDepth,sampledDepth, 1.0);
//         return;
         discard;
        }
        // Ray hit ellipsoid border while still inside mesh - render wound surface
        finalColor = vec4(0.8 * 2 * sampledDepth, 0.1, 0.1, 1.0); // Red wound color
        finalColor = renderShaded(hitPosition);
    } else {
        // Ray didn't hit anything definitive - fallback
        finalColor = vec4(1,0,1, 1.0); // pink error color
        discard;

    }
}
