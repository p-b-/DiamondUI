#pragma once
#include "Entity3d.h"
#include "BoundingSphere.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

// TODO Implement attachments. _fixed is implemented so that lights can be attached to 3d objects (spheres).
enum AttachmentType {
	eAttachmentType_Fixed, // Will absolutely follow the transforms of whatever the entity is attached to
	eAttachmentType_FixedFollow, // Will keep vector position relative to attachment, but not alter where it is pointed at.
	eAttachmentType_Follow, // Will keep angular position relative to attachment, but only around Z=X=0 axis
	eAttachmentType_GradualFollow // Like eAttachmentType_Follow, but does not shift rotation immediately.
};

class SceneEntity;
class Attachment {
public:
	SceneEntity* m_pAttachedTo;
	AttachmentType m_eAttachmentType;
	glm::mat4 m_matTransform;
};

class SceneEntity
{
public:
	SceneEntity(Entity3d* pEntity, glm::vec3 vecPos, float fScale = 1.0f);
	SceneEntity(const SceneEntity& toCopy) = delete;
	SceneEntity(SceneEntity&& toMove) noexcept =  delete;
	~SceneEntity();

	void SetSpeed(float fSpeed) { m_fSpeed = fSpeed; }

	SceneEntity& operator=(const SceneEntity& toCopy) = delete;

	Entity3d* Get3dEntity();
	glm::mat4 GetModelMatrix();
	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 vecNewPos);
	void OrbitAround(glm::vec3 vecRotateAround, float fRotateDegrees, bool bLookAtCentre);
	void LookAt(glm::vec3 vecLookAt);
	void LookAt(SceneEntity* pEntity);

	void Move(glm::vec3 vecMove, float fDeltaTime);

	void Rotate(float fRadians, glm::vec3 vecRotateAround);
	void SetColour(float fR, float fG, float fB);

	void AttachTo(SceneEntity* pSceneEntity, AttachmentType eAT, glm::vec3 vecOffset);
	BoundingSphere GetBoundingSphere() const { return m_boundingSphere; }

private:
	void DetermineIfSelfContainedEntity();
	void AddAttachment(Attachment* pAttachment);
	float CalcSpeed(float fDeltaTime);

private:
	Entity3d* m_pEntity;
	glm::mat4 m_matModel;
	float m_fSpeed;

	std::vector<Attachment* > m_vcpAttachments;
	SceneEntity* m_pAttachedTo;
	bool m_bSelfContainedEntity;

	BoundingSphere m_boundingSphere;
	static glm::vec3 s_vecWorldUp;
};

