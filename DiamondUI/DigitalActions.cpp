#include "DigitalActions.h"
#include "Window.h"
#include "ITouchLayer.h"

#include <iostream>

#define KEYBOARD_ACTION_OFFSET (1*65536)
#define MOUSE_ACTION_OFFSET (2*65536)
#define TOUCH_ACTION_OFFSET (3*65536)

#define AUTOREPEAT_INITIALDELAY 0.250
#define AUTOREPEAT_DELAY 0.150
#define AUTOREPEAT_DELAY_FASTER 0.030

DigitalActions::DigitalActions(IWindow* pWnd, unsigned int unInternalActionStartIndex) {
    m_pWnd = pWnd;
    m_unInternalActionStartIndex = unInternalActionStartIndex;

    m_unCurrentActionSet = 0;
    m_unCurrentActionSetForPressAction = 0;
    m_nInvokeValueForCurrentPressAction = -1;
    m_unCurrentPressAction = 0;
    m_xCurrentPressActionAreaOrigin = 0;
    m_yCurrentPressActionAreaOrigin = 0;

    m_hActiveController = 0;
}

DigitalActions::~DigitalActions() {
    m_lsTouchLayers.clear();

    for (auto as : m_mpActionSetIdToActionSets) {
        delete as.second;
    }
    m_mpActionSetIdToActionSets.clear();
}

void DigitalActions::SetCurrentActionSet(unsigned int unActionSet) {
    m_unCurrentActionSet = unActionSet;
}

void DigitalActions::SetActiveController(InputHandle_t hActiveController) {
    m_hActiveController = hActiveController;
}

void DigitalActions::AddTouchLayer(ITouchLayer* pTouchLayer) {
    m_lsTouchLayers.push_front(pTouchLayer);
}

void DigitalActions::RemoveTouchLayer(ITouchLayer* pTouchLayer) {
    auto find = std::find(m_lsTouchLayers.begin(), m_lsTouchLayers.end(), pTouchLayer);
    if (find != m_lsTouchLayers.end()) {
        m_lsTouchLayers.erase(find);
    }
}

void DigitalActions::CancelEvent(unsigned int unAction) {
    SetActionIsCurrentlyPressed(unAction, false);
    SetActionNextFireTime(unAction, -1.0);
    SetActionSpeedUpTime(unAction, -1.0);
}

void DigitalActions::CancelCurrentTouchEvent(bool bInformTarget) {
    TouchTrigger* pTrigger = RetrieveTouchTriggerForActionInActionSet(m_unCurrentPressAction, m_unCurrentActionSetForPressAction);
    if (pTrigger != nullptr) {
        pTrigger->bPressed = false;
        if (bInformTarget) {
            pTrigger->pTarget->TouchUpdate(m_unCurrentPressAction - TOUCH_ACTION_OFFSET, m_nInvokeValueForCurrentPressAction, false, false, 0, 0);
        }
    }
    m_unCurrentPressAction = 0;
    m_unCurrentActionSetForPressAction = 0;
    m_nInvokeValueForCurrentPressAction = -1;
}

void DigitalActions::CancelAllEvents() {
    CancelCurrentTouchEvent(false);

    for (auto e : m_mpActionSetIdToActionSets) {
        e.second->CancelAllEvents();
    }
}

ActionSet* DigitalActions::GetOrCreateActionSet(unsigned int unActionSetId, bool bCreateIfNonExistent) {
    ActionSet* pActionSet = nullptr;
    auto find = m_mpActionSetIdToActionSets.find(unActionSetId);

    if (find != m_mpActionSetIdToActionSets.end()) {
        pActionSet = find->second;
    }
    else if (bCreateIfNonExistent) {
        pActionSet = new ActionSet();
        m_mpActionSetIdToActionSets[unActionSetId] = pActionSet;
    }
    return pActionSet;
}

