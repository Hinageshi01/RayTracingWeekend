#pragma once

#include "Camera.h"

#include <glm/glm.hpp>
#include <memory>
#include <Walnut/Image.h>

struct Ray
{
	glm::vec3 at(float t) const
	{
		return origin + direction * t;
	}

	glm::vec3 origin;
	glm::vec3 direction;
};

struct HitPayload
{
	glm::vec3 color;
};

class Renderer
{
public:
	Renderer() = default;
	~Renderer() = default;

	void Render(const Camera &camera);
	void OnResize(uint32_t width, uint32_t height);

	std::shared_ptr<Walnut::Image> GetFinalImage() { return m_pFinalImage; }
	const std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_pFinalImage; }

private:
	glm::vec4 GenRay(uint32_t x, uint32_t y);
	HitPayload TraceRay(const Ray &ray);
	HitPayload ClosestHit(const Ray &ray, float hitDistance, uint32_t objectIndex);
	HitPayload Miss(const Ray &ray);

	const Camera *m_pCamera = nullptr;

	uint32_t *m_pFinalImageData = nullptr;
	std::shared_ptr<Walnut::Image> m_pFinalImage;
};
