#pragma once
class Window;
class IGameInput;
#include "IEngineInput.h"
#include "IEngineInputIntl.h"
#include "ActionSet.h"
#include <steam/steam_api.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <stack>
#include <atomic>

class ITouchTarget;
class DigitalActions;
class InputCharSet;
class ITextInputTarget;

class EngineInput : public IEngineInputIntl, public IEngineInput
{
	enum ConnectorConnectionAction {
		eCCA_NoController,
		eCCA_StillConnected,
		eCCA_ControllerConnected,
		eCCA_ControllerDisconnected,
		eCCA_ControllerChanged,
		eCCA_ControllerStillConnected,
		eCCA_ControllerReconnected
	};
	struct DigitalAction {
		bool pressed;
		float lastEventTime;
	};

public:
	EngineInput(Window* pWnd, IGameInput* pGameInput);
	virtual ~EngineInput();

	// IEngineInputIntl declarations
public:
	virtual bool Initialise();
	virtual void DeinitialiseAndDelete();
	virtual bool RegisterInternalTouchAction(unsigned int unAction, DigitalActionType eActionType);
	virtual void ProcessInput(float fDeltaTime);
	virtual void ShowBindingPanel();
	virtual void AddTouchLayer(ITouchLayer* pTouchLayer);
	virtual void RemoveTouchLayer(ITouchLayer* pTouchLayer);
	virtual void NewWindowLayer();
	virtual void AlterTopWindowTouchLayerCount(bool bIncNotDec);
	virtual void RemoveTopWindowLayer(bool bLayersAlreadyRemoved);
	virtual void ActivateTopWindowLayer(bool bActivateNotDeactivate);
	virtual bool ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTextInputTarget, const IntRect& rcInputArea);
	virtual void KeyboardDismissed();

public:
	void ProcessMouseCallback(double fXPosIn, double fYPosIn);
	void ProcessMouseButtonCallback(int nButton, int nAction, int nMods);
	void ProcessScrollCallback(double fYOffset);
	void ProcessKeyCallback(int nKey, int nScancode, int nAction, int nMods);
	void ProcessCharCallback(unsigned int unCodepoint);

	// IEngineInput interface overrides
public:
	virtual bool IsDigitalActionActive(unsigned int unAction, ITouchTarget** ppTarget, int* pnInvokeValue);
	virtual bool GetControllerAnalogAction(unsigned int unAction, float* pfX, float* pfY);
	virtual bool RegisterControllerDigitalAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction, DigitalActionType eActionType);
	virtual bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, int nGLFWKeyToken, DigitalActionType eActionType);
	virtual bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, const char key, DigitalActionType eActionType);
	virtual bool RegisterMouseAction(unsigned int unAction, unsigned int unActionSet, int nGLFWMouseToken, DigitalActionType eActionType);
	// Used for touch actions that are not present as controller, keyboard or mouse actions
	virtual bool RegisterTouchAction(unsigned int unAction, DigitalActionType eActionType);
	virtual bool RegisterControllerAnalogAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction);
	virtual bool RegisterActionSet(unsigned int unActionSet, const char* pszActionSet);
	virtual void SetSteamControllerActionSet(unsigned int unActionSet);
	virtual void CancelEvent(unsigned int unAction);
	virtual void CancelCurrentTouchEvent(bool bInformTarget);
	virtual void CancelAllEvents();
	virtual bool KeyPressed(unsigned int unKeyCode);

	virtual bool LoadGlypthsForActionSet(unsigned int unActionSet, int nWidth);
	virtual const char* GetGlyphFilenameForDigitalAction(unsigned int unAction, unsigned int unActionSet, int nWidth);
	virtual const char* GetGlyphFilenameForAnalogAction(unsigned int unAction, unsigned int unActionSet, int nWidth);
	virtual void GetActionCountForActionSet(unsigned int unActionSet, unsigned int* pnDigitalActionCount, unsigned int* pnAnalogActionCount);
	virtual unsigned int GetDigitalActionAtIndexForActionSet(unsigned int unActionSet, int nIndex);
	virtual unsigned int GetAnalogActionAtIndexForActionSet(unsigned int unActionSet, int nIndex);

	static int GetInternalActionStartIndex();


private:
	void SetManifestFileLocation();
	bool ReadConfigFile();
	void RegisterInputCharSets();
	InputCharSet* GetCharSet(CharSet eCS);


	void PollSteamInput();
	ConnectorConnectionAction FindActiveSteamInputDevice();
	void PopulateControllerIdMapFromConnectedArray(InputHandle_t* pHandles, int nCount);

	void CheckKey(int keyToCheck, const std::string& keyId);
	void ProcessInternalActions();


private:
	Window* m_pWnd;
	IGameInput* m_pGameInput;
	bool m_bInputConfigRead;
	DigitalActions* m_pDigitalActions;
	
	InputHandle_t m_hActiveController;
	unsigned int m_unActiveControllerId;
	InputHandle_t m_hPrevActiveController;
	unsigned int m_unCurrentActionSet;

	std::map<unsigned int, ActionSet*> m_mpActionSetIdToActionSets;

	std::map<unsigned int, DigitalActionType > m_mpActionType;
	std::map<unsigned int, double> m_mpActionNextFireTime;
	std::map<unsigned int, bool> m_mpActionIsPressed;

	std::list<ITouchLayer* > m_lsTouchLayers;
	// Each "window", a tab with tab-bar and interactive elements, an alert, contain 1 or more touch layers.
	std::stack<unsigned int> m_stLayersPerWindow;

	unsigned int m_unCurrentPressAction;
	unsigned int m_unCurrentActionSetForPressAction;
	int m_nInvokeValueForCurrentPressAction;

	int m_nPrevActiveControllerCount;
	std::atomic<unsigned int> m_unNextControllerId;
	// Friendlier ids, not reliant on Steam handles
	std::map<unsigned int, InputHandle_t> m_mpIControllerIdToId;

	std::vector<unsigned int> m_vcInternalActions;

	std::map<CharSet, InputCharSet* > m_mpInputCharSets;
	CharSet m_eActiveCharSet;
	ITextInputTarget* m_pTextInputTarget;
	bool m_bKeyboardDisplayed;
};