bool DigitalActions::RegisterControllerDigitalAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction, DigitalActionType eActionType) {
    if (unAction >= m_unInternalActionStartIndex) {
        return false;
    }
    InputDigitalActionHandle_t handle = SteamInput()->GetDigitalActionHandle(pszAction);
    if (handle != 0) {
        ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
        if (pActionSet == nullptr) {
            std::cerr << "Register action set before registering (digital) actions" << std::endl;
            return false;
        }
        pActionSet->RegisterControllerDigitalAction(unAction, handle);
        m_mpActionType[unAction] = eActionType;
        m_mpActionIsPressed[unAction] = false;
        return true;
    }
    return false;
}

bool DigitalActions::RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, int nGLFWKeyToken, DigitalActionType eActionType) {
    if (unAction >= m_unInternalActionStartIndex) {
        return false;
    }

    // Increase action number to prevent a keyboard press being interfered with by a controller not-pressed, where they share the same action.
    // All unActions for keyboard events are increased by 65536.
    unAction += KEYBOARD_ACTION_OFFSET;
    int scanCode = glfwGetKeyScancode(nGLFWKeyToken);
    if (scanCode == GLFW_NOT_INITIALIZED || scanCode == GLFW_INVALID_ENUM) {
        return false;
    }
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, true);
    pActionSet->RegisterKeyboardScanCode(scanCode, unAction);

    m_mpActionType[unAction] = eActionType;
    return true;
}

bool DigitalActions::RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, char chKey, DigitalActionType eActionType) {
    if (unAction >= m_unInternalActionStartIndex) {
        return false;
    }
    unAction += KEYBOARD_ACTION_OFFSET;
    int scanCode;
    if (chKey == ' ' || chKey == '\'' ||
        (chKey >= ',' && chKey <= '9') ||
        chKey == ';' || chKey == '=' ||
        (chKey >= 'A' && chKey <= ']') ||
        chKey == '`') {
        scanCode = glfwGetKeyScancode(chKey);
    }
    else {
        switch (chKey) {
        case 8: scanCode = glfwGetKeyScancode(GLFW_KEY_BACKSPACE); break;
        case 9: scanCode = glfwGetKeyScancode(GLFW_KEY_TAB); break;
        case 13: scanCode = glfwGetKeyScancode(GLFW_KEY_ENTER); break;
        case 27: scanCode = glfwGetKeyScancode(GLFW_KEY_ESCAPE); break;
        default: return false;
        }
    }

    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, true);
    pActionSet->RegisterKeyboardScanCode(scanCode, unAction);
    m_mpActionType[unAction] = eActionType;
    return true;
}

bool DigitalActions::RegisterMouseAction(unsigned int unAction, unsigned int unActionSet, int nGLFWMouseToken, DigitalActionType eActionType) {
    if (unAction >= m_unInternalActionStartIndex) {
        return false;
    }

    // Increase action number to prevent a mouse press being interfered with by a controller/keyboard not-pressed, where they share the same action.
    // All unActions for mouse events are increased by 131072.
    unAction += MOUSE_ACTION_OFFSET;

    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, true);
    pActionSet->RegisterMouseScanCode(nGLFWMouseToken, unAction);
    m_mpActionType[unAction] = eActionType;
    return true;
}

bool DigitalActions::RegisterTouchAction(unsigned int unAction, DigitalActionType eActionType) {
    if (unAction >= m_unInternalActionStartIndex) {
        return false;
    }

    m_mpActionType[unAction] = eActionType;
    return true;
}

bool DigitalActions::RegisterInternalTouchAction(unsigned int unAction, DigitalActionType eActionType) {
    if (unAction < m_unInternalActionStartIndex) {
        return false;
    }
    m_mpActionType[unAction] = eActionType;
    return true;
}

bool DigitalActions::IsKeyboardActionActive(unsigned int unAction) {
    unAction += KEYBOARD_ACTION_OFFSET;
    bool bTriggered = IsActionTriggerByKeyboard(m_unCurrentActionSet, unAction);

    DigitalActionType dat = GetActionType(unAction);
    switch (dat) {
    case eDAT_EventOnRelease:
        return ProcessDigitalAction_EventOnRelease(unAction, bTriggered);
    case eDAT_EventContinuous:
        return bTriggered;
    case eDAT_EventOnPress:
        return ProcessDigitalAction_EventOnPress(unAction, bTriggered);
    case eDAT_EventAutoRepeat:
        return ProcessDigitalAction_AutoRepeat(unAction, bTriggered);
    case eDAT_EventAutoRepeatTwoLevels:
        return ProcessDigitalAction_AutoRepeat_TwoLevels(unAction, bTriggered);
    }
    return false;
}

