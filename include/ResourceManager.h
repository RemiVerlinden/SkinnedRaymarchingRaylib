#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include "raylib.h"
#include "structs.h"
#include <vector>

namespace DQ {

    typedef enum
    {
        LINEARBLENDSKINNING = 0,
        DUALQUATERNIONBLENDSKINNING,
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

        std::vector<Shader> const& GetShaders();
        CombinedModelData& GetModelData();

        void Shutdown();
    private:

        CombinedModelData m_ModelData;
        std::vector<Shader> m_Shaders;
    };
}

#endif // !RESOURCEMANAGER_H
