#include "GameInput.h"
#include "DiamondGame.h"
#include "IDiamondScene.h"
#include "IEngineInput.h"
#include "Camera.h"
#include "ITouchTarget.h"
#include "SceneActions.h"
#include "Textures.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

GameInput::GameInput(DiamondGame* pGame, IDiamondScene* pScene) {
    m_pGame = pGame;
    m_pScene = pScene;
    m_bFirstMouse = true;
    m_fLastMouseX = 0.0f;
    m_fLastMouseY = 0.0f;
    m_bControllerConnected = false;
    m_unConnectedControllerId = 0;
    m_bGlyphsLoaded = false;

    m_bKeyboardActionsRegistered = false;
    m_bMouseActionsRegistered = false;
    m_bMouseActionsRegistered = false;
    m_bControlActionsRegistered = false;
}

GameInput::~GameInput() {
    m_pEngineInput = nullptr;
}

// IGameInput implementaton
//
void GameInput::SetEngineInput(IEngineInput* pInput) {
    m_pEngineInput = pInput;
}

bool GameInput::RegisterInputs(IEngineInput* pEngineInput) {
    if (!m_bKeyboardActionsRegistered) {
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_UnpauseGame, eControllerActionSet_MenuControls, static_cast<char>(27), eDAT_EventOnRelease);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_Left, eControllerActionSet_MenuControls, 'A', eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_Right, eControllerActionSet_MenuControls, 'D', eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_Up, eControllerActionSet_MenuControls, 'W', eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_Down, eControllerActionSet_MenuControls, 'S', eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_PrevTab, eControllerActionSet_MenuControls, GLFW_KEY_LEFT, eDAT_EventAutoRepeat);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_NextTab, eControllerActionSet_MenuControls, GLFW_KEY_RIGHT, eDAT_EventAutoRepeat);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_PauseMenu, eControllerActionSet_ShipControls, '1', eDAT_EventOnRelease);

        pEngineInput->RegisterKeyboardAction(eDigitalInput_Quit, eControllerActionSet_ShipControls, static_cast<char>(27), eDAT_EventOnRelease);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Left, eControllerActionSet_ShipControls, 'A', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Right, eControllerActionSet_ShipControls, 'D', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Forward, eControllerActionSet_ShipControls, 'W', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Backward, eControllerActionSet_ShipControls, 'S', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Up, eControllerActionSet_ShipControls, 'F', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Down, eControllerActionSet_ShipControls, 'C', eDAT_EventContinuous);
        pEngineInput->RegisterKeyboardAction(eDigitalInput_Slower, eControllerActionSet_ShipControls, GLFW_KEY_LEFT_SHIFT, eDAT_EventContinuous);

        pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_Ok, eControllerActionSet_MenuControls, static_cast<char>(13), eDAT_EventOnRelease);
        m_bKeyboardActionsRegistered = true;
    }

    if (!m_bMouseActionsRegistered) {
        m_bMouseActionsRegistered = true;
    }
    if (!m_bTouchActionsRegistered) {
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_ChooseTab, eDAT_EventOnRelease);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Highlight, eDAT_EventOnRelease);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_DisplayAlert, eDAT_EventOnRelease);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_UnpauseGame, eDAT_EventOnRelease);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_PrevTab, eDAT_EventAutoRepeat);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_NextTab, eDAT_EventAutoRepeat);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Left, eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Right, eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Up, eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Down, eDAT_EventAutoRepeatTwoLevels);
        pEngineInput->RegisterTouchAction(eDigitalInput_Menu_Ok, eDAT_EventOnRelease);

        m_bTouchActionsRegistered = true;

    }
    pEngineInput->RegisterMouseAction(eDigitalInput_LockCamera, eControllerActionSet_ShipControls, GLFW_MOUSE_BUTTON_LEFT, eDAT_EventOnRelease);
    pEngineInput->RegisterMouseAction(eDigitalInput_ShowDepthMap, eControllerActionSet_ShipControls, GLFW_MOUSE_BUTTON_RIGHT, eDAT_EventOnRelease);

    if (!pEngineInput->RegisterActionSet(eControllerActionSet_ShipControls, "InGameControls")) {
        return false;
    }
    if (!pEngineInput->RegisterActionSet(eControllerActionSet_MenuControls, "MenuControls")) {
        return false;
    }
    pEngineInput->RegisterKeyboardAction(eDigitalInput_Menu_UnpauseGame, eControllerActionSet_MenuControls, char(27), eDAT_EventOnRelease);

    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Fire, eControllerActionSet_ShipControls, "fire", eDAT_EventOnRelease)) { 
        return false;  
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_PauseMenu, eControllerActionSet_ShipControls, "pause_menu", eDAT_EventOnRelease)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_ShowDepthMap, eControllerActionSet_ShipControls, "show_depth_map", eDAT_EventOnRelease)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_LockCamera, eControllerActionSet_ShipControls, "lock_camera", eDAT_EventOnRelease)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerAnalogAction(eAnalogInput_Camera, eControllerActionSet_ShipControls, "camera")) {
        return false;
    }
    if (!pEngineInput->RegisterControllerAnalogAction(eAnalogInput_Move, eControllerActionSet_ShipControls, "move")) {
        return false;
    }

    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_UnpauseGame, eControllerActionSet_MenuControls, "unpause_menu", eDAT_EventOnRelease)) {
        return false;
    }

    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_PrevTab, eControllerActionSet_MenuControls, "menu_prevtab", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_NextTab, eControllerActionSet_MenuControls, "menu_nexttab", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_Left, eControllerActionSet_MenuControls, "menu_left", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_Right, eControllerActionSet_MenuControls, "menu_right", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_Up, eControllerActionSet_MenuControls, "menu_up", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_Down, eControllerActionSet_MenuControls, "menu_down", eDAT_EventAutoRepeat)) {
        return false;
    }
    if (!pEngineInput->RegisterControllerDigitalAction(eDigitalInput_Menu_Ok, eControllerActionSet_MenuControls, "menu_ok", eDAT_EventOnRelease)) {
        return false;
    }

    m_bControlActionsRegistered = true;

    return true;
}