bool DigitalActions::IsMouseActionActive(unsigned int unAction) {
    unAction += MOUSE_ACTION_OFFSET;
    bool bTriggered = IsActionTriggerByMouse(m_unCurrentActionSet, unAction);

    DigitalActionType dat = GetActionType(unAction);
    switch (dat) {
    case eDAT_EventOnRelease:
        return ProcessDigitalAction_EventOnRelease(unAction, bTriggered);
    case eDAT_EventContinuous:
        return bTriggered;
    case eDAT_EventOnPress:
        return ProcessDigitalAction_EventOnPress(unAction, bTriggered);
    case eDAT_EventAutoRepeat:
        return ProcessDigitalAction_AutoRepeat(unAction, bTriggered);
    case eDAT_EventAutoRepeatTwoLevels:
        return ProcessDigitalAction_AutoRepeat_TwoLevels(unAction, bTriggered);
    }
    return false;
}

bool DigitalActions::IsTouchActionActive(unsigned int unAction, ITouchTarget** ppTarget, int* pnInvokeValue) {
    unsigned int unAction_ForDAT = unAction;
    unAction += TOUCH_ACTION_OFFSET;
    TouchTrigger* pTT = IsActionTriggeredByPress(m_unCurrentActionSet, unAction);
    if (pTT == nullptr) {
        // No press or release recorded in a touch trigger, against a touch target.
        return false;
    }
    bool bPressed = pTT->bPressed;
    bool bCurrentlyOverTarget = pTT->bCurrentlyOverTarget;
    *ppTarget = pTT->pTarget;
    *pnInvokeValue = pTT->nInvokeValue;
    if (!bPressed) {
        // If this is not being called, so a touch action only triggers once, it is likely that this methods isn't being 
        //  called with unAction. It may need adding to an array in your GameInput code.
        RemovePressActionTrigger(m_unCurrentActionSet, unAction);
    }

    DigitalActionType dat = GetActionType(unAction_ForDAT);
    bool toReturn = true;
    switch (dat) {
    case eDAT_EventOnRelease:
        toReturn = bCurrentlyOverTarget && ProcessDigitalAction_EventOnRelease(unAction, bPressed);
        break;
    case eDAT_EventContinuous:
        toReturn = bPressed && bCurrentlyOverTarget;
        break;
    case eDAT_EventOnPress:
        toReturn = ProcessDigitalAction_EventOnPress(unAction, bPressed);
        break;
    case eDAT_EventAutoRepeat:
        toReturn = bCurrentlyOverTarget && ProcessDigitalAction_AutoRepeat(unAction, bPressed);
        break;
    case eDAT_EventAutoRepeatTwoLevels:
        toReturn = bCurrentlyOverTarget && ProcessDigitalAction_AutoRepeat_TwoLevels(unAction, bPressed);
    }
    if (!toReturn) {
        *ppTarget = nullptr;
        *pnInvokeValue = 0;
    }
    return toReturn;
}

bool DigitalActions::IsControllerActionActive(unsigned int unAction) {
    ActionSet* pActionSet = GetOrCreateActionSet(m_unCurrentActionSet, false);
    if (pActionSet == nullptr) {
        return false;
    }
    InputDigitalActionHandle_t handle;
    if (!pActionSet->GetDigitalHandleFromAction(unAction, &handle)) {
        return false;
    }
    ControllerDigitalActionData_t digitalData = SteamInput()->GetDigitalActionData(m_hActiveController, handle);
    if (digitalData.bActive) {
        DigitalActionType dat = GetActionType(unAction);
        switch (dat) {
        case eDAT_EventOnRelease:
            return ProcessDigitalAction_EventOnRelease(unAction, digitalData.bState);
        case eDAT_EventContinuous:
            return digitalData.bState;
        case eDAT_EventOnPress:
            return ProcessDigitalAction_EventOnPress(unAction, digitalData.bState);
        case eDAT_EventAutoRepeat:
            return ProcessDigitalAction_AutoRepeat(unAction, digitalData.bState);
        case eDAT_EventAutoRepeatTwoLevels:
            return ProcessDigitalAction_AutoRepeat_TwoLevels(unAction, digitalData.bState);
        }
    }
    return false;
}

