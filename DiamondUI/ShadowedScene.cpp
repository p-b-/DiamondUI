#include "ShadowedScene.h"
#include "SceneEntity.h"
#include "PointLight.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void ShadowedScene::Initialise(unsigned int unShadowMapWidth, unsigned int unShadowMapHeight) {
    m_unShadowMapWidth = unShadowMapWidth;
    m_unShadowMapHeight = unShadowMapHeight;

    glGenFramebuffers(1, &m_unFrameBufferObject);

    glGenTextures(1, &m_unDepthMapTexture);
    glBindTexture(GL_TEXTURE_2D, m_unDepthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_unShadowMapWidth, m_unShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    // Bind the texture as the fbo's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_unFrameBufferObject);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_unDepthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowedScene::SetupShadowRendering(SceneEntity* pseLight, glm::vec3 vecLightAimedAt) {
    glm::mat4 lightProjection;
    glm::mat4 lightView;
    float near_plane = 1.0f;
    float far_plane = 7.5f;
    //lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    float fAspect = (float)m_unShadowMapWidth / (float)m_unShadowMapHeight;
    lightProjection = glm::perspective(glm::radians(135.0f), fAspect, 1.0f,25.0f);

    lightView = glm::lookAt(pseLight->GetPosition(), vecLightAimedAt, glm::vec3(0.0, 1.0, 0.0));
    m_matLightSpaceTransform = lightProjection * lightView;

    glViewport(0, 0, m_unShadowMapWidth, m_unShadowMapHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, m_unFrameBufferObject);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_unDepthMapTexture);
}