void GameInput::ProcessInput(IEngineInput* pEngineInput, float fDeltaTime) {
    if (m_eState == eGS_Menu) {
        ProcessMenuInput(pEngineInput, fDeltaTime);
    }
    else {
        ProcessGameInput(pEngineInput, fDeltaTime);
    }
}

void GameInput::ProcessMouseMovement(float fXPos, float fYPos) {
    if (m_bFirstMouse)
    {
        m_fLastMouseX = fXPos;
        m_fLastMouseY = fYPos;
        m_bFirstMouse = false;
    }

    float xoffset = fXPos - m_fLastMouseX;
    float yoffset = m_fLastMouseY - fYPos;
    m_fLastMouseX = fXPos;
    m_fLastMouseY = fYPos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
}

void GameInput::ProcessMouseButton(int button, int action, int mods) {
    if (m_eState == eGS_Menu) {
        /*bool bPress = true;
        if (action == GLFW_PRESS) {
            bPress = true;
        }
        else if (action == GLFW_RELEASE) {
            bPress = false;
        }
        else {
            return;
        }*/
        //std::cout << "Button pressed at (" << m_fLastMouseX << ", " << m_fLastMouseY << ")" << std::endl;
    }
}

void GameInput::ProcessMouseScroll(float fScroll) {
    m_pScene->ProcessAnalogAction(eSceneAction_Zoom, -fScroll, 0.0f);
}

void GameInput::ProcessKeyEvent(int nKey, int nScancode, int nAction, int nMods) {
}

void GameInput::ControllerConnected(unsigned int unActiveControllerId) {
    bool bReloadGlyphs = false;
    if (m_unConnectedControllerId != unActiveControllerId) {
        // Controller changes - reload glyphs
        bReloadGlyphs = true;
    }
    m_bControllerConnected = true;
    m_unConnectedControllerId = unActiveControllerId;;
    m_pScene->ControllerConnected();
    if (!m_bGlyphsLoaded || bReloadGlyphs) {
        m_bGlyphsLoaded = LoadGlyphs();
    }
    m_pGame->ControllerConnected(m_bControllerConnected);
}

void GameInput::ControllerReconnected(unsigned int unActiveControllerId) {
    m_bControllerConnected = true;
    m_unConnectedControllerId = unActiveControllerId;;
    m_pScene->ControllerConnected();
    m_pGame->ControllerConnected(m_bControllerConnected);
}

