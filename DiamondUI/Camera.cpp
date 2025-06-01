#include "Camera.h"
#include "SceneEntity.h"

#ifdef _DEBUG
#include <iostream>>
#endif


Camera::Camera(glm::vec2 vecScreenSize, glm::vec3 vecPosition, float fNearZ, float fFarZ) {
    m_matView = glm::mat4(1.0f);
    m_matProjection = glm::mat4(1.0f);

    m_fFarZ = fFarZ;
    m_fNearZ = fNearZ;

    m_fScreenWidth = vecScreenSize.x;
    m_fScreenHeight = vecScreenSize.y;
    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
    m_fYaw = -90.0f;	
    m_fPitch = 0.0f;
    m_fFov = 45.0f;

    m_vecCameraPos = vecPosition;
    m_vecCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_vecWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    m_vecCameraRight = glm::normalize(glm::cross(m_vecCameraFront, m_vecWorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    m_vecCameraUp = glm::normalize(glm::cross(m_vecCameraRight, m_vecCameraFront));
    m_bUpdateFrustum = true;

    m_bUpdateViewMatrix = true;
    m_bUpdateProjectionMatrix = true;
}

Camera::~Camera() {
    Deinitialise();
}

void Camera::Move(glm::vec3 vecMove, float fDeltaTime) {
    float cameraSpeed = CalcCameraSpeed(fDeltaTime);
    // Forwards
    m_vecCameraPos += cameraSpeed * m_vecCameraFront * vecMove.z;
    // Right
    m_vecCameraPos += glm::normalize(glm::cross(m_vecCameraFront, m_vecCameraUp)) * cameraSpeed*vecMove.x;
    // Upwards
    m_vecCameraPos += cameraSpeed * m_vecCameraUp * vecMove.y;
    m_bUpdateViewMatrix = true;
}

float Camera::CalcCameraSpeed(float fDeltaTime) {
	return static_cast<float>(2.5 * fDeltaTime);
}

void Camera::UpdateProjectionMatrix() {
    m_matProjection = glm::perspective(glm::radians(m_fFov), m_fScreenWidth / m_fScreenHeight, m_fNearZ, m_fFarZ);
    m_bUpdateProjectionMatrix = false;
    m_bUpdateFrustum = true;
}

void Camera::UpdateViewMatrix() {
    m_vecCameraRight = glm::normalize(glm::cross(m_vecCameraFront, m_vecWorldUp));
    m_vecCameraUp = glm::normalize(glm::cross(m_vecCameraRight, m_vecCameraFront));

    m_matView = glm::lookAt(m_vecCameraPos, m_vecCameraPos + m_vecCameraFront, m_vecCameraUp);
    m_bUpdateViewMatrix = false;
    m_bUpdateFrustum = true;
}

void Camera::UpdateFrustum() {
    m_frustum.CreateFromCamera(this);
    m_bUpdateFrustum = false;
}

const glm::mat4& Camera::GetProjectionMatrix() 
{
    if (m_bUpdateProjectionMatrix) {
        UpdateProjectionMatrix();
    }
    return m_matProjection; 
}

const glm::mat4& Camera::GetViewMatrix() 
{
    if (m_bUpdateViewMatrix) {
        UpdateViewMatrix();
    }
    return m_matView;
}

const Frustum& Camera::GetFrustum(){
    if (m_bUpdateFrustum) {
        UpdateFrustum();
    }
    return m_frustum;
}

void Camera::YawAndPitch(float fYawDelta, float fPitchDelta) {
    m_fYaw += fYawDelta;
    m_fPitch += fPitchDelta;
    
    // Make sure out-of-bounds pitch does not flip screen
    ClampTo(m_fPitch, -89.0f, 89.0f);
    glm::vec3 front;
    front.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    front.y = sin(glm::radians(m_fPitch));
    front.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    m_vecCameraFront = glm::normalize(front);
    m_bUpdateViewMatrix = true;
}

void Camera::ClampTo(float& fValue, float fLower, float fHigher) {
    if (fValue < fLower) {
        fValue = fLower;
    }
    else if (fValue > fHigher) {
        fValue = fHigher;
    }
}

void Camera::Zoom(float fYOffset) {
    m_fFov += fYOffset;
    ClampTo(m_fFov, 1.0f,45.0f);
    m_bUpdateProjectionMatrix = true;
}

void Camera::LookAt(glm::vec3 vecLookAt) {
    m_vecCameraFront = glm::normalize(vecLookAt - m_vecCameraPos);
    // 1.0f because vector is normalised, so hypoteneuse is 1.0.
    m_fPitch = glm::degrees(asinf(m_vecCameraFront.y / 1.0f));
    ClampTo(m_fPitch, -89.0f, 89.0f);

    // Reversing these:
    //  front.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    //  front.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
    // to get m_fYaw

    glm::vec3 reversed = glm::normalize(
            glm::vec3(m_vecCameraFront.x / cos(glm::radians(m_fPitch)), 
            0.0,
            m_vecCameraFront.z / cos(glm::radians(m_fPitch))));
    m_fYaw = glm::degrees(acosf(reversed.x / 1.0f));

    if (m_vecCameraFront.z < 0) {
        m_fYaw = -m_fYaw;
    }

    m_bUpdateViewMatrix = true;
}

glm::vec2 Camera::GetPitchAndYaw() const {
    return glm::vec2(m_fPitch, m_fYaw);
}

void Camera::SetScreenSize(glm::vec2 vecScreenSize) {
    m_fScreenWidth = vecScreenSize.x;
    m_fScreenHeight = vecScreenSize.y;
    m_bUpdateProjectionMatrix = true;
}

void Camera::SetPosition(const glm::vec3& vecPos) {
    m_vecCameraPos = vecPos;
    m_bUpdateViewMatrix = true;
}

void Camera::LookAt(SceneEntity* pEntity) {
    LookAt(pEntity->GetPosition());
}

#ifdef _DEBUG
void Camera::OutputInfo() {
    std::cout << "Camera pos: " << VectorToStr(m_vecCameraPos) << std::endl;
    std::cout << "        fr: " << VectorToStr(m_vecCameraFront) << std::endl;
    std::cout << "        =>: " << VectorToStr(m_vecCameraRight) << std::endl;
    std::cout << "        up: " << VectorToStr(m_vecCameraUp) << std::endl;
    std::cout << "        yw: " << m_fYaw << ", pt: " << m_fPitch << std::endl;
}
#endif
