#ifndef COMMON_H
#define COMMON_H

#include <raygizmo.h>
#include <raylib.h>
#include <raymath.h>

#include <stdlib.h>
#include <stddef.h>

#ifndef RESOURCES_PATH
#	define RESOURCES_PATH "/"
#endif


/* === Helper functions === */

static inline Texture2D RES_LoadTexture(const char* fileName)
{
	Texture2D texture = LoadTexture(TextFormat("%s%s", RESOURCES_PATH, fileName));

	GenTextureMipmaps(&texture);
	SetTextureFilter(texture, TEXTURE_FILTER_ANISOTROPIC_4X);

	return texture;
}

static inline Model RES_LoadModel(const char* fileName)
{
	return LoadModel(TextFormat("%s%s", RESOURCES_PATH, fileName));
}

#endif // COMMON_H