bool GameInput::LoadGlyphs() {
    unsigned int unActionSet = eControllerActionSet_MenuControls; 
    int nWidth = 128;

    unsigned int nDigitalActionCount;
    unsigned int nAnalogActionCount;
    m_pEngineInput->GetActionCountForActionSet(unActionSet, &nDigitalActionCount, &nAnalogActionCount);
    if (nDigitalActionCount == 0 && nAnalogActionCount == 0) {
        return false;
    }
    
    m_pEngineInput->LoadGlypthsForActionSet(unActionSet, nWidth);
    Textures* tc = m_pGame->GetTextureCtrl();
    if (m_bGlyphsLoaded) {
        m_pGame->SetGlyphTextureAtlasId(0);
        tc->DeleteTextureAtlas(m_unGlyphTextureAtlas);;
    }
    m_unGlyphTextureAtlas = tc->CreateTextureAtlas(nWidth, nWidth, 8, 5, GL_RGBA);;

    for (unsigned int n = 0; n < nDigitalActionCount; ++n) {
        unsigned int unAction = m_pEngineInput->GetDigitalActionAtIndexForActionSet(unActionSet, n);
        if (unAction == eDigitalInput_Menu_Up) {
            int a = 1;
            a++;

        }
        const char* pzGlypthFilepath = m_pEngineInput->GetGlyphFilenameForDigitalAction(unAction, unActionSet, nWidth);
        if (pzGlypthFilepath != nullptr) {
            tc->AddTextureToAtlas(m_unGlyphTextureAtlas, pzGlypthFilepath, unAction);
        }
    }

    for (unsigned int n = 0; n < nAnalogActionCount; ++n) {
        unsigned int unAction = m_pEngineInput->GetAnalogActionAtIndexForActionSet(unActionSet, n);
        const char* pzGlypthFilepath = m_pEngineInput->GetGlyphFilenameForAnalogAction(unAction, unActionSet, nWidth);
        if (pzGlypthFilepath != nullptr) {
            tc->AddTextureToAtlas(m_unGlyphTextureAtlas, pzGlypthFilepath, unAction);
        }
    }
    m_pGame->SetGlyphTextureAtlasId(m_unGlyphTextureAtlas);
    
    return true;
}

void GameInput::ControllerDisconnected() {
    m_bControllerConnected = false;
    m_pGame->ControllerConnected(m_bControllerConnected);
}
//
// IGameInput implementaton

void GameInput::ProcessGameInput(IEngineInput* pEngineInput, float fDeltaTime) {
    if (!m_bGlyphsLoaded) {
        m_bGlyphsLoaded = LoadGlyphs();
    }
    pEngineInput->SetSteamControllerActionSet(eControllerActionSet_ShipControls);
    float cameraSpeed = static_cast<float>(2.5 * fDeltaTime);
    //float fDeltaTime = m_pWnd->GetDeltaTime();
    ITouchTarget* pTarget = nullptr;
    int nInvokeValue = 0;
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Quit, &pTarget, &nInvokeValue)) {
        m_pScene->ProcessDigitalAction(eSceneAction_Quit, fDeltaTime);
        return;
    }

    if (pEngineInput->IsDigitalActionActive(eDigitalInput_PauseMenu, &pTarget, &nInvokeValue)) {
        m_pScene->ProcessDigitalAction(eSceneAction_Pause, fDeltaTime);
		m_eState = eGS_Menu;
        return;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Fire, &pTarget, &nInvokeValue)) {
        m_pScene->ProcessDigitalAction(eSceneAction_Fire, fDeltaTime);
    }
	if (pEngineInput->IsDigitalActionActive(eDigitalInput_ShowDepthMap, &pTarget, &nInvokeValue)) {
		m_pScene->ProcessDigitalAction(eSceneAction_ShowDepthMap, fDeltaTime);
	}
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_LockCamera, &pTarget, &nInvokeValue)) {
        m_pScene->ProcessDigitalAction(eSceneAction_LockCamera, fDeltaTime);
    }

    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Accelerate, &pTarget, &nInvokeValue)) {
        std::cout << "Accelerate" << std::endl;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Decelerate, &pTarget, &nInvokeValue)) {
        std::cout << "Decelerate" << std::endl;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Fire, &pTarget, &nInvokeValue)) {
    }

    bool keyMovement = false;
    bool upDownMovement = false;
    float dx = 0.0f;
    float dz = 0.0f;
    float dy = 0.0f;
    float speed = 1.0;
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Slower, &pTarget, &nInvokeValue)) {
        speed = speed / 5.0f;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Forward, &pTarget, &nInvokeValue)) {
        dz += speed;
        keyMovement = true;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Backward, &pTarget, &nInvokeValue)) {
        dz -= speed;
        keyMovement = true;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Left, &pTarget, &nInvokeValue)) {
        dx -= speed;
        keyMovement = true;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Right, &pTarget, &nInvokeValue)) {
        dx += speed;
        keyMovement = true;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Up, &pTarget, &nInvokeValue)) {
        dy += speed;
        upDownMovement = true;
    }
    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Down, &pTarget, &nInvokeValue)) {
        dy -= speed;
        upDownMovement = true;
    }

    float x;
    float y;
    if (keyMovement) {
        ProcessAnalogMovement(dx, dz, fDeltaTime);
    }
    else if (m_bControllerConnected) {
        pEngineInput->GetControllerAnalogAction(eAnalogInput_Move, &x, &y);
        if (std::abs(x) > 0.01f || std::abs(y) > 0.01f) {
            ProcessAnalogMovement(x, y, fDeltaTime);
        }
    }
    if (m_bControllerConnected) {
        pEngineInput->GetControllerAnalogAction(eAnalogInput_Camera, &x, &y);
    }
    else {
        x = 0.0f;
        y = 0.0f;
    }
    if (std::abs(x) > 0.01f || std::abs(y) > 0.01f || upDownMovement) {
        ProcessAnalogCameraInput(x, dy, y, fDeltaTime);
    }
}

