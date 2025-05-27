#version 330

in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragWorldPosition;

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

uniform vec3 clippingVolumePosition;
uniform vec3 clippingVolumeScale;
uniform vec3 cameraPosition;

float ellipsoidSDF(vec3 pos, vec3 center, vec3 radii) {
    return length((pos - center) / radii) - 1.0;
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

    // Inside the wound: raymarch through ellipsoid SDF
    vec3 rayOrigin = cameraPosition;
    vec3 rayDir = normalize(fragWorldPosition - cameraPosition);

    float t = 0.0;
    float maxDist = 5.0 * max(clippingVolumeScale.x, max(clippingVolumeScale.y, clippingVolumeScale.z)); // heuristic
    bool hit = false;
    const int maxSteps = 64;
    const float minStep = 0.001;

    for (int i = 0; i < maxSteps; ++i) {
        vec3 pos = rayOrigin + rayDir * t;
        float d = ellipsoidSDF(pos, clippingVolumePosition, clippingVolumeScale);

        if (d > 0.0) {
            // We have exited the ellipsoid; this is the wound "surface"
            hit = true;
            break;
        }
        t += max(d, minStep);
        if (t > maxDist) break;
    }

    if (hit) {
        finalColor = vec4(1.0, 0.0, 0.0, 1.0); // Fullbright red
    } else {
        discard; // Ray missed ellipsoid entirely (shouldn't happen for wound pixels)
    }
}
