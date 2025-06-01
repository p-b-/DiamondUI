#include "Entity3d.h"
#ifdef _DEBUG
#include <iostream>
#include <sstream>
#include <iomanip>
#endif

Entity3d::Entity3d() {
	m_unTriCount = 0;
	m_unEntityId = 0;
	m_bReceivesLight = true;
}

Entity3d::~Entity3d() {
}

glm::vec3 Entity3d::NormalFromVertices(const glm::vec3& vecPos1, const glm::vec3& vecPos2, const glm::vec3& vecPos3) {
	glm::vec3 v1 = vecPos2 - vecPos1;
	glm::vec3 v2 = vecPos3 - vecPos1;

	glm::vec3 crossProd = glm::cross(v1, v2);
	return glm::normalize(crossProd);
}

#ifdef _DEBUG
std::string Entity3d::VectorToStr(const glm::vec3& v) {
	std::ostringstream ss;
	ss << std::setprecision(1) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return ss.str();
}

void Entity3d::OutputVertex(const glm::vec3& v) {
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}
#endif