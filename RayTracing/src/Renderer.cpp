#include "Renderer.h"

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

bool HitSphere(const glm::vec3 &center, double radius, const Ray &r)
{
	// (P - C) dot (P - C) = r^2
	// t^2b dot b + 2tb dot (O - C) + (O - C) dot (O - C) - r^2 = 0

	glm::vec3 oc = r.origin - center;
	float a = glm::dot(r.direction, r.direction);
	float b = 2.0f * glm::dot(r.direction, oc);
	float c = glm::dot(oc, oc) - radius * radius;
	float discriminant = b * b - 4.0f * a * c;

	return discriminant > 0.0f;
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

void Renderer::Render(const Camera &camera)
{
	m_pCamera = &camera;
	assert(m_pCamera && m_pFinalImage);

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

	if (HitSphere(glm::vec3{ 0.0f, 0.0f, -1.0f }, 0.5f, ray))
	{
		return glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f };
	}

	return glm::vec4{ Miss(ray).color, 1.0f };
}

HitPayload Renderer::TraceRay(const Ray &ray)
{
	return HitPayload();
}

HitPayload Renderer::ClosestHit(const Ray &ray, float hitDistance, uint32_t objectIndex)
{
	return HitPayload();
}

HitPayload Renderer::Miss(const Ray &ray)
{
	HitPayload payload;

	float t = 0.5 * (glm::normalize(ray.direction).y + 1.0f);
	payload.color = glm::vec3{ 1.0f - t } + glm::vec3{ 0.5f, 0.7f, 1.0f } *t;

	return payload;
}
