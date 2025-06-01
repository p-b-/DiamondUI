#include "EngineInput.h"
#include "Window.h"
#include "IGameInput.h"
#include "TouchLayer.h"
#include "DigitalActions.h"
#include "Overlay.h"
#include "InputCharSet.h"
#include "ITextInputTarget.h"

#include <direct.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#ifdef _DEBUG
int gnActionSetError = 0;
#endif

#define INTERNAL_ACTION_START 65500

void char_callback(GLFWwindow* pGLWnd, unsigned int unCodepoint);
void key_callback(GLFWwindow* pGLWnd, int nKey, int nScancode, int nAction, int nMods);
void mouse_callback(GLFWwindow* pGLWnd, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* pGLWnd, int button, int action, int mods);
void scroll_callback(GLFWwindow* pGLWnd, double xoffset, double yoffset);
void focus_callback(GLFWwindow* pGLWnd, int focused);

bool LogEvery(int& count, int every) {
    if (count == -1) {
        count = 0;
        return true;
    }
    ++count;
    if (++count == every) {
        count = 0;
        return true;
    }
    return false;
}

int gLogSetActionSet = -1;
int gLogGetControllerAnalogAction = -1;

EngineInput::EngineInput(Window* pWnd, IGameInput* pGameInput) {
    m_pWnd = pWnd;
    m_pGameInput = pGameInput;
    m_hActiveController = 0;
    m_hPrevActiveController = 0;
    m_bInputConfigRead = false;
    m_pDigitalActions = new DigitalActions(pWnd, INTERNAL_ACTION_START);

    m_unNextControllerId = 1;
    m_nPrevActiveControllerCount = 0;
    m_unActiveControllerId = 0;

    m_pWnd->SetMouseCallback(mouse_callback);
    m_pWnd->SetMouseButtonCallback(mouse_button_callback);
    m_pWnd->SetScrollCallback(scroll_callback);
    m_pWnd->SetKeyCallback(key_callback);
    m_pWnd->SetCharCallback(char_callback);
    m_pWnd->SetFocusCallback(focus_callback);
    m_pWnd->SetInput(this);

    m_pGameInput->SetEngineInput(this);
    TouchLayer::SetEngineInput(this);

    m_unCurrentActionSet = 0;
    m_unCurrentActionSetForPressAction = 0;
    m_nInvokeValueForCurrentPressAction = -1;
    m_unCurrentPressAction = 0;

    // TODO Overlay system should register these actions, as they're internal to it. However, overlay system is used before EngineInput is created, so cannot request for registration
    //  The order is because the game is created first, which creates the overlays. The game creates the GameInput, which is then passed to this constructor.
    // One solution would be to have the game create its overlay at a later point.
    RegisterInternalTouchAction(INTERNAL_ACTION_START+0, eDAT_EventOnRelease);
    RegisterInternalTouchAction(INTERNAL_ACTION_START+1, eDAT_EventOnRelease);
    RegisterInternalTouchAction(INTERNAL_ACTION_START+2, eDAT_EventOnRelease);
    RegisterInternalTouchAction(INTERNAL_ACTION_START+3, eDAT_EventOnRelease);

    m_eActiveCharSet = eCS_None;
    m_pTextInputTarget = nullptr;
    m_bKeyboardDisplayed = false;
    RegisterInputCharSets();
}

EngineInput::~EngineInput() {
    m_lsTouchLayers.clear();

    for (auto as : m_mpActionSetIdToActionSets) {
        delete as.second;
    }
    m_mpActionSetIdToActionSets.clear();
    delete m_pDigitalActions;
    m_pDigitalActions = nullptr;

    for (auto cs : m_mpInputCharSets) {
        delete cs.second;
    }
    m_mpInputCharSets.clear();
}

// IEngineInputIntl implementation
//
bool EngineInput::Initialise() {
    if (!SteamInput()->Init(false))
    {
        return false;
    }
    m_hActiveController = 0;
    m_bInputConfigRead = ReadConfigFile();
    if (m_bInputConfigRead) {
        std::cout << "Read controller config on startup" << std::endl;
    }
    SetManifestFileLocation();

    return true;
}

void EngineInput::DeinitialiseAndDelete() {
    SteamInput()->Shutdown();
    m_lsTouchLayers.clear();
    delete this;
}

