#pragma once
#include "IGame.h"
#include "OverlayDefs.h"
#include "ITextRenderer.h"
#include "SceneActions.h"
#include "GameInput.h"
#include <tuple>

class DiamondScene;
class GameOverlay;
class IOverlay;
class IOverlayEntity;
class IGameInput;
class GameInput;
class IWindow;
class IOverlayView;

class DiamondGame :
    public IGame
{
public:
    DiamondGame();
    virtual ~DiamondGame();
    virtual void Initialise(ITextRenderer* pTextRenderer, Textures* pTextureCtrl, IWindow* pWnd);
    virtual void InitialiseUI();
    virtual void Deinitialise();
    virtual void WindowSizeChanged(int nWidth, int nHeight);
    virtual void ReleaseEngineResources();
    virtual void RenderOverlay(float fDeltaTime);
    virtual IScene* GetScene();
    virtual IGameInput* GetGameInput();

    void SetGlyphTextureAtlasId(unsigned int unGlyphAtlasId);
    void GlyphFilepathForAction(unsigned int unAction, unsigned int unActionSet, const char* pzFilename);

    void DisplayUI(bool bDisplay);

    void ProcessGameDigitalAction(SceneAction eAction, float fDeltaTime);
    ActionResult ProcessMenuDigitalAction(EDigitalInput eAction, float fDeltaTime);
    void ControllerConnected(bool bConnected);
    bool CanCloseUI();

    Textures* GetTextureCtrl() { return m_pTextureCtrl; }
    static bool InvokeTargetAction(unsigned int unTargetHandle, int nAction, unsigned int unInvokeValue);

private:
    void CreateUI(IWindow* pWnd, std::tuple<float, float, float> rgbText, FontFaceHandle fontFace);
    void CreateSampleUI();
    void CreateSampleUI_SingleTab();
    bool InvokeAction(int nAction, unsigned int unInvokeValue);
    void CreateOverlay(std::tuple<float, float, float> rgbText, FontFaceHandle fontFace);

private:
    IWindow* m_pWindow;
    DiamondScene* m_pScene;
    GameOverlay* m_pOverlay;
    Textures* m_pTextureCtrl;
    unsigned int m_unGlyphAtlasId;

    bool m_bDisplayUI;
    IOverlay* m_pUI;
    IOverlay* m_pFullScreenOverlay;
    ITextRenderer* m_pTextRenderer;
    GameInput* m_pGameInput;

    IOverlayEntity* m_pPositionLabel;
    IOverlayEntity* m_pYawLabel;
    IOverlayEntity* m_pPitchLabel;

    IOverlayView* m_pViewDialog;

    int m_nNumber[12];

    static DiamondGame* s_pGame;
    std::string m_sName;
    std::string m_sNickname;
};

