#include <iostream>
#include "Frustum.h"
#include "Camera.h"

Frustum::Frustum() {
}

void Frustum::CreateFromCamera(Camera* pCam) {
    const glm::vec3& front = pCam->GetFrontVector();
    const glm::vec3& position = pCam->GetPosition();
    const glm::vec3& up = pCam->GetUpVector();
    const glm::vec3& right = pCam->GetRightVector();

    float fAspectRatio = pCam->GetAspectRatio();
    float fNearZ = pCam->GetNearZ();
    float fFarZ = pCam->GetFarZ();

    const glm::vec3 frontPt = fFarZ * front;
    float fovY = glm::radians(pCam->GetZoom());

    float halfVSide = fFarZ * tanf(fovY * 0.5f);
    float halfHSide = halfVSide * fAspectRatio;

    m_aPlanes[ePlaneId_Near] = { front, position + fNearZ * front };
    m_aPlanes[ePlaneId_Far] = { -front, position + frontPt };

    m_aPlanes[ePlaneId_Left] = { glm::cross(up, frontPt + right * halfHSide), position };
    m_aPlanes[ePlaneId_Right] = { glm::cross(frontPt - right * halfHSide, up) , position };
    m_aPlanes[ePlaneId_Top] = { glm::cross(right, frontPt - up * halfVSide) , position };
    m_aPlanes[ePlaneId_Bottom] = { glm::cross(frontPt + up * halfVSide, right), position };
}

bool Frustum::ContainsSphere(const BoundingSphere& s) const {
    for (int e = (int)ePlaneId_Near; e <= (int)ePlaneId_Bottom; ++e) {
        if (!m_aPlanes[e].IsOnOrForwardOfPlane(s)) {
            return false;
        }
    }
    return true;
}