bool EngineInput::RegisterInternalTouchAction(unsigned int unAction, DigitalActionType eActionType) {
    m_vcInternalActions.push_back(unAction);
    return m_pDigitalActions->RegisterInternalTouchAction(unAction, eActionType);
}

void EngineInput::ProcessInput(float fDeltaTime) {
    PollSteamInput();
    ProcessInternalActions();
    m_pGameInput->ProcessInput(this, fDeltaTime);
}

void EngineInput::ShowBindingPanel() {
    SteamInput()->ShowBindingPanel(m_hActiveController);
}

void EngineInput::AddTouchLayer(ITouchLayer* pTouchLayer) {
    m_pDigitalActions->AddTouchLayer(pTouchLayer);
    m_lsTouchLayers.push_front(pTouchLayer);
    AlterTopWindowTouchLayerCount(true);
}

void EngineInput::RemoveTouchLayer(ITouchLayer* pTouchLayer) {
    m_pDigitalActions->RemoveTouchLayer(pTouchLayer);
    auto find = std::find(m_lsTouchLayers.begin(), m_lsTouchLayers.end(), pTouchLayer);
    if (find != m_lsTouchLayers.end()) {
        m_lsTouchLayers.erase(find);
    }
    AlterTopWindowTouchLayerCount(false);
}

void EngineInput::NewWindowLayer() {
    m_stLayersPerWindow.push(0);
}

void EngineInput::AlterTopWindowTouchLayerCount(bool bIncNotDec) {
    unsigned int nCurrentCount = 0;
    if (m_stLayersPerWindow.size() > 0) {
        nCurrentCount = m_stLayersPerWindow.top();
        m_stLayersPerWindow.pop();
    }
    nCurrentCount += bIncNotDec ? 1 : -1;
    m_stLayersPerWindow.push(nCurrentCount);
}

void EngineInput::RemoveTopWindowLayer(bool bLayersAlreadyRemoved) {
    if (m_stLayersPerWindow.size() == 0) {
        // TODO Log error
        return;
    }
    int nCurrentCount = m_stLayersPerWindow.top();
    m_stLayersPerWindow.pop();
    if (!bLayersAlreadyRemoved) {
        for (int n = 0; n < nCurrentCount; ++n) {
            if (m_lsTouchLayers.size() > 0) {
                // This object does not own the touch layers so does not delete them
                m_lsTouchLayers.pop_front();
            }
            else {
                // TODO Log error
            }
        }
    }
}

void EngineInput::ActivateTopWindowLayer(bool bActivateNotDeactivate) {
    if (m_stLayersPerWindow.size() == 0) {
        return;
    }
    int nLayerCount = m_stLayersPerWindow.top();
    if (nLayerCount > m_lsTouchLayers.size()) {
        // TODO Log error or set error condition
        return;
    }
    int n = 0;
    for (auto l : m_lsTouchLayers) {
        l->SetLayerActive(bActivateNotDeactivate);
        n++;
        if (n == nLayerCount) {
            break;
        }
    }
}

bool EngineInput::ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTextInputTarget, const IntRect& rcInputArea) {
    bool bDisplayKeyboard = false;
    if (rcInputArea.m_nWidth!=0) {
        bDisplayKeyboard = true;
    }

    if (bDisplayKeyboard && m_bKeyboardDisplayed) {
#ifdef _DEBUG
        std::cout << "Attempt to display keyboard when it is already displayed" << std::endl;
        m_pDigitalActions->OutputCurrentAction();
#endif
        return false;
    }
    bool bKeyboardDisplayed = false;
    if (bDisplayKeyboard) {
        m_pDigitalActions->CancelCurrentTouchEvent(true);
        m_bKeyboardDisplayed = m_pWnd->ShowKeyboard(rcInputArea.m_nXOrigin, rcInputArea.m_nYOrigin, rcInputArea.m_nWidth, rcInputArea.m_nHeight);
    }
    else {
        m_pWnd->HideKeyboard();
    }
    m_eActiveCharSet = eCharSet;
    m_pTextInputTarget = pTextInputTarget;
    return m_bKeyboardDisplayed;
}

