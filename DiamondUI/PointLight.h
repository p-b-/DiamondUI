#pragma once
#include "Entity3d.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Entity3d;
class Camera;
class Shader;

class PointLight : public Entity3d
{
public:
	PointLight(glm::vec3 vecColour);
	virtual ~PointLight();
	virtual void Initialise(unsigned int unEntityId) {}
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl) {}
	virtual void Deinitialise() {}
	virtual void PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace) {}
	virtual void PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture) {}
	virtual void Draw(glm::mat4 matModel, bool bForShadowMap) {}
	virtual bool IsDrawable() const { return false; }
	virtual bool IsDirectional() const { return false; }

	virtual void SetColour(float r, float g, float b) {}

protected:
	glm::vec3 m_vecColour;
};

