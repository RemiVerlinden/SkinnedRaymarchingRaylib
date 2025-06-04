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

	Shader StaticRaymarchingShader = LoadShader(TextFormat("shaders/glsl%i/raymarching.vert", GLSL_VERSION),
		TextFormat("shaders/glsl%i/raymarching.frag", GLSL_VERSION));

    Shader SkinnedRaymarchingShader = LoadShader(TextFormat("shaders/glsl%i/skinnedraymarching.vert", GLSL_VERSION),
        TextFormat("shaders/glsl%i/skinnedraymarching.frag", GLSL_VERSION));

	m_Shaders.push_back(skinningShader);
	m_Shaders.push_back(DQSkinningShader);
    m_Shaders.push_back(StaticRaymarchingShader);
    m_Shaders.push_back(SkinnedRaymarchingShader);
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

void DQ::ResourceManager::LoadTextureWeight(const char* fileNameWeightData, const char* fileNameWeightIndex)
{
    // They are both 4 channel textures with data, they can both be loaded in same manner :)
    m_TextureWeightData = Load3DTextureWeight(fileNameWeightData);
    m_TextureWeightIndex = Load3DTextureWeight(fileNameWeightIndex);

    if (m_TextureWeightData.id < 0)
        TRACELOG(LOG_ERROR, "Failed to load EXR weight DATA texture: %s", fileNameWeightData);
    if (m_TextureWeightIndex.id < 0)
        TRACELOG(LOG_ERROR, "Failed to load EXR weight INDEX texture: %s", fileNameWeightIndex);
}

void DQ::ResourceManager::DebugSaveImageData(const char* fileName, const char* outputFileName)
{
    Image loadedImage = LoadImage(fileName);
    
    if (loadedImage.data == NULL)
    {
        TRACELOG(LOG_ERROR, "Failed to load image for debug: %s", fileName);
        return;
    }
    
    TRACELOG(LOG_INFO, "Debug: Loaded image %dx%d, format: %d, mipmaps: %d", 
             loadedImage.width, loadedImage.height, loadedImage.format, loadedImage.mipmaps);
    
    // Convert to a simple format for saving
    Image debugImage = { 0 };
    
    //if (loadedImage.format == PIXELFORMAT_UNCOMPRESSED_R32G32B32A32)
    //{
    //    // Convert from 32-bit float RGBA to 8-bit grayscale (using R channel)
    //    debugImage.width = loadedImage.width;
    //    debugImage.height = loadedImage.height;
    //    debugImage.mipmaps = 1;
    //    debugImage.format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    //    
    //    int pixelCount = loadedImage.width * loadedImage.height;
    //    debugImage.data = (unsigned char*)malloc(pixelCount);
    //    
    //    float* sourceData = (float*)loadedImage.data;
    //    unsigned char* destData = (unsigned char*)debugImage.data;
    //    
    //    for (int i = 0; i < pixelCount; i++)
    //    {
    //        // Get R channel from RGBA float data and convert to 0-255 range
    //        float value = sourceData[i * 4]; // R channel
    //        
    //        // Clamp and convert to byte
    //        if (value < 0.0f) value = 0.0f;
    //        if (value > 1.0f) value = 1.0f;
    //        
    //        destData[i] = (unsigned char)(value * 255.0f);
    //    }
    //    
    //    TRACELOG(LOG_INFO, "Debug: Converted to grayscale %dx%d", debugImage.width, debugImage.height);
    //}
    //else
    //{
    //    // For other formats, just copy the image
    //    debugImage = ImageCopy(loadedImage);
    //}
    
    // Export as PNG
    bool success = ExportImage(loadedImage, outputFileName);
    
    if (success)
    {
        TRACELOG(LOG_INFO, "Debug: Successfully saved image to %s", outputFileName);
    }
    else
    {
        TRACELOG(LOG_ERROR, "Debug: Failed to save image to %s", outputFileName);
    }
    
    UnloadImage(debugImage);
    UnloadImage(loadedImage);
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
        for (int i = 0; i < m_Shaders.size(); i++) UnloadShader(m_Shaders[i]);
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
