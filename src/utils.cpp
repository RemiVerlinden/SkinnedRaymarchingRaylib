#include "utils.h"

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
