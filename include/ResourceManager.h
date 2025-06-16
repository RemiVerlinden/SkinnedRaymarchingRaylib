#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "raylib.h"
#include "structs.h"
#include <vector>
#include <unordered_map>

namespace DQ {

    typedef enum
    {
        LINEARBLENDSKINNING = 0,
        DUALQUATERNIONBLENDSKINNING,
        STATICRAYMARCHING,
        SKINNEDRAYMARCHING,
        SKINNEDRAYMARCHINGNONLINEAR,
        TOTALTYPES
        //FX_FXAA
    } ShaderTypes;

    struct CombinedModelData
    {
        Model model;
        ModelAnimation* pAnimations;
        int animcount;
    };

    class ResourceManager final
    {

        void LoadAllShaders();
        void LoadLightbulbIcon();
    public:
        void init();
        void LoadModel(const char* fileName);
        void LoadTextureSDF(const char* fileName);
        void LoadTextureWeight(const char* fileNameBoneWeight, const char* fileNameBoneIndex);
        void DebugSaveImageData(const char* fileName, const char* outputFileName);

        std::vector<Shader> const& GetShaders();
        CombinedModelData& GetModelData();
        Texture GetTextureSDF() const;
        Texture GetTextureBoneWeight() const;
        Texture GetTextureBoneIndex() const;
        Texture2D GetLightbulbIcon() const;

        void Shutdown();
    private:

        CombinedModelData m_ModelData;
        std::vector<Shader> m_Shaders; // cant figure out why, but the fact that this is a vector is causing memory leaks???? if I use a static array, I get no problems.
        Texture m_TextureBindPoseSDF; // 3D texture stored as raylib Texture struct - need rework
        Texture m_TextureBoneWeight; // 3D texture with 4 channel weight data (volumetric 4 weight blend skinning)
        Texture m_TextureBoneIndex; // 3D texture with 4 channel weight index data
        Texture2D m_LightbulbIcon;
    };
}

#endif // !RESOURCEMANAGER_H
