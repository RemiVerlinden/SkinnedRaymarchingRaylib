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
#include "DemoScene.h"

#include "structs.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

struct ScreenInfo
{
	const int WIDTH = 900;
	const int HEIGHT = 600;
};

ScreenInfo gScreenInfo;

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------

	ChangeDirectory(RESOURCES_PATH);

	InitWindow(gScreenInfo.WIDTH, gScreenInfo.HEIGHT, "raylib [models] example - GPU skinning");

	DisableCursor();                    // Limit cursor to relative movement inside the window
	SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second

	DQ::DemoScene scene{};
	scene.Init();

	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())        // Detect window close button or ESC key
	{
		{
			static DQ::UpdateContext context;

			scene.Update(context);
			scene.Draw();
		
		
			static bool pause;
			if (IsKeyPressed(KEY_P)) pause = !pause;

			if(!pause)	context.frame++;
		}
	}
	//----------------------------------------------------------------------------------

	// De-Initialization
	//--------------------------------------------------------------------------------------

	scene.Shutdown();
	CloseWindow();                  // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	return 0;
}