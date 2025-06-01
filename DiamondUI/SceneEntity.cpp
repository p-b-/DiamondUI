#include "SceneEntity.h"

glm::vec3 SceneEntity::s_vecWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

SceneEntity::SceneEntity(Entity3d* pEntity, glm::vec3 vecModel, float fScale) {
	glm::mat4 unit = glm::mat4(1.0f);
	glm::mat4 matModel = glm::translate(unit, vecModel);
	matModel = glm::scale(matModel, glm::vec3(fScale, fScale, fScale));
	m_pEntity = pEntity;
	m_pEntity->IncrementRefCount();
	DetermineIfSelfContainedEntity();
	m_matModel = matModel;
	m_pAttachedTo = nullptr;

	m_boundingSphere = m_pEntity->GetBoundingSphere();
	m_boundingSphere.Scale(matModel);
}

SceneEntity::~SceneEntity() {
	if (m_pEntity != nullptr) {
		m_pEntity->DecrementRefCount();
	}

	for (auto a : m_vcpAttachments) {
		delete a->m_pAttachedTo;
	}
}

void SceneEntity::DetermineIfSelfContainedEntity() {
	SelfContainedEntity3d* pSI = dynamic_cast<SelfContainedEntity3d*>(m_pEntity);
	if (pSI != nullptr) {
		m_bSelfContainedEntity = true;
	}
	else {
		m_bSelfContainedEntity = false;
	}
}

Entity3d* SceneEntity::Get3dEntity() {
	return m_pEntity;
}

glm::mat4 SceneEntity::GetModelMatrix() {
	return m_matModel;
}

glm::vec3 SceneEntity::GetPosition() const {
	if (m_bSelfContainedEntity) {
		return dynamic_cast<SelfContainedEntity3d*>(m_pEntity)->GetPosition();
	}
	else {
		return glm::vec3(m_matModel[3]);
	}
}

void SceneEntity::SetPosition(glm::vec3 vecNewPos) {
	if (m_bSelfContainedEntity) {
		dynamic_cast<SelfContainedEntity3d*>(m_pEntity)->SetPosition(vecNewPos);
	}
	else {
		m_matModel[3] = glm::vec4(vecNewPos, 1);
	}

	for (auto a : m_vcpAttachments) {
		switch (a->m_eAttachmentType) {
		case eAttachmentType_Fixed:
			a->m_pAttachedTo->SetPosition(vecNewPos);
			break;
		}
	}
}

void SceneEntity::OrbitAround(glm::vec3 vecRotateAround, float fRotateDegrees, bool bLookAtCentre) {
	glm::vec3 currentPos = GetPosition();


	glm::vec3 centreToPos = currentPos - vecRotateAround;

	float fRadius = glm::length(centreToPos);
	centreToPos = glm::normalize(centreToPos);

	glm::mat4 transform = glm::mat4(1.0f);

	transform = glm::rotate(transform, glm::radians(fRotateDegrees), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::vec3 vecRotatedCentreToPos = transform * glm::vec4(centreToPos, 1.0f);
	vecRotatedCentreToPos = glm::normalize(vecRotatedCentreToPos);
	vecRotatedCentreToPos *= fRadius;

	SetPosition(vecRotatedCentreToPos + vecRotateAround);

	if (bLookAtCentre) {
		LookAt(vecRotateAround);
	}
}

void SceneEntity::LookAt(glm::vec3 vecLookAt) {
	if (m_pEntity->IsDirectional()) {
		if (m_bSelfContainedEntity) {
			dynamic_cast<SelfContainedEntity3d*>(m_pEntity)->LookAt(vecLookAt);
		}
		else {
			// TODO Alter model matrix
			// 	    Need to know where entity is currently looking to change its direction
		}
	}
	for (auto a : m_vcpAttachments) {
		switch (a->m_eAttachmentType) {
		case eAttachmentType_Fixed:
			a->m_pAttachedTo->LookAt(vecLookAt);
			break;
		}
	}
}

void SceneEntity::LookAt(SceneEntity* pEntity) {
	LookAt(pEntity->GetPosition());
}

void SceneEntity::Move(glm::vec3 vecMove, float fDeltaTime) {
	glm::vec3 currentPos = GetPosition();
	if (m_bSelfContainedEntity) {
		dynamic_cast<SelfContainedEntity3d*>(m_pEntity)->Move(vecMove, fDeltaTime);
	}
	else {
		float speed = CalcSpeed(fDeltaTime);
		auto pos = GetPosition();
		pos += vecMove * speed;
		SetPosition(pos);
	}
}

void SceneEntity::Rotate(float fRadians, glm::vec3 vecRotateAround) {
	m_matModel = glm::rotate(m_matModel, fRadians, vecRotateAround);
}

void SceneEntity::SetColour(float fR, float fG, float fB) {
	m_pEntity->SetColour(fR, fG, fB);
}

void SceneEntity::AttachTo(SceneEntity* pSceneEntity, AttachmentType eAT, glm::vec3 vecOffset) {
	Attachment* pAttachment = new Attachment();
	pAttachment->m_pAttachedTo = this;
	pAttachment->m_eAttachmentType = eAT;
	glm::mat4 matAttachment(1.0f);
	matAttachment = glm::translate(matAttachment, vecOffset);
	pAttachment->m_matTransform = matAttachment;

	pSceneEntity->AddAttachment(pAttachment);
	m_pAttachedTo = pSceneEntity;
}

void SceneEntity::AddAttachment(Attachment* pAttachment) {
	m_vcpAttachments.push_back(pAttachment);
}

float SceneEntity::CalcSpeed(float fDeltaTime) {
	return static_cast<float>(m_fSpeed * fDeltaTime);
}