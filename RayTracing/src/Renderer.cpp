#include "Renderer.h"

#include <iostream>

namespace
{

__forceinline uint32_t ConvertToRGBA8(glm::vec4 vec4Color)
{
	vec4Color = glm::clamp(std::move(vec4Color), glm::vec4{ 0.0f }, glm::vec4{ 1.0f });

	return (static_cast<uint8_t>(vec4Color.w * 255.0f) << 24) |
		(static_cast<uint8_t>(vec4Color.z * 255.0f) << 16) |
		(static_cast<uint8_t>(vec4Color.y * 255.0f) << 8) |
		(static_cast<uint8_t>(vec4Color.x * 255.0f));
}

__forceinline uint32_t ConvertToRGBA8(glm::vec3 vec3Color)
{
	return ConvertToRGBA8(glm::vec4{ std::move(vec3Color) , 1.0f});
}

}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_pFinalImage && m_pFinalImage->GetWidth() == width && m_pFinalImage->GetHeight() == height)
	{
		return;
	}

	if (m_pFinalImage)
	{
		m_pFinalImage->Resize(width, height);
	}
	else
	{
		m_pFinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_pFinalImageData;
	m_pFinalImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene &scene, const Camera &camera)
{
	m_pCamera = &camera;
	m_pScene = &scene;
	assert(m_pCamera && m_pScene && m_pFinalImage);

	const uint32_t width = m_pFinalImage->GetWidth();
	const uint32_t height = m_pFinalImage->GetHeight();

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			const size_t index = x + y * width;

			glm::vec4 color = GenRay(x, y);
			m_pFinalImageData[index] = ConvertToRGBA8( std::move(color) );
		}
	}

	m_pFinalImage->SetData(m_pFinalImageData);
}

glm::vec4 Renderer::GenRay(uint32_t x, uint32_t y)
{
	const size_t pixelIndex = x + y * m_pFinalImage->GetWidth();

	Ray ray;
	ray.origin = m_pCamera->GetPosition();
	ray.direction = m_pCamera->GetRayDirections()[pixelIndex];

	HitPayload payload = TraceRay(ray);

	if (payload.hitDistance < 0.0f)
	{
		// Sky color
		float tmp = 0.5 * (glm::normalize(ray.direction).y + 1.0f);
		glm::vec3 color = glm::vec3{ 1.0f - tmp } + glm::vec3{ 0.5f, 0.7f, 1.0f } * tmp;
		return glm::vec4{ color, 1.0f };
	}

	// const Sphere &sphere = m_pScene->spheres[payload.objectIndex];

	return glm::vec4{ payload.normal * 0.5f + glm::vec3{ 0.5 }, 1.0f };
}

HitPayload Renderer::TraceRay(const Ray &ray)
{
	uint32_t closestSphereIndex = Scene::InvalidIndex;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t index = 0; index < m_pScene->spheres.size(); ++index)
	{
		const Sphere &sphere = m_pScene->spheres[index];

		glm::vec3 origin = ray.origin - sphere.m_center;
		float a = glm::dot(ray.direction, ray.direction);
		float b_half = glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - sphere.m_radius * sphere.m_radius;

		float discriminant = b_half * b_half - a * c;
		if (discriminant < 0.0f)
		{
			continue;
		}

		float closestT = (-b_half - glm::sqrt(discriminant)) / a;
		if (closestT >= 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphereIndex = index;
		}
	}

	if (Scene::InvalidIndex == closestSphereIndex)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, hitDistance, closestSphereIndex);
}

HitPayload Renderer::ClosestHit(const Ray &ray, float hitDistance, uint32_t objectIndex)
{
	HitPayload payload;
	payload.objectIndex = objectIndex;
	payload.hitDistance = hitDistance;

	const Sphere &closestSphere = m_pScene->spheres[objectIndex];
	glm::vec3 origin = ray.origin - closestSphere.m_center;
	glm::vec3 hitPoint = origin + ray.direction * hitDistance;

	payload.position = hitPoint + closestSphere.m_center;
	payload.normal = glm::normalize(std::move(hitPoint));

	return payload;
}

HitPayload Renderer::Miss(const Ray &ray)
{
	HitPayload payload;
	payload.hitDistance = -1.0f;

	return payload;
}
