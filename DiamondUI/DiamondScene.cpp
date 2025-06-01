#include "DiamondScene.h"
#include "DiamondGame.h"
#include "Diamond.h"
#include "Sphere3d.h"
#include "Tile3d.h"
#include "Shader.h"
#include "Textures.h"
#include "Camera.h"
#include "IGameLoop.h"
#include "PointLight.h"
#include "GameOverlay.h"
#include "TextRenderer.h"
#include "IWindow.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/matrix_decompose.hpp>>
#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

DiamondScene::DiamondScene(DiamondGame* pGame) {
    m_pGame = pGame;
	m_bCameraLocked = false;
    m_bDisplayShadowDebug = false;
    m_pCam = nullptr;
    m_unDiamondEntity = 0;
    m_unLightSphereEntityId = 0;
    m_unSphereEntityId = 0;
    m_unTileEntityId = 0;
    m_unDepthMapTexture = 0;
	m_pOverlay = nullptr;

    m_pDiamond = nullptr;
    m_pGameLoop = nullptr;
    m_pShere = nullptr;
    m_pseCamera = nullptr;
    m_pseLight1 = nullptr;
    m_pseLightingSphere = nullptr;
    m_vecLightPosition = glm::vec3(0.0f);
    m_vecRotateLightAround = glm::vec3(0.0f);
}

DiamondScene::~DiamondScene() {
}

// IScene implementation
//
bool DiamondScene::Initialise(Textures* pTextureCtrl, IWindow* pWnd) {
    ShadowedScene::Initialise(1*1024, 1*1024);

    m_vecRotateLightAround = glm::vec3(0.0f, 0.0f, -2.0f);
    m_bDisplayShadowDebug = false;
    
    InitialiseCamera(pWnd, glm::vec3(0.0f, 1.5, 0.0f));
    InitialiseLighting(glm::vec3(-2.0f, 2.0f, -1.0f));
    InitialiseDiamonds();
    InitialiseTiles();
    InitialiseSpheres(glm::vec3(-2.0f, 1.0f, -2.0f));
	return true;
}

void DiamondScene::Deinitialise() {
    for (auto pSE : m_vcEntities) {
        delete pSE;
    }
	m_vcEntities.clear();
}

void DiamondScene::WindowSizeChanged(int nWidth, int nHeight) {
    if (m_pCam != nullptr) {
        glm::vec2 vecScreenSize = glm::vec2((float)nWidth, (float)nHeight);

        m_pCam->SetScreenSize(vecScreenSize);
    }
}

void DiamondScene::SetGameLoop(IGameLoop* pGameLoop) {
    m_pGameLoop = pGameLoop;
}

