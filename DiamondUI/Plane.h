#pragma once
#include "BoundingSphere.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Plane
{
public:
    Plane();
    Plane(glm::vec3 vecV1, glm::vec3 vecV2, glm::vec3 vecV3);
    Plane(glm::vec3 vecNormal, glm::vec3 vecV1);
    Plane(const Plane& toCopy);

    bool IsOnOrForwardOfPlane(const BoundingSphere& sphere) const;

protected:
    glm::vec3 m_vecNormal;
    float m_fDistanceToOrigin;
};

