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

float ellipsoidSDF(vec3 pos, vec3 center, vec3 radii) {
    return length((pos - center) / radii) - 1.0;
}

// Remap world position to SDF texture coordinates (0-1 range)
vec3 worldToSDFCoords(vec3 worldPos) {
    return (worldPos - minBounds3DTextureSDF) / (maxBounds3DTextureSDF - minBounds3DTextureSDF);
}

void main() {
    // SDF: <0 inside ellipsoid
    float woundSDF = ellipsoidSDF(fragWorldPosition, clippingVolumePosition, clippingVolumeScale);
    
    if (woundSDF >= 0.0) {
        // Outside the wound: render as normal
        vec4 texelColor = texture(texture0, fragTexCoord);
        finalColor = texelColor * colDiffuse * fragColor;
        return;
    }

    // Inside the wound: raymarch through ellipsoid and mesh SDF
    vec3 rayOrigin = fragWorldPosition;
    vec3 rayDir = normalize(fragWorldPosition - cameraPosition);

    float t = 0.0;
    float maxDist = 5.0 * max(clippingVolumeScale.x, max(clippingVolumeScale.y, clippingVolumeScale.z));
    bool hitEllipsoidBorder = false;
    bool exitedMesh = false;
    const int maxSteps = 128;
    const float minStep = 0.001;

    for (int i = 0; i < maxSteps; ++i) {
        vec3 pos = rayOrigin + rayDir * t;
        float ellipsoidDist = ellipsoidSDF(pos, clippingVolumePosition, clippingVolumeScale);
        
        // Sample mesh SDF at current position
        vec3 sdfCoords = worldToSDFCoords(pos);
        
        // Check if we're still within SDF texture bounds
        if (sdfCoords.x >= 0.0 && sdfCoords.x <= 1.0 && 
            sdfCoords.y >= 0.0 && sdfCoords.y <= 1.0 && 
            sdfCoords.z >= 0.0 && sdfCoords.z <= 1.0) {
            
            float meshSDF = texture(bindPose3DTextureSDF, sdfCoords).r;
            
            // If mesh SDF > 0.5, we've exited the mesh
            if (meshSDF > 0.5) {
                exitedMesh = true;
                break;
            }
        }
        
        // Check if we've hit the ellipsoid border
        if (ellipsoidDist > 0.0) {
            hitEllipsoidBorder = true;
            break;
        }
        
        // Step forward (use smaller steps for better quality)
        t += max(abs(ellipsoidDist) * 0.5, minStep);
        if (t > maxDist) break;
    }

    if (exitedMesh) {
        // Ray exited mesh before hitting ellipsoid border - render wound hole (transparent/discard)
        discard;
    } else if (hitEllipsoidBorder) {
        // Ray hit ellipsoid border while still inside mesh - render wound surface
        finalColor = vec4(0.8, 0.1, 0.1, 1.0); // Red wound color
    } else {
        // Ray didn't hit anything definitive - fallback
        finalColor = vec4(1,0,1, 1.0); // pink error color
    }
}