void DiamondScene::AnimateScene(float fDeltaTime) {
    for (auto pSE : m_vcEntities) {
        if (pSE->Get3dEntity()->GetEntityId() != m_unDiamondEntity) {
            continue;
        }
        pSE->Rotate(glm::radians(90.0f*fDeltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void DiamondScene::DrawScene(bool bForShadows) {
    if (bForShadows) {
        SetupShadowRendering(m_pseLight1, m_vecRotateLightAround);
        NoCullForLightFrustum();
        glCullFace(GL_FRONT);
    }
    else {
        CullEntitiesFromFrustum();
    }
    glm::vec3 vecLightPos = m_vcLights[0]->GetPosition();

    bool anyDrawn = false;
    for (unsigned int unEntityId = 1; unEntityId < m_unNextEntityId; unEntityId++)
    {
        bool prepared = false;
        for (auto pSE : m_vcEntitiesToDraw) {
            if (pSE->Get3dEntity()->GetEntityId() != unEntityId) {
                continue;
            }
            if (pSE->Get3dEntity()->IsDrawable() == false) {
                break;
            }
            auto e = pSE->Get3dEntity();
            auto m = pSE->GetModelMatrix();
            if (!prepared) {
                if (bForShadows) {
                    e->PrepareToDrawShadows(vecLightPos, GetLightSpaceTransform());
                }
                else {
                    e->PrepareToDraw(m_pCam, vecLightPos, GetLightSpaceTransform(), m_unDepthMapTexture);
                }
                prepared = true;
            }
            e->Draw(m, bForShadows);
            anyDrawn = true;
        }
    }
    if (!anyDrawn) {
        // force view frustum to be reeavulated - to allow for culling in next call to CullEntitiesFromFrustum
        m_pCam->GetViewMatrix();
    }
    glCullFace(GL_BACK);
}
//
// IScene implementation

void DiamondScene::InitialiseCamera(IWindow* pWnd, glm::vec3 vecPos) {
    glm::vec2 vecScreenSize = glm::vec2((float)pWnd->GetWidth(), (float)pWnd->GetHeight());
    float fNearZ = 0.1f;
    float fFarZ = 100.0f;
    if (m_pseCamera!=nullptr) {
        // This will delete m_pCam
        delete m_pseCamera;
        m_pseCamera = nullptr;
    }
    m_pCam = new Camera(vecScreenSize, vecPos, fNearZ, fFarZ);
    m_pCam->LookAt(m_vecRotateLightAround);
    m_pseCamera = new SceneEntity(m_pCam, vecPos);
    m_bCameraLocked = false;
    m_vcEntities.push_back(m_pseCamera);
}

void DiamondScene::InitialiseLighting(glm::vec3 vecLightPos) {
    Sphere3d* pLightingSphere = new Sphere3d(2);
    pLightingSphere->ReceivesLight(false);
    m_unLightSphereEntityId = m_unNextEntityId++;
    pLightingSphere->Initialise(m_unLightSphereEntityId);

    m_pseLightingSphere = new SceneEntity(pLightingSphere, vecLightPos, 0.25f);
    m_pseLightingSphere->SetSpeed(0.25f);

    PointLight* pLight1 = new PointLight(glm::vec3(1.0f, 1.0f, 1.0f));
    m_pseLight1 = new SceneEntity(pLight1, vecLightPos);
    m_pseLight1->AttachTo(m_pseLightingSphere, eAttachmentType_Fixed, glm::vec3(0));

    m_vcEntities.push_back(m_pseLightingSphere);
    m_vcLights.push_back(m_pseLight1);
}

void DiamondScene::InitialiseDiamonds() {
    Diamond* pDiamond = new Diamond();
    m_unDiamondEntity = m_unNextEntityId++;
    pDiamond->Initialise(m_unDiamondEntity);

    m_pDiamond = new SceneEntity(pDiamond, m_vecRotateLightAround);
    // Colour will change to red when controller connects. This is to give visual feedback on the steamdeck.
    m_pDiamond->SetColour(0.0f, 1.0f, 0.0f);
    m_vcEntities.push_back(m_pDiamond);
}

void DiamondScene::InitialiseTiles() {
    Tile3d* pTile = new Tile3d();

    m_unTileEntityId = m_unNextEntityId++;
    pTile->Initialise(m_unTileEntityId);

    SceneEntity* pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(0.0f, -1.0, 0.0f));
    m_vcEntities.push_back(pTileSE);

    float negD = -6.0f;
    float posD = 6.0f;

    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(negD, -0.8, 0.0f));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(posD, -0.8, 0.0f));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(0.0f, -0.8, negD));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(0.0f, -0.8, posD));
    m_vcEntities.push_back(pTileSE);

    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(negD, -0.6, negD));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(posD, -0.6, negD));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(posD, -0.6, posD));
    m_vcEntities.push_back(pTileSE);
    pTileSE = new SceneEntity(pTile, m_vecRotateLightAround + glm::vec3(negD, -0.6, posD));
    m_vcEntities.push_back(pTileSE);
}

void DiamondScene::InitialiseSpheres(glm::vec3 vecPos) {
    Sphere3d* pSphere = new Sphere3d(2);
    m_unSphereEntityId = m_unNextEntityId++;
    pSphere->Initialise(m_unSphereEntityId);
    m_pShere = new SceneEntity(pSphere, vecPos);
    m_vcEntities.push_back(m_pShere);
}