void EngineInput::KeyboardDismissed() {
    m_bKeyboardDisplayed = false;
    if (m_pTextInputTarget != nullptr) {
        m_pTextInputTarget->KeyboardDismissed();
    }
#ifdef _DEBUG
    else {
        std::cout << "Null text input target, cannot send keyboard dismissed callback" << std::endl;
    }
#endif
    m_pTextInputTarget = nullptr;
}
//
// IEngineInputIntl implementation

void EngineInput::SetManifestFileLocation() {
    char rgchCWD[1024];
    if (!_getcwd(rgchCWD, sizeof(rgchCWD)))
    {
        strcpy(rgchCWD, ".");
    }

    char rgchFullPath[1024];
#if defined(OSX)
    // hack for now, because we do not have utility functions available for finding the resource path
    // alternatively we could disable the SteamController init on OS X
    _snprintf(rgchFullPath, sizeof(rgchFullPath), "%s/steamworksexample.app/Contents/Resources/%s", rgchCWD, "input_manifest.vdf");
#else
    _snprintf(rgchFullPath, sizeof(rgchFullPath), "%s\\%s", rgchCWD, "input_manifest.vdf");
#endif

    SteamInput()->SetInputActionManifestFilePath(rgchFullPath);
}

bool EngineInput::ReadConfigFile() {
    return m_pGameInput->RegisterInputs(this);
}

void EngineInput::RegisterInputCharSets() {
    for (int i = 0; i < 2; ++i) {
        CharSet eCS = eCS_None;
        if (i == 0) {
            eCS = eCS_Text;
        }
        else if (i == 1) {
            eCS = eCS_Numeric;
        }
        InputCharSet* pCS = new InputCharSet(eCS);
        if (eCS == eCS_Text) {
            pCS->AddCharRange('A', 'Z');
            pCS->AddCharRange('a', 'z');
            pCS->AddChar('_');
            pCS->AddChar(' ');
        }
        pCS->AddCharRange('0', '9');

        pCS->KeyIsProcessed(eTIK_LeftArrow, ePS_Process);
        pCS->KeyIsProcessed(eTIK_RightArrow, ePS_Process);
        pCS->KeyIsProcessed(eTIK_Home, ePS_Process);
        pCS->KeyIsProcessed(eTIK_End, ePS_Process);
        pCS->KeyIsProcessed(eTIK_Backspace, ePS_Process);
        pCS->KeyIsProcessed(eTIK_Delete, ePS_Process);

        m_mpInputCharSets[eCS] = pCS;
    }
}

InputCharSet* EngineInput::GetCharSet(CharSet eCS) {
    auto find = m_mpInputCharSets.find(m_eActiveCharSet);
    if (find == m_mpInputCharSets.end()) {
        return nullptr;
    }
    return find->second;
}

void EngineInput::PollSteamInput() {
    // There's a bug where the action handles aren't non-zero until a config is done loading. Soon config
    // information will be available immediately. Until then try to init as long as the handles are invalid.
    if (!m_bInputConfigRead ||
        m_pDigitalActions->GetOrCreateActionSet(m_unCurrentActionSet, false)==nullptr) {
    //if (!m_bInputConfigRead && m_mpControllerDigitalActionHandles.size() == 0) {
        if (ReadConfigFile()) {
            std::cout << "EngineInput::PollSteamInput() Read config file" << std::endl;
            m_bInputConfigRead = true;
        }
    }
    auto connectionAction = FindActiveSteamInputDevice();

    switch (connectionAction) {
    case eCCA_ControllerConnected:
        if (!m_bInputConfigRead && ReadConfigFile()) {
            std::cout << "EngineInput::PollSteamInput() Read config file after finding controller" << std::endl;
            m_bInputConfigRead = true;
        }
        m_pGameInput->ControllerConnected(m_unActiveControllerId);
        break;
    case eCCA_ControllerDisconnected:
        std::cout << "Controller disconnected" << std::endl;
        m_hActiveController = 0;
        m_pDigitalActions->SetActiveController(m_hActiveController);
        m_bInputConfigRead = false;
        m_pGameInput->ControllerDisconnected();
        break;
    case eCCA_ControllerReconnected:
        std::cout << "Previous controller reconnected" << std::endl;
        // TODO Determine if we need to re-read the config file for reconnections
        m_bInputConfigRead = ReadConfigFile();
        if (m_bInputConfigRead) {
            std::cout << "Re-read controller config" << std::endl;
        }
        m_pGameInput->ControllerReconnected(m_unActiveControllerId);
        break;
    case eCCA_ControllerChanged:
        std::cout << "Controller changed" << std::endl;
        m_bInputConfigRead = ReadConfigFile();
        if (m_bInputConfigRead) {
            std::cout << "Re-read controller config" << std::endl;
        }
        m_pGameInput->ControllerConnected(m_unActiveControllerId);
        break;
    }
}