bool DigitalActions::IsActionTriggerByKeyboard(unsigned int unActionSet, unsigned int unAction) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return false;
    }
    return pActionSet->GetKeyboardActionTriggeredState(unAction);
}

bool DigitalActions::IsActionTriggerByMouse(unsigned int unActionSet, unsigned int unAction) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return false;
    }
    return pActionSet->GetMouseActionTriggeredState(unAction);
}

TouchTrigger* DigitalActions::IsActionTriggeredByPress(unsigned int unActionSet, unsigned int unAction) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return nullptr;
    }
    return pActionSet->GetTouchTriggerForAction(unAction);
}

void DigitalActions::RemovePressActionTrigger(unsigned int unActionSet, unsigned int unAction) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return;
    }
    pActionSet->RemoveTouchTriggerForAction(unAction);
}

bool DigitalActions::ProcessDigitalAction_EventOnRelease(unsigned int unAction, bool bTriggered) {
    if (ActionIsCurrentlyPressed(unAction)) {
        // Button is recorded as being already pressed
        if (bTriggered) {
            // It is still present, and event is to occur on release
            return false;
        }
        else {
            // Not still pressed, send action
            SetActionIsCurrentlyPressed(unAction, false);
            return true;
        }
    }
    else {
        // Button not previous pressed, if it has been pressed record that fact
        if (bTriggered) {
            SetActionIsCurrentlyPressed(unAction, true);
        }
        return false;
    }
}

bool DigitalActions::ProcessDigitalAction_EventOnPress(unsigned int unAction, bool bTriggered) {
    if (ActionIsCurrentlyPressed(unAction)) {
        // Action was already sent when it was pressed.
        if (bTriggered == false) {
            // Actual event data indicates button is no longer pressed. Reset it.
            SetActionIsCurrentlyPressed(unAction, false);
        }
        return false;
    }
    if (bTriggered) {
        // Button is pressed, and event has not been previously sent
        SetActionIsCurrentlyPressed(unAction, true);
        return true;
    }
    return false;
}

bool DigitalActions::ProcessDigitalAction_AutoRepeat(unsigned int unAction, bool bTriggered) {
    if (ActionIsCurrentlyPressed(unAction)) {
        // Previously pressed, see if enough time has elapsed to send another event
        if (bTriggered) {
            float currentTime = static_cast<float>(m_pWnd->GetCurrentTime());
            float nextFireTime = static_cast<float>(GetActionNextFireTime(unAction));
            if (nextFireTime == -1) {
                // Event was cancelled.
                return false;
            }

            if (currentTime > nextFireTime) {
                SetActionNextFireTime(unAction, currentTime + AUTOREPEAT_DELAY);
                return true;
            }
            return false;
        }
        else {
            SetActionIsCurrentlyPressed(unAction, false);
            SetActionNextFireTime(unAction, -1.0);
            SetActionSpeedUpTime(unAction, -1.0);
        }
    }
    else {
        // Was not previously pressed. If it is not pressed, send event and record the next time an event can be sent.
        if (bTriggered) {
            SetActionIsCurrentlyPressed(unAction, true);
            double currentTime = m_pWnd->GetCurrentTime();
            SetActionNextFireTime(unAction, currentTime + AUTOREPEAT_INITIALDELAY);
            SetActionSpeedUpTime(unAction, currentTime + AUTOREPEAT_INITIALDELAY + 5 * AUTOREPEAT_DELAY);
            return true;
        }
    }
    return false;
}

