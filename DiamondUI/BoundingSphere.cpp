#include "BoundingSphere.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>>
#include <algorithm>

BoundingSphere::BoundingSphere() {
	m_vecCentre = glm::vec3(0.0f);
	m_fRadius = 0.0f;
}

BoundingSphere::BoundingSphere(glm::vec3 vecCentre, float fRadius) {
	m_vecCentre = vecCentre;
	m_fRadius = fRadius;
}

BoundingSphere::BoundingSphere(const BoundingSphere& toCopy) {
	m_vecCentre = toCopy.m_vecCentre;
	m_fRadius = toCopy.m_fRadius;
}

float BoundingSphere::GetRadius() const {
	return m_fRadius;
}
glm::vec3 BoundingSphere::GetCentre() const {
	return m_vecCentre;
}

void BoundingSphere::Scale(glm::mat4 matScale) {
	float r = glm::length(matScale[0]);
	float u = glm::length(matScale[1]);
	float b = glm::length(matScale[2]);

	float maxScale = std::max(r, std::max(u,b));
	m_fRadius = m_fRadius * maxScale;
}


void BoundingSphere::Transform(glm::mat4 transform) {
	glm::vec3 scale;
	glm::quat rotation;
	glm::vec3 translation;
	glm::vec3 skew;
	glm::vec4 perspective;

	glm::decompose(transform, scale, rotation, translation, skew, perspective);
	 auto v = transform * glm::vec4(m_vecCentre,1.0f);
	 m_vecCentre = v;
}

void BoundingSphere::ExpandToIncludeSphere(const BoundingSphere& sphere) {
	glm::vec3 dv = m_vecCentre - sphere.m_vecCentre;
	float distance = glm::length(dv);

	float newRadius = distance + sphere.m_fRadius;

	if (newRadius > m_fRadius) {
		m_fRadius = newRadius;
	}
}