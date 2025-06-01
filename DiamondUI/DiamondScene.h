#pragma once
#include "ShadowedScene.h"
#include "IDiamondScene.h"
#include "SceneEntity.h"
#include "SceneActions.h"
#include <vector>
#include <map>
class Diamond;
class IWindow;
class IGameLoop;
class DiamondGame;
class GameOverlay;

class DiamondScene : public ShadowedScene, public IDiamondScene {
public:
	DiamondScene(DiamondGame* pGame);
	virtual ~DiamondScene();

	// IScene declarations
	virtual bool Initialise(Textures* pTextureCtrl, IWindow* pWnd);
	virtual void Deinitialise();
	virtual void WindowSizeChanged(int nWidth, int nHeight);
	virtual void SetGameLoop(IGameLoop* pGameLoop);
	virtual void AnimateScene(float fDeltaTime);
	virtual void DrawScene(bool bForShadows);

	// IDiamondScene declarations
	virtual void ControllerConnected();
	virtual void ProcessDigitalAction(SceneAction eAction, float fDeltaTime);
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue, float fDeltaTime);
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fDeltaTime);
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fAnalogValue3, float fDeltaTime);

	glm::vec3 GetCameraPosition();
	glm::vec2 GetCameraPitchAndYaw();
private:
	void ApplyAnalogMovement(float fX, float fY, float fZ, float fDeltaTime);


private:
	void InitialiseCamera(IWindow* pWnd, glm::vec3 vecPos);
	void InitialiseLighting(glm::vec3 vecLightPos);
	void InitialiseDiamonds();
	void InitialiseTiles();
	void InitialiseSpheres(glm::vec3 vecPos);

	void NoCullForLightFrustum();
	void CullEntitiesFromFrustum();

private:
	std::vector<SceneEntity*> m_vcEntities;
	std::vector<SceneEntity*> m_vcEntitiesToDraw;

	std::vector<SceneEntity*> m_vcLights;

	SceneEntity* m_pseLight1;
	SceneEntity* m_pseLightingSphere;
	SceneEntity* m_pDiamond;
	SceneEntity* m_pseCamera;
	SceneEntity* m_pShere;
	Camera* m_pCam;
	IGameLoop* m_pGameLoop;
//	TextRenderer* m_pTextRenderer;

	bool m_bCameraLocked;
	glm::vec3 m_vecLightPosition;
	glm::vec3 m_vecRotateLightAround;

	unsigned int m_unNextEntityId = 1;
	unsigned int m_unDiamondEntity;

	bool m_bDisplayShadowDebug;

	unsigned int m_unLightSphereEntityId;
	unsigned int m_unTileEntityId;
	unsigned int m_unSphereEntityId;

	GameOverlay* m_pOverlay;
	DiamondGame* m_pGame;
};