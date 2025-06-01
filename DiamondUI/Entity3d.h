#pragma once
#include "Entity.h"
#include "BoundingSphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#ifdef _DEBUG
#include <string>
#endif

class Textures;
class Camera;
class Shader;

class Entity3d : public Entity
{
public:
	Entity3d();
	virtual ~Entity3d();

	virtual void Initialise(unsigned int unEntityId) = 0;
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl) = 0;
	virtual void Deinitialise() = 0;
	virtual void PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace) = 0;
	virtual void PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture) = 0;
	virtual void Draw(glm::mat4 matModel, bool bForShadowMap) = 0;
	virtual void SetColour(float r, float g, float b) = 0;
	virtual bool IsDrawable() const = 0;
	virtual bool IsDirectional() const = 0;
	unsigned int GetEntityId() const { return m_unEntityId; }
	void ReceivesLight(bool bReceivesLight) { m_bReceivesLight = bReceivesLight; }
	bool GetReceivesLight() const { return m_bReceivesLight; }

#ifdef _DEBUG
	std::string VectorToStr(const glm::vec3& v);
	void OutputVertex(const glm::vec3& v);

#endif
	BoundingSphere GetBoundingSphere() const { return m_boundingSphere; }

protected:
	glm::vec3 NormalFromVertices(const glm::vec3& vecPos1, const glm::vec3& vecPos2, const glm::vec3& vecPos3);

protected:
	unsigned int m_unEntityId;
	unsigned int m_unTriCount;
	bool m_bReceivesLight;

	BoundingSphere m_boundingSphere;
};

// Entities that can only uses in one SceneEntity per instance
class SelfContainedEntity3d : public Entity3d {
public:
	virtual void SetPosition(const glm::vec3& vecPos) = 0;
	virtual const glm::vec3& GetPosition() const = 0;
	virtual void Move(glm::vec3 vecMove, float fDeltaTime) = 0;
	virtual void LookAt(glm::vec3 vecLookAt) = 0;
};