void EngineInput::CheckKey(int keyToCheck, const std::string& keyId) {
    int v = m_pWnd->GetKey(keyToCheck);
}

EngineInput::ConnectorConnectionAction EngineInput::FindActiveSteamInputDevice() {
    InputHandle_t currentController = m_hActiveController;

    // Use the first available steam controller for all interaction. We can call this each frame to handle
    // a controller disconnecting and a different one reconnecting. Handles are guaranteed to be unique for
    // a given controller, even across power cycles.

    // See how many Steam Controllers are active. 
    InputHandle_t pHandles[STEAM_CONTROLLER_MAX_COUNT];
    int nNumActive = SteamInput()->GetConnectedControllers(pHandles);
    if (nNumActive != m_nPrevActiveControllerCount) {
        PopulateControllerIdMapFromConnectedArray(pHandles, nNumActive);
    }
    if (nNumActive == 0 && m_hActiveController!=0) {
        std::cout << "Controller disconnected" << std::endl;
        m_hActiveController = 0;
        m_pDigitalActions->SetActiveController(m_hActiveController);
        m_nPrevActiveControllerCount = 0;
        m_unActiveControllerId = 0;
        return eCCA_ControllerDisconnected;
    }

    // If there's an active controller, and if we're not already using it, select the first one.
    if (nNumActive && (m_hActiveController != pHandles[0])) {
        m_hActiveController = pHandles[0];
        m_pDigitalActions->SetActiveController(m_hActiveController);
        m_unActiveControllerId = m_mpIControllerIdToId[m_hActiveController];
        if (currentController == 0 && m_hPrevActiveController == pHandles[0]) {
            return eCCA_ControllerReconnected;
        }

        if (currentController == 0) {
            std::cout << "Found controller" << std::endl;
            m_hPrevActiveController = m_hActiveController;

            return eCCA_ControllerConnected;
        }
        else {
            std::cout << "Controller changed" << std::endl;
            return eCCA_ControllerChanged;
        }
    }
    if (nNumActive > 0) {
        return eCCA_ControllerStillConnected;
    }
    else {
        m_unActiveControllerId = 0;
        return eCCA_NoController;
    }
}

void EngineInput::PopulateControllerIdMapFromConnectedArray(InputHandle_t* pHandles, int nCount) {
    for (int i = 0; i < nCount; ++i) {
        auto find = m_mpIControllerIdToId.find(pHandles[i]);
        if (find == m_mpIControllerIdToId.end()) {
            unsigned int unNewControllerId = m_unNextControllerId++;
            m_mpIControllerIdToId[pHandles[i]] = unNewControllerId;
        }
    }
}

bool EngineInput::RegisterControllerDigitalAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction, DigitalActionType eActionType) {
    return m_pDigitalActions->RegisterControllerDigitalAction(unAction, unActionSet, pszAction, eActionType);
}

bool EngineInput::RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, int nGLFWKeyToken, DigitalActionType eActionType) {
    return m_pDigitalActions->RegisterKeyboardAction(unAction, unActionSet, nGLFWKeyToken, eActionType);
}

bool EngineInput::RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, char chKey, DigitalActionType eActionType) {
    return m_pDigitalActions->RegisterKeyboardAction(unAction, unActionSet, chKey, eActionType);
}

bool EngineInput::RegisterMouseAction(unsigned int unAction, unsigned int unActionSet, int nGLFWMouseToken, DigitalActionType eActionType) {
    return m_pDigitalActions->RegisterMouseAction(unAction, unActionSet, nGLFWMouseToken, eActionType);
}

