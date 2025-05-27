#version 330

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
uniform vec3 clippingVolumePosition;

// Output vertex attributes (to fragment shader)
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;
out vec3 fragWorldPosition;

void main()
{
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;

    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal,0)));

    gl_Position = mvp*vec4(vertexPosition,1);
    fragWorldPosition = vertexPosition; // In model space
}