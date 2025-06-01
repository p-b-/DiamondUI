#pragma once
#include "IScene.h"
#include "Shader.h"

class SceneEntity;
class ShadowedScene :
    public IScene
{
protected:
    void Initialise(unsigned int unShadowMapWidth, unsigned int unShadowMapHeight);
    
public:
    void SetupShadowRendering(SceneEntity* pseLight, glm::vec3 vecLightAimedAt);
    virtual bool GetUsingShadows() { return true; }

protected:
    const glm::mat4& GetLightSpaceTransform() const { return m_matLightSpaceTransform; }

protected:
    unsigned int m_unShadowMapWidth;
    unsigned int m_unShadowMapHeight;

    unsigned int m_unFrameBufferObject;
    unsigned int m_unDepthMapTexture;

private:
    glm::mat4 m_matLightSpaceTransform;
};

