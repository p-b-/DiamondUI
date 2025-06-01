#pragma once
#include "Entity3d.h"
#include "Frustum.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifdef _DEBUG
#include <string>
#endif

class SceneEntity;

class Camera : public SelfContainedEntity3d
{
public:
	Camera(glm::vec2 vecScreenSize, glm::vec3 vecPosition, float fNearZ, float fFarZ);
	virtual ~Camera();
	virtual void Initialise(unsigned int unEntityId) {}
	virtual void Initialise(unsigned int unEntityId, Textures* pTextureCtrl) {}
	virtual void Deinitialise() {}
	virtual void PrepareToDrawShadows(const glm::vec3& vecLightPos, const glm::mat4& matLightSpace) {}
	virtual void PrepareToDraw(Camera* pCamera, const glm::vec3& vecLightPos, const glm::mat4& matLightSpace, unsigned int unDepthMapTexture) {}
	virtual void Draw(glm::mat4 matModel, bool bForShadowMap) {}
	virtual void SetColour(float r, float g, float b) {}
	virtual bool IsDrawable() const { return false; }
	virtual bool IsDirectional() const { return true; }

	virtual void Move(glm::vec3 vecMove, float fDeltaTime);
	virtual void LookAt(glm::vec3 vecLookAt);
	void YawAndPitch(float fYawDelta, float fPitchDelta);
	void Zoom(float fYOffset);
	void LookAt(SceneEntity* pEntity);
	glm::vec2 GetPitchAndYaw() const;
	void SetScreenSize(glm::vec2 vecScreenSize);

public:
	const glm::mat4& GetProjectionMatrix();
	const glm::mat4& GetViewMatrix();
	virtual const glm::vec3& GetPosition() const { return m_vecCameraPos; }
	virtual void SetPosition(const glm::vec3& vecPos);
	const glm::vec3& GetFrontVector() const { return m_vecCameraFront; }
	const glm::vec3& GetUpVector() const { return m_vecCameraUp; }
	const glm::vec3& GetWorldUpVector() const { return m_vecWorldUp; }
	const glm::vec3& GetRightVector() const { return m_vecCameraRight; }
	const float GetAspectRatio() const { return m_fScreenWidth / m_fScreenHeight; }
	const float GetZoom() const { return m_fFov; }
	const float GetNearZ() const { return m_fNearZ; }
	const float GetFarZ() const { return m_fFarZ; }

	const Frustum& GetFrustum();

#ifdef _DEBUG
	void OutputInfo();
#endif

protected:
	float CalcCameraSpeed(float fDeltaTime);
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	void UpdateFrustum();
	void ClampTo(float& fValue, float fLower, float fHigher);

private:
	float m_fNearZ;
	float m_fFarZ;

	glm::vec3 m_vecCameraPos;
	glm::vec3 m_vecCameraFront;
	glm::vec3 m_vecCameraUp;
	glm::vec3 m_vecCameraRight;

	glm::vec3 m_vecWorldUp;

	glm::mat4 m_matProjection;
	glm::mat4 m_matView;

	float m_fYaw;
	float m_fPitch;
	float m_fFov;

	float m_fScreenWidth;
	float m_fScreenHeight;

	Frustum m_frustum;

	bool m_bUpdateViewMatrix;
	bool m_bUpdateProjectionMatrix;
	bool m_bUpdateFrustum;
};
