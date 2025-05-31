#include "demo_utils.h"
#include "raylib.h"
#include "raymath.h"
#include <cstring>
#include <klein\klein.hpp>

// HELPER FUNCTIONS FOR THIS CPP COMP UNIT
//------------------------------------------------------------------------------------------

__m128 quaternion_to_m128(const Quaternion& q) noexcept
{
    return _mm_set_ps(q.z, q.y, q.x, q.w);
}

kln::motor RaylibTransformToBoneTransform(const Transform& t) noexcept
{
    kln::rotor rotor(quaternion_to_m128(t.rotation));

    Vector4 position = { 0,t.translation.x, t.translation.y, t.translation.z };
    position /= 2;
    kln::translator translator{};
    translator.load_normalized(reinterpret_cast<float*>(&position));
    return rotor * translator;
}


//------------------------------------------------------------------------------------------

Camera DQ::GetCamera()
{
	Camera camera = { 0 };

	camera.position = Vector3{ 2.0f, 1.5f, 2.0f };	// Camera position
	camera.target = Vector3{ 0.0f, 0.75f, 0.0f };	// Camera looking at point
	camera.up = Vector3{ 0.0f, 1.f, 0.0f };		// Camera up vector (rotation towards target)
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
            for (int boneId = 0; boneId < anim.boneCount; boneId++)
            {
                Transform& bindTransform = model.bindPose[boneId];
                Transform& targetTransform = anim.framePoses[frame][boneId];

                // ----------- TRANSFORM SPACE EXPLANATION -----------
                // To drive the mesh with animation, we need to transform vertices from their **bind pose**
                // (the default, unanimated pose stored in the model) to the **current animated pose**.
                //
                // Visually:
                //      [bind pose] -----(animation delta)-----> [animated pose]
                //
                // In matrix math, the "animation delta" is:
                //      deltaM = inverse(bindMatrix) * targetMatrix
                //      (think: "undo bind pose, then apply animation pose")
                //
                // For dual quaternions (DQ), to rotate/transform a point 'p':
                //      DQ * p * ~DQ   // Standard sandwich product
                //
                // However, due to our GLSL shader's conventions (klein library), the built-in DQ sandwich function
                // applies DQ * p * ~DQ, which gives a mirrored/inverted rotation compared to our intended direction.
                //
                // To correct for this, we invert the target dual quaternion BEFORE composing it with the bind pose DQ,
                // so that the resulting DQ is correct for the transformation we want:
                //
                //      [Correct way for our pipeline]:
                //      bindToAnimationPoseDQ = targetDQ.inverted() * bindDQ
                //      (Undo target pose, then apply bind pose, for correct relative orientation)
                //
                // This is visually akin to aligning the bone from its original rest pose to its new animated orientation.

                kln::motor bindDQ = RaylibTransformToBoneTransform(bindTransform);
                kln::motor targetDQ = RaylibTransformToBoneTransform(targetTransform);
                targetDQ.invert();

                // Compute the dual quaternion that transforms from bind pose to current animation pose for this bone.
                kln::motor bindToAnimationPoseDQ = targetDQ * bindDQ;

                // 'constrain()' is necessary to keep it rotating using the shortest arc, this is very important to fix artifacts from doing targetDQ * bindDQ
                bindToAnimationPoseDQ.normalize();
                bindToAnimationPoseDQ.constrain();

                static_cast<kln::motor*>(model.meshes[firstMeshWithBones].boneMotors)[boneId] = bindToAnimationPoseDQ;
            }


            // Update remaining meshes with bones
            // NOTE: Using deep copy because shallow copy results in double free with 'UnloadModel()'
            for (int i = firstMeshWithBones + 1; i < model.meshCount; i++)
            {
                if (model.meshes[i].boneMatrices)
                {
                    memcpy(model.meshes[i].boneMotors,
                        model.meshes[firstMeshWithBones].boneMotors,
                        model.meshes[i].boneCount * sizeof(kln::motor));
                }
            }
        }
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
        model.meshes[meshIndex].boneMotors = new kln::motor[model.meshes[meshIndex].boneCount];
    }
	return model;
}

// Wrapper around default functionality, just inlclude deletion of our own extended DualQuaternion bone transforms
void DQ::UnloadModel(Model model)
{
		for (unsigned int meshIndex = 0; meshIndex < model.meshCount; meshIndex++)
			delete[] static_cast<kln::motor*>(model.meshes[meshIndex].boneMotors);

        ::UnloadModel(model);
}
