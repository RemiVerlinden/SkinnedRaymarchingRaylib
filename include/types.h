#ifndef TYPES_H
#define TYPES_H
#include <klein\klein.hpp>
#include <vector>

namespace DQ
{
	using BoneTransform = kln::motor;

	using MeshBones = std::vector<BoneTransform>;
}

#endif // !TYPES_H
