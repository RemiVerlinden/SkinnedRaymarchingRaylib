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

        if(sampledDepth > 0.5025)
        discard;
        // Ray hit ellipsoid border while still inside mesh - render wound surface
        finalColor = vec4(0.8, 0.1, 0.1, 1.0); // Red wound color
    } else {
        // Ray didn't hit anything definitive - fallback
        finalColor = vec4(1,0,1, 1.0); // pink error color
    }
}
