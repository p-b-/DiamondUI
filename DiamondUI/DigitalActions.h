#pragma once
#include "InputDefs.h"
#include "ActionSet.h"

#include <map>
#include <list>
class IWindow;
class ITouchLayer;

//namespace Condensate {
    class DigitalActions
    {
    public:
        DigitalActions(IWindow* pWnd, unsigned int unInternalActionStartIndex);
        ~DigitalActions();

        void SetCurrentActionSet(unsigned int unActionSet);
        void SetActiveController(InputHandle_t hActiveController);

        void AddTouchLayer(ITouchLayer* pTouchLayer);
        void RemoveTouchLayer(ITouchLayer* pTouchLayer);
        void CancelEvent(unsigned int unAction);
        void CancelCurrentTouchEvent(bool bInformTarget);
        void CancelAllEvents();
        ActionSet* GetOrCreateActionSet(unsigned int unActionSetId, bool bCreateIfNonExistent);

        bool RegisterControllerDigitalAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction, DigitalActionType eActionType);
        bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, int nGLFWKeyToken, DigitalActionType eActionType);
        bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, char chKey, DigitalActionType eActionType);
        bool RegisterMouseAction(unsigned int unAction, unsigned int unActionSet, int nGLFWMouseToken, DigitalActionType eActionType);
        bool RegisterTouchAction(unsigned int unAction, DigitalActionType eActionType);
        bool RegisterInternalTouchAction(unsigned int unAction, DigitalActionType eActionType);

        bool IsKeyboardActionActive(unsigned int unAction);
        bool IsMouseActionActive(unsigned int unAction);
        bool IsTouchActionActive(unsigned int unAction, ITouchTarget** ppTarget, int* pnInvokeValue);
        bool IsControllerActionActive(unsigned int unAction);

        void ProcessMouseCallback(double fXPosIn, double fYPosIn);
        void ProcessMouseButtonCallback(int nButton, int nAction, int nMods);
        void ProcessKeyCallback(int nKey, int nScancode, int nAction, int nMods);

#ifdef _DEBUG
        void OutputCurrentAction();
#endif

    protected:
        bool IsActionTriggerByKeyboard(unsigned int unActionSet, unsigned int unAction);
        bool IsActionTriggerByMouse(unsigned int unActionSet, unsigned int unAction);
        TouchTrigger* IsActionTriggeredByPress(unsigned int unActionSet, unsigned int unAction);
        void RemovePressActionTrigger(unsigned int unActionSet, unsigned int unAction);
    private:
        bool ProcessDigitalAction_EventOnRelease(unsigned int unAction, bool bTriggered);
        bool ProcessDigitalAction_EventOnPress(unsigned int unAction, bool bTriggered);
        bool ProcessDigitalAction_AutoRepeat(unsigned int unAction, bool bTriggered);
        bool ProcessDigitalAction_AutoRepeat_TwoLevels(unsigned int unAction, bool bTriggered);

        DigitalActionType GetActionType(unsigned int unAction);
        bool ActionIsCurrentlyPressed(unsigned int unAction);
        void SetActionIsCurrentlyPressed(unsigned int unAction, bool bPressed);
        void SetActionNextFireTime(unsigned int unAction, double dNextFireTime);
        void SetActionSpeedUpTime(unsigned int unAction, double dSpeedUpTime);
        double GetActionNextFireTime(unsigned int unAction);
        double GetActionSpeedUpTime(unsigned int unAction);

        TouchTrigger* RetrieveTouchTriggerForActionInActionSet(unsigned int unAction, unsigned int unActionSet);
        void CreateCurrentTouchTriggerForActionInActionSet(const Touch& t, unsigned int unActionSet);
        //    void CreateCurrentTouchTriggerForActionInActionSet(unsigned int unAction, ITouchTarget* pTarget, unsigned int unActionSet, int nInvokeValue);

            //bool GetTargetForTouch(int xPress, int yPress, unsigned int* punAction, ITouchTarget** ppTarget, int* pnInvokeValue, bool* pbContinuousUpdates);
        Touch GetTargetForTouch(int xPress, int yPress);

    private:
        IWindow* m_pWnd;
        unsigned int m_unInternalActionStartIndex;
        std::map<unsigned int, ActionSet*> m_mpActionSetIdToActionSets;

        // Set from EngineInput
        InputHandle_t m_hActiveController;
        unsigned int m_unCurrentActionSet;

        std::map<unsigned int, DigitalActionType > m_mpActionType;
        std::map<unsigned int, double> m_mpActionNextFireTime;
        std::map<unsigned int, double> m_mpActionSpeedUpTime;
        std::map<unsigned int, bool> m_mpActionIsPressed;

        std::list<ITouchLayer* > m_lsTouchLayers;

        unsigned int m_unCurrentPressAction;
        unsigned int m_unCurrentActionSetForPressAction;
        int m_nInvokeValueForCurrentPressAction;
        int m_xCurrentPressActionAreaOrigin;
        int m_yCurrentPressActionAreaOrigin;
    };
//}