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

	InitWindow(gScreenInfo.WIDTH, gScreenInfo.HEIGHT, "Raymarching skinned meshes DEMO");

	DQ::SetCustomCursor();

	SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)

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
			static bool pause = true;
			if (IsKeyPressed(KEY_C)) pause = !pause;
			if (!pause)
			{
				context.frame++;
				context.timestep = GetFrameTime();
			}


			scene.Update(context);
			scene.Draw();
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