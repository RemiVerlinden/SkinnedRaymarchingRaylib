#include "demo_utils.h"
#include "raylib.h"
#include "types.h"
#include "raymath_typecast.h"
#include "globalvars.h"
#include "raymath.h"

extern GlobalVars gGlobals;

Camera DQ::GetCamera()
{
	Camera camera = { 0 };

	camera.position = Vector3{ 5.0f, 5.0f, 5.0f };	// Camera position
	camera.target = Vector3{ 0.0f, 2.0f, 0.0f };	// Camera looking at point
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };		// Camera up vector (rotation towards target)
	camera.fovy = 45.0f;                            // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

	return camera;
}

RLAPI void DQ::UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame)
{
	::UpdateModelAnimationBones(model, anim,frame);

    if ((anim.frameCount > 0) && (anim.bones != NULL) && (anim.framePoses != NULL))
    {
        if (frame >= anim.frameCount) frame = frame % anim.frameCount;

        // Get first mesh which have bones
        int firstMeshWithBones = -1;

        for (int i = 0; i < model.meshCount; i++)
        {
            if (model.meshes[i].boneMatrices)
            {
                if (firstMeshWithBones == -1)
                {
                    firstMeshWithBones = i;
                    break;
                }
            }
        }

        if (firstMeshWithBones != -1)
        {
            // Update all bones and boneMatrices of first mesh with bones.
            for (int boneId = 0; boneId < anim.boneCount; boneId++)
            {
                Transform& bindTransform = model.bindPose[boneId];
                Transform& targetTransform = anim.framePoses[frame][boneId];

                DQ::BoneTransform bindMotor = RaylibTransformToBoneTransform(bindTransform);
                DQ::BoneTransform targetMotor = RaylibTransformToBoneTransform(targetTransform);
                targetMotor.invert();


                DQ::BoneTransform finalMotor = targetMotor * bindMotor;
                finalMotor.normalize();
                static_cast<DQ::BoneTransform*>(model.meshes[firstMeshWithBones].boneMotors)[boneId] = finalMotor;
            }

            // Update remaining meshes with bones
            // NOTE: Using deep copy because shallow copy results in double free with 'UnloadModel()'
            for (int i = firstMeshWithBones + 1; i < model.meshCount; i++)
            {
                if (model.meshes[i].boneMatrices)
                {
                    memcpy(model.meshes[i].boneMotors,
                        model.meshes[firstMeshWithBones].boneMotors,
                        model.meshes[i].boneCount * sizeof(DQ::BoneTransform));
                }
            }
        }
    }
    if (gGlobals.toggleskinning)
    {
        int loc = GetShaderLocation(model.materials[1].shader, "boneQuats");
        SetShaderValueV(model.materials[1].shader, loc, model.meshes[0].boneMotors, SHADER_UNIFORM_VEC4, anim.boneCount * 2);
    }
}

void DQ::DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    ::DrawMesh(mesh, material, transform);
}

// This is just a wrapper that calls the original LoadModel, then additionally initialises a seperate array where we store the Dual Quaternion bone transforms 
// possibly unconventional implementation
Model DQ::LoadModel(const char* fileName)
{
	Model model = ::LoadModel(fileName);

    for (unsigned int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
    {
        model.meshes[meshIndex].boneMotors = new DQ::BoneTransform[model.meshes[meshIndex].boneCount];

        meshIndex++; // Move to next mesh
    }
	return model;
}

// Wrapper around default functionality, just inlclude deletion of our own extended DualQuaternion bone transforms
void DQ::UnloadModel(Model model)
{
		for (unsigned int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
			delete[] static_cast<DQ::BoneTransform*>(model.meshes[meshIndex].boneMotors);

        ::UnloadModel(model);
}
