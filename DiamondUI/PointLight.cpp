#include "PointLight.h"
#include "Entity3d.h"
#include "Camera.h"


PointLight::PointLight(glm::vec3 vecColour) {
	m_vecColour = vecColour;
}

PointLight::~PointLight() {
	Deinitialise();
}
