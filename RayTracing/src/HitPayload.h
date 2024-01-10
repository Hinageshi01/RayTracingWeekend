#pragma once

#include <glm/glm.hpp>

struct HitPayload
{
	void SetFaceNormal(const glm::vec3 rayDirection, const glm::vec3 &outwardNormal)
	{
		isFront = glm::dot(rayDirection, outwardNormal) < 0.0f;
		normal = isFront ? outwardNormal : -outwardNormal;
	}

	glm::vec3 position;
	glm::vec3 normal;

	float hitDistance;
	uint32_t objectIndex;
	bool isFront;
};