bool EngineInput::RegisterTouchAction(unsigned int unAction, DigitalActionType eActionType) {
    return m_pDigitalActions->RegisterTouchAction(unAction, eActionType);
}

bool EngineInput::RegisterControllerAnalogAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction) {
    InputAnalogActionHandle_t handle = SteamInput()->GetAnalogActionHandle(pszAction);
    if (handle != 0) {
        ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
        if (pActionSet == nullptr) {
            std::cerr << "Register action set before registering (analog) actions" << std::endl;
            return false;
        }
        pActionSet->RegisterControllerAnalogAction(unAction, handle);
        return true;
    }
    return false;
}

bool EngineInput::RegisterActionSet(unsigned int unActionSet, const char* pszActionSet) {
    InputActionSetHandle_t handle = SteamInput()->GetActionSetHandle(pszActionSet);
    if (handle != 0) {
        ActionSet* pActionSet  = m_pDigitalActions->GetOrCreateActionSet(unActionSet, true);
        // May have created actionset for keyboard, mouse or touch action, so this wouldn't have been known:
        pActionSet->SetHandle(handle);
        return true;
    }
    return false;
}

void EngineInput::SetSteamControllerActionSet(unsigned int unActionSet) {
    m_unCurrentActionSet = unActionSet;
    m_pDigitalActions->SetCurrentActionSet(unActionSet);
    if (m_hActiveController == 0) {
        return;
    }
    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet==nullptr || pActionSet->GetHandle() == 0) {
#ifdef _DEBUG
        // Help to trace controller errors. To display in steamdeck, change project properties->linker->system from Windows to Console.
        // Then on steamdeck, press steammenu when running this 'game', select the console window, and press 'B' 
        if (gnActionSetError < 3 || (gnActionSetError % 1000) == 0) {
            std::cerr << "Cannot set action set handle, this has occured " << gnActionSetError << " times" << std::endl;
        }
        ++gnActionSetError;
#endif
        return;
    }
    SteamInput()->ActivateActionSet(m_hActiveController, pActionSet->GetHandle());
}

bool EngineInput::IsDigitalActionActive(unsigned int unAction, ITouchTarget** ppTarget, int* pnInvokeValue) {
    *ppTarget = nullptr;
    // Have to consider all, as if one is pressed after the other, releasing keyboard (if that was the only one considered) would
    //  prevent state processing correctly for the controller.
    bool bKeyboardActive = m_pDigitalActions->IsKeyboardActionActive(unAction);
    bool bMouseActive = m_pDigitalActions->IsMouseActionActive(unAction);
    bool bTouchActive = m_pDigitalActions->IsTouchActionActive(unAction, ppTarget, pnInvokeValue);
    bool bControllerActive = false;
    if (m_hActiveController != 0) {
        bControllerActive = m_pDigitalActions->IsControllerActionActive(unAction);
    }
    return bKeyboardActive || bControllerActive || bMouseActive || bTouchActive;
}

bool EngineInput::GetControllerAnalogAction(unsigned int unAction, float* pfX, float* pfY) {
    *pfX = 0.0f;
    *pfY = 0.0f;
    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(m_unCurrentActionSet, false);
    if (pActionSet == nullptr) {
        return false;
    }
    InputAnalogActionHandle_t handle;
    if (!pActionSet->GetAnalogHandleFromAction(unAction, &handle)) {
        return false;
    }
    ControllerAnalogActionData_t analogData = SteamInput()->GetAnalogActionData(m_hActiveController, handle);

    // Actions are only 'active' when they're assigned to a control in an action set, and that action set is active.
    if (analogData.bActive)
    {
        *pfX = analogData.x;
        *pfY = analogData.y;
    }
    else
    {
        *pfX = 0.0f;
        *pfY = 0.0f;
    }
    return true;
}

bool EngineInput::KeyPressed(unsigned int unKeyCode) {
    return m_pWnd->GetKey(unKeyCode) == GLFW_PRESS;
}