void DiamondScene::ApplyAnalogMovement(float fX, float fY, float fZ, float fDeltaTime) {
    if (m_bCameraLocked) {
        if (fabsf(fX) > 0.01) {
            m_pseLightingSphere->OrbitAround(m_vecRotateLightAround, fX, false);
        }

        if (fabsf(fZ) > 0.01) {
            m_pseLightingSphere->Move(glm::vec3(0, fZ, 0), fDeltaTime);
        }
    }
    else {
        m_pseCamera->Move(glm::vec3(fX, fY, fZ), fDeltaTime);
    }
}

void DiamondScene::ControllerConnected() {
    m_pDiamond->SetColour(1.0f, 0.0f, 0.0f);
}

void DiamondScene::ProcessDigitalAction(SceneAction eAction, float fDeltaTime) {
    switch (eAction) {
    case eSceneAction_Pause:
        m_pGame->DisplayUI(true);
        break;
    case eSceneAction_ShowDepthMap:
        m_bDisplayShadowDebug = !m_bDisplayShadowDebug;
        m_pGameLoop->DisplayDepthMap(m_bDisplayShadowDebug, m_unDepthMapTexture, 1.0f, 7.5f);
        break;
    case eSceneAction_Fire:


        break;
    case eSceneAction_ShowBindingPanel:
        m_pGameLoop->DisplayBindingPanel();
        break;
    case eSceneAction_LockCamera:
#ifdef _DEBUG
        m_pCam->OutputInfo();
#endif
        m_bCameraLocked = !m_bCameraLocked;
        if (m_bCameraLocked) {
            m_pseCamera->LookAt(m_pDiamond);
        }
        break;
    /*case eSceneAction_Accelerate:
        m_pGame->MenuTabRight();
        break;
    case eSceneAction_Decelerate:
        m_pGame->MenuTabLeft();
        break;*/
    case eSceneAction_Quit:
        m_pGameLoop->QuitGame();
        break;
    }
}

void DiamondScene::ProcessAnalogAction(SceneAction eAction, float fAnalogValue, float fDeltaTime) {
    switch (eAction) {
    case eSceneAction_Zoom:
        m_pCam->Zoom(-fAnalogValue);
        return;
    }
}

void DiamondScene::ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fDeltaTime) {
    ProcessAnalogAction(eAction, fAnalogValue1, 0.0f, fAnalogValue2, fDeltaTime);
}

void DiamondScene::ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fAnalogValue3, float fDeltaTime) {
    switch (eAction) {
    case eSceneAction_MoveCamera:
        if (m_bCameraLocked) {
            m_pseCamera->OrbitAround(m_vecRotateLightAround, fAnalogValue1, true);
        }
        else {
            m_pCam->YawAndPitch(fAnalogValue1, fAnalogValue3);
            m_pseCamera->Move(glm::vec3(0, fAnalogValue2, 0), fDeltaTime);
        }
        return;
    case eSceneAction_Move:
        ApplyAnalogMovement(fAnalogValue1,fAnalogValue2, fAnalogValue3, fDeltaTime);
        return;
    }
}

void DiamondScene::NoCullForLightFrustum() {
    m_vcEntitiesToDraw.clear();
    for (auto pSE : m_vcEntities) {
        if (pSE->Get3dEntity()->IsDrawable() == false ||
            pSE->Get3dEntity()->GetReceivesLight() == false) {
            continue;
        }
        m_vcEntitiesToDraw.push_back(pSE);
    }
}

void DiamondScene::CullEntitiesFromFrustum() {
    const Frustum& f = m_pCam->GetFrustum();
    m_vcEntitiesToDraw.clear();
    for (auto pSE: m_vcEntities) {
        if (pSE->Get3dEntity()->IsDrawable() == false) {
            continue;
        }
        BoundingSphere sphere = pSE->Get3dEntity()->GetBoundingSphere();
        sphere.Transform(pSE->GetModelMatrix());
        if (f.ContainsSphere(sphere)) {
            m_vcEntitiesToDraw.push_back(pSE);
        }
    }
}

glm::vec3 DiamondScene::GetCameraPosition() {
    return m_pseCamera->GetPosition();
}

glm::vec2 DiamondScene::GetCameraPitchAndYaw() {
    Camera* pCam = dynamic_cast<Camera*>(m_pseCamera->Get3dEntity());
    return pCam->GetPitchAndYaw();
}