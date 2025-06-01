#pragma once
#include "IGameInput.h"

class Camera;
class DiamondGame;

enum EDigitalInput
{
    eDigitalInput_Accelerate,
    eDigitalInput_Decelerate,
    eDigitalInput_Fire,
    eDigitalInput_PauseMenu,
    eDigitalInput_ShowDepthMap,
    eDigitalInput_LockCamera,

    eDigitalInput_Menu_UnpauseGame,
    eDigitalInput_Menu_NextTab,
    eDigitalInput_Menu_PrevTab,
    eDigitalInput_Menu_ChooseTab,
    eDigitalInput_Menu_Highlight,
    eDigitalInput_Menu_Left,
    eDigitalInput_Menu_Right,
    eDigitalInput_Menu_Up,
    eDigitalInput_Menu_Down,

    eDigitalInput_Menu_DisplayAlert,

    eDigitalInput_Menu_Ok,
    
    eDigitalInput_Left,
    eDigitalInput_Right,
    eDigitalInput_Forward,
    eDigitalInput_Backward,
    eDigitalInput_Up,
    eDigitalInput_Down,
    eDigitalInput_Quit,
    eDigitalInput_Slower
};

enum EAnalogInput {
    eAnalogInput_Camera,
    eAnalogInput_Move
};

enum EControllerActionSet
{
    eControllerActionSet_ShipControls,
    eControllerActionSet_MenuControls
};

enum EGameState {
    eGS_Game,
    eGS_Menu
};

class Window;
class IDiamondScene;

class GameInput :
    public IGameInput
{
public:
    GameInput(DiamondGame* pGame, IDiamondScene* pScene);
    virtual ~GameInput();

public:
    // IGameInput declarations
    virtual void SetEngineInput(IEngineInput* pInput);
    virtual bool RegisterInputs(IEngineInput* pEngineInput);
    virtual void ProcessInput(IEngineInput* pEngineInput, float fDeltaTime);
    virtual void ProcessMouseMovement(float fXPos, float fYPos);
    virtual void ProcessMouseButton(int button, int action, int mods);
    virtual void ProcessMouseScroll(float fScroll);
    virtual void ProcessKeyEvent(int nKey, int nScancode, int nAction, int nMods);
    virtual void ControllerConnected(unsigned int unActiveControllerId);
    virtual void ControllerReconnected(unsigned int hActiveControllerId);
    virtual void ControllerDisconnected();


    //void SetGameState(EGameState);

protected:
    void ProcessAnalogCameraInput(float fX, float fY, float fZ, float fDeltaTime);
    void ProcessAnalogMovement(float fX, float fZ, float fDeltaTime);

    void ProcessMenuInput(IEngineInput* pEngineInput, float fDeltaTime);
    void ProcessGameInput(IEngineInput* pEngineInput, float fDeltaTime);

    bool LoadGlyphs();

protected:
    IDiamondScene* m_pScene;
    DiamondGame* m_pGame;
    IEngineInput* m_pEngineInput;

    bool m_bGlyphsLoaded;
    unsigned int m_unGlyphTextureAtlas;

    bool m_bControllerConnected;
    unsigned int m_unConnectedControllerId;
    bool m_bFirstMouse;
    float m_fLastMouseX;
    float m_fLastMouseY;

    bool m_bMouseActionsRegistered;
    bool m_bKeyboardActionsRegistered;
    bool m_bTouchActionsRegistered;
    bool m_bControlActionsRegistered;

    EGameState m_eState;
};