bool EngineInput::LoadGlypthsForActionSet(unsigned int unActionSet, int nWidth) {
    ESteamInputGlyphSize eSize;
    switch (nWidth) {
    case 32: eSize = k_ESteamInputGlyphSize_Small; break;
    case 128: eSize = k_ESteamInputGlyphSize_Medium; break;
    case 256: eSize = k_ESteamInputGlyphSize_Large; break;
    default:
        std::cerr << "Unsupported size (" << nWidth << " for steam glypth. Options are 32,64 and 256 pixels wide" << std::endl;
        return false;
    }

    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return false;
    }
    pActionSet->PopulateGlyphFilenamesForController(m_hActiveController, eSize);
    return true;
}

const char* EngineInput::GetGlyphFilenameForDigitalAction(unsigned int unAction, unsigned int unActionSet, int nWidth) {
    ESteamInputGlyphSize eSize;
    switch (nWidth) {
    case 32: eSize = k_ESteamInputGlyphSize_Small; break;
    case 128: eSize = k_ESteamInputGlyphSize_Medium; break;
    case 256: eSize = k_ESteamInputGlyphSize_Large; break;
    default:
        std::cerr << "Unsupported size (" << nWidth << " for steam glypth. Options are 32,64 and 256 pixels wide" << std::endl;
        return nullptr;
    }

    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return nullptr;
    }
    return pActionSet->GetGlyphFilenameForDigitalAction(unAction, eSize);
}

const char* EngineInput::GetGlyphFilenameForAnalogAction(unsigned int unAction, unsigned int unActionSet, int nWidth) {
    ESteamInputGlyphSize eSize;
    switch (nWidth) {
    case 32: eSize = k_ESteamInputGlyphSize_Small; break;
    case 128: eSize = k_ESteamInputGlyphSize_Medium; break;
    case 256: eSize = k_ESteamInputGlyphSize_Large; break;
    default:
        std::cerr << "Unsupported size (" << nWidth << " for steam glypth. Options are 32,64 and 256 pixels wide" << std::endl;
        return nullptr;
    }

    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return nullptr;
    }
    return pActionSet->GetGlyphFilenameForAnalogAction(unAction, eSize);
}

void EngineInput::GetActionCountForActionSet(unsigned int unActionSet, unsigned int* pnDigitalActionCount, unsigned int* pnAnalogActionCount) {
    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet != nullptr) {
        pActionSet->GetActionCount(pnDigitalActionCount, pnAnalogActionCount);
    }
}

unsigned int EngineInput::GetDigitalActionAtIndexForActionSet(unsigned int unActionSet, int nIndex) {
    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet != nullptr) {
        return pActionSet->GetDigitalActionAtIndex(nIndex);
    }
    return 0;
}

unsigned int EngineInput::GetAnalogActionAtIndexForActionSet(unsigned int unActionSet, int nIndex) {
    ActionSet* pActionSet = m_pDigitalActions->GetOrCreateActionSet(unActionSet, false);
    if (pActionSet != nullptr) {
        return pActionSet->GetAnalogActionAtIndex(nIndex);
    }
    return 0;
}

void EngineInput::CancelEvent(unsigned int unAction) {
    m_pDigitalActions->CancelEvent(unAction);
}

void EngineInput::CancelCurrentTouchEvent(bool bInformTarget) {
    m_pDigitalActions->CancelCurrentTouchEvent(bInformTarget);
}

void EngineInput::CancelAllEvents() {
    m_pDigitalActions->CancelAllEvents();
}

void EngineInput::ProcessMouseCallback(double fXPosIn, double fYPosIn) {
    m_pGameInput->ProcessMouseMovement((float)fXPosIn, (float)fYPosIn);
    m_pDigitalActions->ProcessMouseCallback(fXPosIn, fYPosIn);
}

void EngineInput::ProcessMouseButtonCallback(int nButton, int nAction, int nMods) {
    m_pGameInput->ProcessMouseButton(nButton, nAction, nMods);
    m_pDigitalActions->ProcessMouseButtonCallback(nButton, nAction, nMods);
}

void EngineInput::ProcessScrollCallback(double fYOffset) {
    m_pGameInput->ProcessMouseScroll((float)fYOffset);
}

