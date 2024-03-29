#include "Renderer.h"

#include <execution>
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

__forceinline glm::vec3 GammaCorrection(const glm::vec3 &color)
{
	constexpr float power = 1.0f / 2.2f;

	return glm::vec3{ std::pow(color.x, power),
		std::pow(color.y, power),
		std::pow(color.z, power) };
}

__forceinline glm::vec4 GammaCorrection(const glm::vec4 &color)
{
	return glm::vec4{ GammaCorrection(glm::vec3{color}), color.w };
}

__forceinline uint32_t PCG_Hash(uint32_t input)
{
	uint32_t state = input * 747796405u + 2891336453u;
	uint32_t word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

__forceinline float RandomFloat(uint32_t &seed)
{
	seed = PCG_Hash(seed);
	return (float)seed / (float)std::numeric_limits<uint32_t>::max();
}

__forceinline glm::vec3 RandomUnitSphere(uint32_t &seed)
{
	return glm::normalize(glm::vec3{
		RandomFloat(seed) * 2.0f - 1.0f,
		RandomFloat(seed) * 2.0f - 1.0f,
		RandomFloat(seed) * 2.0f - 1.0f });
}

__forceinline glm::vec3 RandomHemisphereSphere(uint32_t &seed, glm::vec3 normal)
{
	glm::vec3 unitSphere = RandomUnitSphere(seed);
	if (glm::dot(unitSphere, normal) > 0.0f)
	{
		return unitSphere;
	}
	else
	{
		return -unitSphere;
	}
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

	delete[] m_pAccumulateData;
	m_pAccumulateData = new glm::vec4[width * height];

	m_frameIndex = 1;
	std::memset(m_pAccumulateData, 0, width * height * sizeof(glm::vec4));

	m_imageHorizontalIter.resize(width);
	for (uint32_t i = 0; i < width; ++i)
	{
		m_imageHorizontalIter[i] = i;
	}
	m_imageVerticallIter.resize(height);
	for (uint32_t i = 0; i < height; ++i)
	{
		m_imageVerticallIter[i] = i;
	}
}

void Renderer::ResetAccumulate()
{
	m_frameIndex = 1;
	std::memset(m_pAccumulateData, 0, m_pFinalImage->GetWidth() * m_pFinalImage->GetHeight() * sizeof(glm::vec4));
}

void Renderer::Render(const Scene &scene, const Camera &camera)
{
	m_pCamera = &camera;
	m_pScene = &scene;
	assert(m_pCamera && m_pScene && m_pFinalImage);

	static bool s_lastFrameAccumulateState = m_isAccumulate;
	if (!m_isAccumulate || !s_lastFrameAccumulateState)
	{
		// Clear accumulate
		m_frameIndex = 1;
		std::memset(m_pAccumulateData, 0, m_pFinalImage->GetWidth() * m_pFinalImage->GetHeight() * sizeof(glm::vec4));
	}

#define PARALLEL 1

#if PARALLEL
	std::for_each(std::execution::par, m_imageVerticallIter.begin(), m_imageVerticallIter.end(),
		[this](uint32_t y)
		{
			const uint32_t imageWidth = m_pFinalImage->GetWidth();
			for (uint32_t x = 0; x < imageWidth; ++x)
			{
				const size_t index = x + y * imageWidth;
				m_pAccumulateData[index] += GammaCorrection(GenRay(x, y));
				m_pFinalImageData[index] = ConvertToRGBA8(m_pAccumulateData[index] / static_cast<float>(m_frameIndex));
			}
});
#else
	for (uint32_t y = 0; y < imageHeight; ++y)
	{
		for (uint32_t x = 0; x < imageWidth; ++x)
		{
			const size_t index = x + y * imageWidth;
			m_pAccumulateData[index] += PerPixel(x, y);
			m_pFinalImageData[index] = ConvertToRGBA8(m_pAccumulateData[index] / static_cast<float>(m_frameIndex));
		}
	}
#endif

	m_pFinalImage->SetData(m_pFinalImageData);

	if (m_isAccumulate)
	{
		++m_frameIndex;
	}

	s_lastFrameAccumulateState = m_isAccumulate;
}

glm::vec4 Renderer::GenRay(uint32_t x, uint32_t y)
{
	const size_t pixelIndex = x + y * m_pFinalImage->GetWidth();

	Ray ray;
	ray.origin = m_pCamera->GetPosition();
	ray.direction = m_pCamera->GetRayDirections()[pixelIndex];

	glm::vec3 light{ 0.0f };
	glm::vec3 contribution{ 1.0f };
	uint32_t seed = pixelIndex * m_frameIndex;
	for (uint32_t i = 0; i < m_bounces; ++i)
	{
		seed += i;

		HitPayload payload = TraceRay(ray);
		if (payload.hitDistance < 0.0f)
		{
			// Sky color
			float tmp = 0.5 * (glm::normalize(ray.direction).y + 1.0f);
			light += (glm::vec3{ 1.0f - tmp } + glm::vec3{ 0.5f, 0.7f, 1.0f } *tmp) * contribution;
			break;
		}

		const Sphere &sphere = m_pScene->spheres[payload.objectIndex];
		const Material &material = m_pScene->materials[sphere.materialIndex];

		contribution *= material.albedo;

		const float refractionRatio = 1.0f / material.eta;
		const float cosTheta = std::max(glm::dot(-ray.direction, payload.normal), 0.0f);
		const float sinTheta = std::sqrt(1.0 - cosTheta * cosTheta);
		if (!material.isTransparent ||
			refractionRatio * sinTheta > 1.0f ||
			material.SnellSchlick(cosTheta, refractionRatio) > RandomFloat(seed))
		{
			ray.origin = payload.position + payload.normal * 0.0001f;
			ray.direction = glm::reflect(ray.direction, payload.normal + RandomUnitSphere(seed) * material.roughness);
		}
		else
		{
			ray.origin = payload.position;
			ray.direction = glm::refract(-ray.direction, payload.normal, refractionRatio);
		}
	}

	return glm::vec4{ light, 1.0f };
}

HitPayload Renderer::TraceRay(const Ray &ray)
{
	uint32_t closestSphereIndex = INVALID_INDEX;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t index = 0; index < m_pScene->spheres.size(); ++index)
	{
		const Sphere &sphere = m_pScene->spheres[index];

		glm::vec3 origin = ray.origin - sphere.center;
		float a = glm::dot(ray.direction, ray.direction);
		float b_half = glm::dot(origin, ray.direction);
		float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

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

	if (INVALID_INDEX == closestSphereIndex)
	{
		return Miss(ray);
	}

	return ClosestHit(ray, hitDistance, closestSphereIndex);
}

HitPayload Renderer::ClosestHit(const Ray &ray, float hitDistance, uint32_t objectIndex)
{
	HitPayload payload;
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	const Sphere &closestSphere = m_pScene->spheres[objectIndex];
	payload.position = ray.Cast(hitDistance);
	payload.normal = glm::normalize(payload.position - closestSphere.center);

	return payload;
}

HitPayload Renderer::Miss(const Ray &ray)
{
	HitPayload payload;
	payload.hitDistance = -1.0f;

	return payload;
}
