#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Ray
{
	glm::vec3 Cast(float t) const
	{
		return origin + direction * t;
	}

	glm::vec3 origin;
	glm::vec3 direction;
};

struct HitPayload
{
	glm::vec3 position;
	glm::vec3 normal;
	float hitDistance;
	uint32_t objectIndex;
};

class HittableI
{
public:
	virtual HitPayload Hit(const Ray &ray, float t_min, float t_max) const = 0;
};

// class Sphere : public HittableI
struct Sphere
{
	glm::vec3 center;
	float radius;
};

struct Scene
{
	static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

	std::vector<Sphere> spheres;
};