bool DigitalActions::ProcessDigitalAction_AutoRepeat_TwoLevels(unsigned int unAction, bool bTriggered) {
    if (ActionIsCurrentlyPressed(unAction)) {
        // Previously pressed, see if enough time has elapsed to send another event
        if (bTriggered) {
            float currentTime = static_cast<float>(m_pWnd->GetCurrentTime());
            float nextFireTime = static_cast<float>(GetActionNextFireTime(unAction));
            if (nextFireTime == -1) {
                // Event was cancelled.
                return false;
            }
            float speedUpTime = static_cast<float>(GetActionSpeedUpTime(unAction));
            double autoRepeatDelay = AUTOREPEAT_DELAY;
            if (currentTime > speedUpTime) {
                autoRepeatDelay = AUTOREPEAT_DELAY_FASTER;
            }
            
            if (currentTime > nextFireTime) {
                SetActionNextFireTime(unAction, currentTime + autoRepeatDelay);
                return true;
            }
            return false;
        }
        else {
            SetActionIsCurrentlyPressed(unAction, false);
            SetActionNextFireTime(unAction, -1.0);
            SetActionSpeedUpTime(unAction, -1.0);
        }
    }
    else {
        // Was not previously pressed. If it is not pressed, send event and record the next time an event can be sent.
        if (bTriggered) {
            SetActionIsCurrentlyPressed(unAction, true);
            double currentTime = m_pWnd->GetCurrentTime();
            SetActionNextFireTime(unAction, currentTime + AUTOREPEAT_INITIALDELAY);
            SetActionSpeedUpTime(unAction, currentTime + AUTOREPEAT_INITIALDELAY + 5 * AUTOREPEAT_DELAY);
            return true;
        }
    }
    return false;
}


DigitalActionType DigitalActions::GetActionType(unsigned int unAction) {
    auto find = m_mpActionType.find(unAction);
    if (find == m_mpActionType.end()) {
        return eDAT_None;
    }
    return find->second;
}

bool DigitalActions::ActionIsCurrentlyPressed(unsigned int unAction) {
    auto find = m_mpActionIsPressed.find(unAction);
    if (find == m_mpActionIsPressed.end()) {
        return false;
    }
    return find->second;
}

void DigitalActions::SetActionIsCurrentlyPressed(unsigned int unAction, bool bPressed) {
    m_mpActionIsPressed[unAction] = bPressed;
}

void DigitalActions::SetActionNextFireTime(unsigned int unAction, double dNextFireTime) {
    if (dNextFireTime == -1) {
        // -1 means event is being cancelled or it has finished
        auto find = m_mpActionNextFireTime.find(unAction);
        if (find != m_mpActionNextFireTime.end()) {
            m_mpActionNextFireTime.erase(find);
        }
        return;
    }
    m_mpActionNextFireTime[unAction] = dNextFireTime;
}

void DigitalActions::SetActionSpeedUpTime(unsigned int unAction, double dSpeedupTime) {
    if (dSpeedupTime == -1) {
        // -1 means event is being cancelled or it has finished
        auto find = m_mpActionSpeedUpTime.find(unAction);
        if (find != m_mpActionSpeedUpTime.end()) {
            m_mpActionSpeedUpTime.erase(find);
        }
        return;
    }
    m_mpActionSpeedUpTime[unAction] = dSpeedupTime;
}

double DigitalActions::GetActionNextFireTime(unsigned int unAction) {
    auto find = m_mpActionNextFireTime.find(unAction);
    if (find == m_mpActionNextFireTime.end()) {
        return -1.0;
    }
    return find->second;
}

double DigitalActions::GetActionSpeedUpTime(unsigned int unAction) {
    auto find = m_mpActionSpeedUpTime.find(unAction);
    if (find == m_mpActionSpeedUpTime.end()) {
        return -1.0;
    }
    return find->second;

}

TouchTrigger* DigitalActions::RetrieveTouchTriggerForActionInActionSet(unsigned int unAction, unsigned int unActionSet) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        return nullptr;
    }
    return pActionSet->GetTouchTriggerForAction(unAction);
}

