#include "Plane.h"

Plane::Plane() {
    m_vecNormal = glm::vec3(0.0f);
    m_fDistanceToOrigin = 0;
}

Plane::Plane(glm::vec3 vecV1, glm::vec3 vecV2, glm::vec3 vecV3) {
    glm::vec3 v1ToV2 = vecV2 - vecV1;
    glm::vec3 v3ToV2 = vecV2 - vecV3;

    m_vecNormal = glm::normalize(glm::cross(v1ToV2, v3ToV2));

    m_fDistanceToOrigin = glm::dot(m_vecNormal, vecV1);
}

Plane::Plane(glm::vec3 vecNormal, glm::vec3 vecV1) {
    m_vecNormal = glm::normalize(vecNormal);
    m_fDistanceToOrigin = glm::dot(m_vecNormal, vecV1);
}


Plane::Plane(const Plane& toCopy) {
    m_vecNormal = toCopy.m_vecNormal;
    m_fDistanceToOrigin = toCopy.m_fDistanceToOrigin;
}

bool Plane::IsOnOrForwardOfPlane(const BoundingSphere& sphere) const {
    float signedDistanceToPane = glm::dot(m_vecNormal, sphere.GetCentre()) - m_fDistanceToOrigin;
    return signedDistanceToPane > -sphere.GetRadius();
}