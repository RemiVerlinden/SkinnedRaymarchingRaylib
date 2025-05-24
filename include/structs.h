#ifndef DEMOSTRUCTS
#define DEMOSTRUCTS
#include "types.h"

// Dual Quaternion
namespace DQ
{
    struct Model
    {
        ::Model raylib;
        std::vector<MeshBones> submeshes;
    };
}

#endif // !DEMOSTRUCTS
