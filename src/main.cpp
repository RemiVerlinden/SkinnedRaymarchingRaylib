/*******************************************************************************************
*
*   Doing skinning on the gpu using a vertex shader
*
*
*
*   Example originally created with raylib 4.5, last time updated with raylib 4.5
*
*
*
*
*
*
*
*
*   Note: This was built upon the raylib GPU skinning example project
*
********************************************************************************************/


#include "common.h"
#include "DemoScene.h"
#include "structs.h"

#include "glfw_util.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

struct ScreenInfo
{
	const int WIDTH = 1024;
	const int HEIGHT = 1024;
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

	InitWindow(gScreenInfo.WIDTH, gScreenInfo.HEIGHT, "Raymarching skinned meshes DEMO");

	DQ::SetCustomCursor();

	SetConfigFlags(FLAG_MSAA_4X_HINT);

	DisableCursor();                    // Limit cursor to relative movement inside the window
	SetTargetFPS(60);

	DQ::DemoScene scene{};
	scene.Init();

	//--------------------------------------------------------------------------------------

	// Main game loop
	while (!WindowShouldClose())        // Detect window close button or ESC key
	{
		{
			static DQ::UpdateContext context;
			static bool pause = false;
			
			if (IsKeyPressed(KEY_C)) pause = !pause;
			
			context.timestep = GetFrameTime();
			context.resetAnimation = IsKeyPressed(KEY_X);
			
			if (!pause)
			{
				context.frame++;
				// Animation runs at 60 FPS equivalent regardless of actual framerate
				context.animationTime += context.timestep * 60.0f;
			}
			
			if (context.resetAnimation)
			{
				context.animationTime = 0.0f;
			}

			scene.Update(context);
			scene.Draw(context);
		}
	}
	//----------------------------------------------------------------------------------

	// De-Initialization
	//--------------------------------------------------------------------------------------
	
	scene.Shutdown();
	CloseWindow();                  // Close window and OpenGL context
	//--------------------------------------------------------------------------------------

	_CrtDumpMemoryLeaks();
	return 0;
}