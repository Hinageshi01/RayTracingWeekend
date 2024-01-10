#pragma once

#include "Hittable.h"

#include <glm/glm.hpp>

class Sphere : public Hittable
{
public:
    Sphere() = default;
    Sphere(glm::vec3 center, float radius) : m_center(std::move(center)), m_radius(radius)
    {

    };

public:
    glm::vec3 m_center;
    float m_radius;
};
