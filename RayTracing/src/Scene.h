#pragma once

#include <glm/glm.hpp>
#include <vector>

inline constexpr uint32_t INVALID_INDEX = std::numeric_limits<uint32_t>::max();

struct Ray
{
	glm::vec3 Cast(float t) const
	{
		return origin + direction * t;
	}

	glm::vec3 origin;
	glm::vec3 direction;
};

struct Material
{
	// 返回出射光线的方向。
	glm::vec3 Sample(const glm::vec3 &wi, const glm::vec3 &N) { return {}; };
	// 返回采样的 PDF。
	float PDF(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) { return {}; };
	// 返回 BRDF。
	glm::vec3 Eval(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) { return {}; };

	float SnellSchlick(float cos, float idx) const
	{
		float r0 = (1.0f - idx) / (1.0f + idx);
		r0 = r0 * r0;
		return r0 + (1.0f - r0) * pow((1.0f - cos), 5);
	}

	glm::vec3 albedo{ 1.0f, 1.0f , 1.0f };
	float roughness = 0.9f;
	float metallic = 0.1f;

	glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
	float emissiveIntensity = 0.0f;

	// TODO : Change it to float
	bool isTransparent = false;
	float eta = 1.0f;
};

struct HitPayload
{
	glm::vec3 position;
	glm::vec3 normal;
	float hitDistance;

	uint32_t objectIndex = INVALID_INDEX;
	uint32_t materialIndex = INVALID_INDEX;
};

class Hittable
{
public:
	virtual HitPayload Hit(const Ray &ray, float t_min, float t_max) const = 0;
};

// class Sphere : public HittableI
struct Sphere
{
	int materialIndex;
	
	glm::vec3 center;
	float radius;
};

struct Scene
{
	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};
