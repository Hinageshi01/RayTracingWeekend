#pragma once

#include <glm/glm.hpp>

struct Ray
{
	glm::vec3 Cast(float t) const
	{
		return origin + direction * t;
	}

	glm::vec3 origin;
	glm::vec3 direction;
};
