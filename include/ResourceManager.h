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
        RAYMARCHING,
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

    public:
        void LoadAllShaders();
        void LoadModel(const char* fileName);
        void LoadTextureSDF(const char* fileName);
        void DebugSaveImageData(const char* fileName, const char* outputFileName);

        std::vector<Shader> const& GetShaders();
        CombinedModelData& GetModelData();
        Texture GetTextureSDF() const;

        void Shutdown();
    private:

        CombinedModelData m_ModelData;
        std::vector<Shader> m_Shaders;
        Texture m_TextureBindPoseSDF; // 3D texture stored as raylib Texture struct - need rework
    };
}

#endif // !RESOURCEMANAGER_H