void EngineInput::ProcessKeyCallback(int nKey, int nScancode, int nAction, int nMods) {
    InputAction eAction = m_pWnd->TranslateActionToEnum(nAction);
    bool bShift = false;
    bool bCtrl = false;
    bool bAlt = false;
    m_pWnd->TranslateModifiersToFlags(nMods, &bShift, &bCtrl, &bAlt);
    if (m_eActiveCharSet != eCS_None) {
        TextInputKey eKey = m_pWnd->TranslateKeyToEnum(nKey);
        if (eKey != eTIK_Unknown) {
            InputCharSet* pCS = GetCharSet(m_eActiveCharSet);

            EProcessChar pc = pCS->GetKeyIsProcessed(eKey);
            switch (pc) {
            case ePS_Ignore:
                return;
            case ePS_Process:
                if ((eAction == eIA_Press || eAction == eIA_AutoRepeat) && m_pTextInputTarget != nullptr) {
                    m_pTextInputTarget->KeyPressed(eKey, bShift, bCtrl, bAlt);
                }
                return;
            case ePS_NotProcessed:
                break;
            }
        }
        else if (eAction == eIA_Press) {
            // Unknown key presses may be characters to be input. Key releases are allowed otherwise there could be unfinished actions
            return;
        }
    }
    m_pGameInput->ProcessKeyEvent(nKey, nScancode, nAction, nMods);
    m_pDigitalActions->ProcessKeyCallback(nKey, nScancode, nAction, nMods);
}

void EngineInput::ProcessCharCallback(unsigned int unCodepoint) {
    if (m_eActiveCharSet == eCS_None) {
        return;
    }
    InputCharSet* pCS = GetCharSet(m_eActiveCharSet);

    EProcessChar pc = pCS->GetIsCharProcessed(unCodepoint);
    if (pc == ePS_Process && m_pTextInputTarget!=nullptr) {
        m_pTextInputTarget->CharInput((char)unCodepoint);
    }
}

void EngineInput::ProcessInternalActions() {
    for (unsigned int unAction : m_vcInternalActions) {
        ITouchTarget* pTarget;
        int nInvokeValue;
        bool bTouchActive = m_pDigitalActions->IsTouchActionActive(unAction, &pTarget, &nInvokeValue);
        if (bTouchActive) {
            if (pTarget != nullptr) {
                pTarget->Invoke(unAction, nInvokeValue, 0.01f);
            }
        }
    }
}

int EngineInput::GetInternalActionStartIndex() {
    return INTERNAL_ACTION_START;
}

void mouse_callback(GLFWwindow* pGLWnd, double xposIn, double yposIn) {
    Window* pWindow = Window::FindAssociatedWindow(pGLWnd);
    if (pWindow == nullptr) {
        return;
    }
    EngineInput* pEngineInput = pWindow->GetEngineInput();
    pEngineInput->ProcessMouseCallback(xposIn, yposIn);
}

void mouse_button_callback(GLFWwindow* pGLWnd, int button, int action, int mods) {
    Window* pWindow = Window::FindAssociatedWindow(pGLWnd);
    if (pWindow == nullptr) {
        return;
    }
    EngineInput* pEngineInput = pWindow->GetEngineInput();
    pEngineInput->ProcessMouseButtonCallback(button, action, mods);
}

void scroll_callback(GLFWwindow* pGLWnd, double xoffset, double yoffset)
{
    Window* pWindow = Window::FindAssociatedWindow(pGLWnd);
    if (pWindow == nullptr) {
        return;
    }
    EngineInput* pEngineInput = pWindow->GetEngineInput();
    pEngineInput->ProcessScrollCallback(yoffset);
}

void key_callback(GLFWwindow* pGLWnd, int nKey, int nScancode, int nAction, int nMods) {
    Window* pWindow = Window::FindAssociatedWindow(pGLWnd);
    if (pWindow == nullptr) {
        return;
    }
    EngineInput* pEngineInput = pWindow->GetEngineInput();
    pEngineInput->ProcessKeyCallback(nKey, nScancode, nAction, nMods);
}

void char_callback(GLFWwindow* pGLWnd, unsigned int unCodepoint) {
    Window* pWindow = Window::FindAssociatedWindow(pGLWnd);
    if (pWindow == nullptr) {
        return;
    }
    EngineInput* pEngineInput = pWindow->GetEngineInput();
    pEngineInput->ProcessCharCallback(unCodepoint);
}

void focus_callback(GLFWwindow* pGLWnd, int focused) {
}
