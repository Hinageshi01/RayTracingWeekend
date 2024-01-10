#pragma once

#include "Sphere.h"

#include <glm/glm.hpp>
#include <vector>

struct Scene
{
	static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

	glm::vec3 skyColor{ 1.0f };
	std::vector<Sphere> spheres;
};
