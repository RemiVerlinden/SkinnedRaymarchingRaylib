#include "ResourceManager.h"
#include "demo_utils.h"
#include <rlgl.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION            330
#else   // PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION            100
#endif

void DQ::ResourceManager::LoadAllShaders()
{
	// Load skinning shader
	Shader skinningShader = LoadShader(TextFormat("shaders/glsl%i/LinearBlendSkinning.vs", GLSL_VERSION),
		TextFormat("shaders/glsl%i/LinearBlendSkinning.fs", GLSL_VERSION));

	Shader DQSkinningShader = LoadShader(TextFormat("shaders/glsl%i/DualQuaternionBlendSkinning.vs", GLSL_VERSION),
		TextFormat("shaders/glsl%i/DualQuaternionBlendSkinning.fs", GLSL_VERSION));

	Shader RaymarchingShader = LoadShader(TextFormat("shaders/glsl%i/raymarching.vert", GLSL_VERSION),
		TextFormat("shaders/glsl%i/raymarching.frag", GLSL_VERSION));

	m_Shaders.push_back(skinningShader);
	m_Shaders.push_back(DQSkinningShader);
	m_Shaders.push_back(RaymarchingShader);
}

void DQ::ResourceManager::LoadModel(const char* fileName)
{
	CombinedModelData modelData;
	int& animationCount = modelData.animcount;
	modelData.model = DQ::LoadModel(fileName);
	modelData.pAnimations = LoadModelAnimations(fileName, &animationCount);

	m_ModelData = modelData;
}
// Load 3D texture using raylib-style pipeline (copied and adapted from LoadTexture -> LoadTextureFromImage -> rlLoadTexture)
void DQ::ResourceManager::LoadTextureSDF(const char* fileName)
{
    m_TextureBindPoseSDF = Load3DTextureSDF(fileName);
    
    if(m_TextureBindPoseSDF.id < 0)
    {
        TRACELOG(LOG_ERROR, "Failed to load EXR SDF texture: %s", fileName);
    }
}

std::vector<Shader> const& DQ::ResourceManager::GetShaders()
{
	return m_Shaders;
}

DQ::CombinedModelData& DQ::ResourceManager::GetModelData()
{
	return m_ModelData;
}

Texture DQ::ResourceManager::GetTextureSDF() const
{
	return m_TextureBindPoseSDF;
}

void DQ::ResourceManager::Shutdown()
{
	// needs rework, if we dont load the texture we unload anyways
	{
		::UnloadTexture(m_TextureBindPoseSDF);
	}
	{
		auto& it = m_ModelData;

		Model& model = it.model;
		ModelAnimation* pAnimations = it.pAnimations;
		int& animationCount = it.animcount;

		UnloadModelAnimations(pAnimations, animationCount);
		DQ::UnloadModel(model);
	}
}
