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

struct Material
{
	// 返回出射光线的方向。
	glm::vec3 Sample(const glm::vec3 &wi, const glm::vec3 &N) { return {}; };
	// 返回采样的 PDF。
	float PDF(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) { return {}; };
	// 返回 BRDF。
	glm::vec3 Eval(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N) { return {}; };

	glm::vec3 albedo{ 1.0f, 1.0f , 1.0f };
	float roughness = 0.9f;
	float metallic = 0.1f;
	glm::vec3 emissiveColor{ 0.0f, 0.0f, 0.0f };
	float emissiveIntensity = 0.0f;
};

struct HitPayload
{
	glm::vec3 position;
	glm::vec3 normal;
	float hitDistance;

	uint32_t objectIndex;
	uint32_t materialIndex;
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
	static constexpr uint32_t InvalidIndex = std::numeric_limits<uint32_t>::max();

	std::vector<Sphere> spheres;
	std::vector<Material> materials;
};