void DigitalActions::CreateCurrentTouchTriggerForActionInActionSet(const Touch& t, unsigned int unActionSet) {
    ActionSet* pActionSet = GetOrCreateActionSet(unActionSet, false);
    if (pActionSet == nullptr) {
        std::cerr << "Failed to create touch trigger for action set, as action set " << unActionSet << " does not exist" << std::endl;
        return;
    }
    pActionSet->CreateNewTouchTriggerForAction(t.m_unAction, t.m_pTarget, t.m_nInvocationValue);
    m_unCurrentPressAction = t.m_unAction;
    m_unCurrentActionSetForPressAction = unActionSet;
    m_nInvokeValueForCurrentPressAction = t.m_nInvocationValue;
    m_xCurrentPressActionAreaOrigin = t.m_rcTouch.m_nXOrigin;
    m_yCurrentPressActionAreaOrigin = t.m_rcTouch.m_nYOrigin;
}

Touch DigitalActions::GetTargetForTouch(int xPress, int yPress) {
    for (auto l : m_lsTouchLayers) {
        if (l->GetLayerActive()) {
            Touch t = l->GetTouchTarget(xPress, yPress);
            if (t.m_bTouchValid) {
                t.m_unAction+= TOUCH_ACTION_OFFSET;
                return t;
            }
        }
    }
    return Touch(false);
}

void DigitalActions::ProcessMouseCallback(double fXPosIn, double fYPosIn) {
    if (m_unCurrentPressAction != 0) {
        bool markCurrentActionAsPaused = false;

        // Window converts to integer and flips the Y-axis.
        int xMouse;
        int yMouse;
        m_pWnd->GetMousePosition(&xMouse, &yMouse);

        // Should be GetPotentialTargetForMouseOver, but, it would just call GetTargetForTouch
        Touch t = GetTargetForTouch(xMouse, yMouse);
        if (t.m_bTouchValid) {
            TouchTrigger* pTrigger = RetrieveTouchTriggerForActionInActionSet(t.m_unAction, m_unCurrentActionSet);
            if (pTrigger != nullptr) {
                ITouchTarget* pLastInteraction = pTrigger->pTarget;
                pTrigger->bCurrentlyOverTarget = (pLastInteraction == t.m_pTarget && t.m_unAction == m_unCurrentPressAction && t.m_nInvocationValue == m_nInvokeValueForCurrentPressAction);
                if (t.m_bContinuousUpdate) {
                    pLastInteraction->TouchUpdate(m_unCurrentPressAction - TOUCH_ACTION_OFFSET, m_nInvokeValueForCurrentPressAction, true, pTrigger->bCurrentlyOverTarget, xMouse - m_xCurrentPressActionAreaOrigin, yMouse - m_yCurrentPressActionAreaOrigin);
                }
                return;
            }
        }
        TouchTrigger* pTrigger = RetrieveTouchTriggerForActionInActionSet(m_unCurrentPressAction, m_unCurrentActionSetForPressAction);
        if (pTrigger != nullptr) {
            pTrigger->bCurrentlyOverTarget = false;
            pTrigger->pTarget->TouchUpdate(m_unCurrentPressAction - TOUCH_ACTION_OFFSET, m_nInvokeValueForCurrentPressAction, true, pTrigger->bCurrentlyOverTarget, xMouse - m_xCurrentPressActionAreaOrigin, yMouse - m_yCurrentPressActionAreaOrigin);
        }
        return;
    }
}