void GameInput::ProcessMenuInput(IEngineInput* pEngineInput, float fDeltaTime) {
    pEngineInput->SetSteamControllerActionSet(eControllerActionSet_MenuControls);
    ITouchTarget* pTarget = nullptr;
    int nInvokeValue = 0;

    if (pEngineInput->IsDigitalActionActive(eDigitalInput_Menu_UnpauseGame, &pTarget, &nInvokeValue)) {
        // If multiple UIs present on screen, would pass a handle here
        if (m_pGame->CanCloseUI()) {
            m_pGame->ProcessMenuDigitalAction(eDigitalInput_Menu_UnpauseGame, fDeltaTime);
            m_eState = eGS_Game;
            // It is possible an ongoing touch event inside a text-input field, could cause:
            //  a: the steamdeck keyboard to stay active
            //  b: OS keyboard events to be directed to a textinput field rather than processed by the game loop
            // Cancel any ongoing touch events
            m_pEngineInput->CancelAllEvents();
            return;
        }
    }

    EDigitalInput inputs[] = { eDigitalInput_Menu_Left,eDigitalInput_Menu_Right, eDigitalInput_Menu_Up,eDigitalInput_Menu_Down,eDigitalInput_Menu_PrevTab, eDigitalInput_Menu_NextTab,eDigitalInput_Menu_ChooseTab, eDigitalInput_Menu_Highlight, eDigitalInput_Menu_DisplayAlert, eDigitalInput_Menu_Ok };

    for (EDigitalInput e : inputs) {
        pTarget = nullptr;

        if (pEngineInput->IsDigitalActionActive(e, &pTarget, &nInvokeValue)) {
            if (pTarget != nullptr) {
                pTarget->Invoke(e, nInvokeValue, fDeltaTime);
            }
            else {
                ActionResult result = m_pGame->ProcessMenuDigitalAction(e, fDeltaTime);
                if (result.m_bSuccess && result.m_nRetryWithTargetAction != -1) {
                    // Button press in UI requested different action to be processed
                    m_pGame->ProcessMenuDigitalAction(static_cast<EDigitalInput>(result.m_nRetryWithTargetAction), fDeltaTime);
                    if (result.m_nRetryWithTargetAction == eDigitalInput_Menu_UnpauseGame) {
                        m_eState = eGS_Game;
                        return;
                    }
                }
            }
        }
    }
}

void GameInput::ProcessAnalogCameraInput(float fX, float fY,float fZ, float fDeltaTime) {
    m_pScene->ProcessAnalogAction(eSceneAction_MoveCamera, fX / 10.0f, fY, fZ/10.0f, fDeltaTime);
}

void GameInput::ProcessAnalogMovement(float fX, float fZ, float fDeltaTime) {
    m_pScene->ProcessAnalogAction(eSceneAction_Move, fX * 4.0f, fZ * 4.0f , fDeltaTime);
}