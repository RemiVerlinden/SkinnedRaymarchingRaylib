#ifndef UTILS_H
#define UTILS_H
#include "raylib.h"

namespace DQ 
{
	Camera GetCamera();

	Model LoadModel(const char* fileName);
	void UnloadModel(Model model);
	RLAPI void UpdateModelAnimationBones(Model model, ModelAnimation anim, int frame);
	void DrawMesh(Mesh mesh, Material material, Matrix transform);
	void DrawModelWires(Model model, Vector3 position, float scale, Color tint);
}

#endif // !UTILS_H