void DigitalActions::ProcessMouseButtonCallback(int nButton, int nAction, int nMods) {
    bool cancelAnyCurrentPressAction = false;
    if (nButton == GLFW_MOUSE_BUTTON_1) {
        int xMouse;
        int yMouse;
        m_pWnd->GetMousePosition(&xMouse, &yMouse);
        Touch t = GetTargetForTouch(xMouse, yMouse);
        if (t.m_bTouchValid) {
            TouchTrigger* pTrigger = RetrieveTouchTriggerForActionInActionSet(t.m_unAction, m_unCurrentActionSet);
            if (pTrigger != nullptr) {
                // This touch are was pressed already
                //  No need to compare whether the release-touch was over original target. Any release will fire on-release events
                if (nAction == GLFW_RELEASE) {
                    pTrigger->bPressed = false;
                    // pTrigger member, bCurrentlyOverTarget, is tracked by mouse movement callback
                    pTrigger->pTarget->TouchUpdate(m_unCurrentPressAction - TOUCH_ACTION_OFFSET, m_nInvokeValueForCurrentPressAction, false, pTrigger->bCurrentlyOverTarget, xMouse - m_xCurrentPressActionAreaOrigin, yMouse - m_yCurrentPressActionAreaOrigin);
                    m_unCurrentPressAction = 0;
                    m_unCurrentActionSetForPressAction = 0;
                    m_nInvokeValueForCurrentPressAction = -1;
                } // else: trigger is still pressed. Take no action
            }
            else {
                // This touch area was not previously pressed.
                if (nAction == GLFW_PRESS) {
                    // New touch action
                    CreateCurrentTouchTriggerForActionInActionSet(t, m_unCurrentActionSet);
                    if (t.m_bContinuousUpdate) {
                        t.m_pTarget->TouchUpdate(t.m_unAction - TOUCH_ACTION_OFFSET, t.m_nInvocationValue, true, true, xMouse - m_xCurrentPressActionAreaOrigin, yMouse - m_yCurrentPressActionAreaOrigin);
                    }
                    return;
                } // else: A release, but, press was recorded for another action. Attempt to release the current action 
                else {
                    cancelAnyCurrentPressAction = true;
                }
            }
        }
        else if (nAction == GLFW_RELEASE) {
            cancelAnyCurrentPressAction = true;
        }
        if (cancelAnyCurrentPressAction) {
            TouchTrigger* pCurrentTrigger = RetrieveTouchTriggerForActionInActionSet(m_unCurrentPressAction, m_unCurrentActionSetForPressAction);
            if (pCurrentTrigger != nullptr) {
                pCurrentTrigger->bPressed = false;
                pCurrentTrigger->pTarget->TouchUpdate(m_unCurrentPressAction - TOUCH_ACTION_OFFSET, m_nInvokeValueForCurrentPressAction, false, false, xMouse - m_xCurrentPressActionAreaOrigin, yMouse - m_yCurrentPressActionAreaOrigin);
                m_unCurrentPressAction = 0;
                m_unCurrentActionSetForPressAction = 0;
                m_nInvokeValueForCurrentPressAction = -1;
            }
        }
    }

    ActionSet* pActionSet = GetOrCreateActionSet(m_unCurrentActionSet, false);
    if (pActionSet == nullptr) {
        return;
    }

    unsigned int unAction;
    if (!pActionSet->GetActionFromMouseScanCode(nButton, &unAction)) {
        return;
    }
    if (nAction == GLFW_PRESS) {
        pActionSet->SetMouseActionTriggeredState(unAction, true);
    }
    else if (nAction == GLFW_RELEASE) {
        pActionSet->SetMouseActionTriggeredState(unAction, false);
    }
}

void DigitalActions::ProcessKeyCallback(int nKey, int nScancode, int nAction, int nMods) {
    ActionSet* pActionSet = GetOrCreateActionSet(m_unCurrentActionSet, false);
    unsigned int unAction;
    if (!pActionSet->GetActionFromKeyboardScanCode(nScancode, &unAction)) {
        return;
    }
    if (nAction == GLFW_PRESS) {
        pActionSet->SetKeyActionTriggeredState(unAction, true);
    }
    else if (nAction == GLFW_RELEASE) {
        pActionSet->SetKeyActionTriggeredState(unAction, false);
    }
}

#ifdef _DEBUG
void DigitalActions::OutputCurrentAction() {
    std::cout << "Current press action: " << m_unCurrentPressAction << std::endl;
    std::cout << "        invoke value: " << m_nInvokeValueForCurrentPressAction << std::endl;
}
#endif
