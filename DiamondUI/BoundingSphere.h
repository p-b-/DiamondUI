#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class BoundingSphere
{
public:
	BoundingSphere();
	BoundingSphere(glm::vec3 vecCentre, float fRadius);
	BoundingSphere(const BoundingSphere& toCopy);

	float GetRadius() const;
	glm::vec3 GetCentre() const;

	void Transform(glm::mat4 transform);
	void Scale(glm::mat4 matScale);
	void ExpandToIncludeSphere(const BoundingSphere& sphere);


protected:
	glm::vec3 m_vecCentre;
	float m_fRadius;
};

