#include "glfw_util.h"

#include "raylib.h"
#include <GLFW\glfw3.h>

void DQ::SetCustomCursor()
{
	Image cursor = LoadImage("textures/cursor/arrow.png");
	if (cursor.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8)
	{
		ImageFormat(&cursor, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
	}
	GLFWimage image;
	image.width = cursor.width;
	image.height = cursor.height;
	image.pixels = reinterpret_cast<unsigned char*>(cursor.data);
	GLFWcursor* glfwCursor = glfwCreateCursor(&image, 0, 0); // xhot/yhot: the "hotspot"
	glfwSetCursor(glfwGetCurrentContext(), glfwCursor);

	UnloadImage(cursor);
}

// do I need to unload glfwCursor manually on destroy?