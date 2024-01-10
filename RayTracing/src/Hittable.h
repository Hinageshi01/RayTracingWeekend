#pragma once

#include "HitPayload.h"
#include "Ray.h"

#include <glm/glm.hpp>

class Hittable
{
public:
	virtual HitPayload Hit(const Ray &ray, float t_min, float t_max) const = 0;
};
