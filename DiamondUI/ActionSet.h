#pragma once
#include "ITouchTarget.h"
#include <steam/steam_api.h>
#include <map>

struct TouchTrigger {
	bool bPressed;
	ITouchTarget* pTarget;
	bool bCurrentlyOverTarget;
	int nInvokeValue;
};

class ActionSet
{
public:
	ActionSet();
	~ActionSet();

public:
	InputActionSetHandle_t GetHandle() const { return m_hSteamHandle; }
	void SetHandle(InputActionSetHandle_t handle) { m_hSteamHandle = handle; }

	void CancelAllEvents();

	void GetActionCount(unsigned int* pnDigitalActionCount, unsigned int* pnAnalogActionCount);
	unsigned int GetDigitalActionAtIndex(int nIndex);
	unsigned int GetAnalogActionAtIndex(int nIndex);

	void RegisterControllerDigitalAction(unsigned int unAction, InputDigitalActionHandle_t handle);
	void RegisterControllerAnalogAction(unsigned int unAction, InputAnalogActionHandle_t handle);
	bool GetDigitalHandleFromAction(unsigned int unAction, InputDigitalActionHandle_t* pHandle) const;
	bool GetAnalogHandleFromAction(unsigned int unAction, InputAnalogActionHandle_t* pHandle) const;

	void RegisterKeyboardScanCode(int scanCode, unsigned int unAction);
	void RegisterMouseScanCode(int scanCode, unsigned int unAction);
	bool GetActionFromKeyboardScanCode(int nScanCode, unsigned int* punAction) const;
	bool GetActionFromMouseScanCode(int nScanCode, unsigned int* punAction) const;

	void SetKeyActionTriggeredState(unsigned int unAction, bool bTriggered);
	void SetMouseActionTriggeredState(unsigned int unAction, bool bTriggered);
	bool GetKeyboardActionTriggeredState(unsigned int unAction) const;
	bool GetMouseActionTriggeredState(unsigned int unAction) const;

	void CreateNewTouchTriggerForAction(unsigned int unAction, ITouchTarget* pTarget, int nInvokeValue);
	void RemoveTouchTriggerForAction(unsigned int unAction);
	TouchTrigger* GetTouchTriggerForAction(unsigned int unAction);
	
	void PopulateGlyphFilenamesForController(InputHandle_t hController, ESteamInputGlyphSize eSize);
	const char* GetGlyphFilenameForDigitalAction(unsigned int unAction, ESteamInputGlyphSize eSize);
	const char* GetGlyphFilenameForAnalogAction(unsigned int unAction, ESteamInputGlyphSize eSize);

private:
	InputActionSetHandle_t m_hSteamHandle;

	std::map<int, unsigned int> m_mpScanCodeToAction;
	std::map<int, unsigned int> m_mpMouseScanCodesToAction;

	// A map of game 'enums' to steam handle for digital Steam Controller events
	std::map<unsigned int, InputDigitalActionHandle_t> m_mpControllerDigitalActionHandles;
	// A map of game 'enums' to steam handle for analog Steam Controller events
	std::map<unsigned int, InputAnalogActionHandle_t> m_mpControllerAnalogActionHandles;

	// A map of actions to whether that action (key/mouse press) is triggered.
	std::map<unsigned int, bool> m_mpActionTriggeredFlags;
	std::map<unsigned int, bool> m_mpMouseActionTriggeredFlags;
	// Similar to above, but instead of a simple trigger, it stores what caused the trigger as well as the trigger flag.
	std::map<unsigned int, TouchTrigger* > m_mpActionToTouchTriggers;

	std::map< unsigned int, const char*> m_mpDigitalActionToGlypthFilename;
	std::map< unsigned int, const char*> m_mpAnalogActionToGlypthFilename;
};

