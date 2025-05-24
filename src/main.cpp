/*******************************************************************************************
*
*   raylib [core] example - Doing skinning on the gpu using a vertex shader
*
*   Example complexity rating: [★★★☆] 3/4
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*   Example contributed by Daniel Holden (@orangeduck) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2024-2025 Daniel Holden (@orangeduck)
*
*   Note: Due to limitations in the Apple OpenGL driver, this feature does not work on MacOS
*
********************************************************************************************/


#include "common.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif


GlobalVars gGlobals;
ScreenInfo gScreenInfo;
SequenceInfo gAnimInfo;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	
	ChangeDirectory(RESOURCES_PATH);

	InitWindow(gScreenInfo.WIDTH, gScreenInfo.HEIGHT, "raylib [models] example - GPU skinning");


	// Define the camera to look into our 3d world
	Camera camera = DQ::GetCamera();

	// Load gltf model
	DQ::Model model;
	model.raylib = LoadModel("models/gltf/pirate/pirate.glb"); // Load character model

	Model& characterModel = model.raylib;
	
	// Load skinning shader
	Shader skinningShader = LoadShader(TextFormat("shaders/glsl%i/LinearBlendSkinning.vs", GLSL_VERSION),
		TextFormat("shaders/glsl%i/LinearBlendSkinning.fs", GLSL_VERSION));

	characterModel.materials[1].shader = skinningShader;

	// Load gltf model animations
	ModelAnimation* modelAnimations = LoadModelAnimations("models/gltf/pirate/pirate.glb", &gAnimInfo.total);

	Vector3 position = { 0.0f, 0.0f, 0.0f }; // Set model position

	DisableCursor();                    // Limit cursor to relative movement inside the window

	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())        // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------
		UpdateCamera(&camera, CAMERA_FREE);

		// Select current animation
		if (IsKeyPressed(KEY_T)) gAnimInfo.index = (gAnimInfo.index + 1) % gAnimInfo.total;
		else if (IsKeyPressed(KEY_G)) gAnimInfo.index = (gAnimInfo.index + gAnimInfo.total - 1) % gAnimInfo.total;
		
		if (IsKeyPressed(KEY_P)) gGlobals.paused = !gGlobals.paused;

		// Update model animation
		ModelAnimation anim = modelAnimations[gAnimInfo.index];
		unsigned int animCurrentFrame = gGlobals.frame % anim.frameCount;
		characterModel.transform = MatrixTranslate(position.x, position.y, position.z);
		characterModel.transform = MatrixScale(0.02, 0.02, 0.02);
		UpdateModelAnimationBones(characterModel, anim, animCurrentFrame);
		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);

		// Draw character mesh, pose calculation is done in shader (GPU skinning)
		DrawMesh(characterModel.meshes[0], characterModel.materials[1], characterModel.transform);

		DrawGrid(10, 1.0f);

		EndMode3D();

		DrawText("Use the T/G to switch animation", 10, 10, 20, GRAY);

		EndDrawing();
		//----------------------------------------------------------------------------------

		if(!gGlobals.paused)
			gGlobals.frame++;
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadModelAnimations(modelAnimations, gAnimInfo.total); // Unload model animation
	UnloadModel(characterModel);    // Unload model and meshes/material
	UnloadShader(skinningShader);   // Unload GPU skinning shader

	CloseWindow();                  // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}