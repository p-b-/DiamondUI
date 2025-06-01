#include "ActionSet.h"

ActionSet::ActionSet() {
	m_hSteamHandle = 0;
}

ActionSet::~ActionSet() {
	for (auto d : m_mpActionToTouchTriggers) {
		delete d.second;
	}
}

void ActionSet::CancelAllEvents() {
	m_mpActionTriggeredFlags.clear();
	m_mpMouseActionTriggeredFlags.clear();
	for (auto d : m_mpActionToTouchTriggers) {
		delete d.second;
	}
	m_mpActionToTouchTriggers.clear();
}

void ActionSet::GetActionCount(unsigned int* pnDigitalActionCount, unsigned int* pnAnalogActionCount) {
	*pnDigitalActionCount = static_cast<unsigned int>(m_mpControllerDigitalActionHandles.size());
	*pnAnalogActionCount = static_cast<unsigned int>(m_mpControllerAnalogActionHandles.size());
}

unsigned int ActionSet::GetDigitalActionAtIndex(int nIndex) {
	// TODO Provide an iterator to EngineInput instead of this
	int n = 0;
	for (auto i : m_mpControllerDigitalActionHandles) {
		if (n == nIndex) {
			return i.first;
		}
		n++;
	}
	return 0;
}

unsigned int ActionSet::GetAnalogActionAtIndex(int nIndex) {
	// TODO Provide an iterator to EngineInput instead of this
	int n = 0;
	for (auto i : m_mpControllerAnalogActionHandles) {
		if (n == nIndex) {
			return i.first;
		}
		n++;
	}
	return 0;
}

void ActionSet::RegisterControllerDigitalAction(unsigned int unAction, InputDigitalActionHandle_t handle) {
	m_mpControllerDigitalActionHandles[unAction] = handle;
}

void ActionSet::RegisterControllerAnalogAction(unsigned int unAction, InputAnalogActionHandle_t handle) {
	m_mpControllerAnalogActionHandles[unAction] = handle;
}

bool ActionSet::GetDigitalHandleFromAction(unsigned int unAction, InputDigitalActionHandle_t* pHandle) const {
	auto find = m_mpControllerDigitalActionHandles.find(unAction);
	if (find == m_mpControllerDigitalActionHandles.end()) {
		return false;
	}
	*pHandle = find->second;
	return true;
}

bool ActionSet::GetAnalogHandleFromAction(unsigned int unAction, InputAnalogActionHandle_t* pHandle) const {
	auto find = m_mpControllerAnalogActionHandles.find(unAction);
	if (find == m_mpControllerAnalogActionHandles.end()) {
		return false;
	}
	*pHandle = find->second;
	return true;
}

void ActionSet::RegisterKeyboardScanCode(int scanCode, unsigned int unAction) {
	m_mpScanCodeToAction[scanCode] = unAction;
}

void ActionSet::RegisterMouseScanCode(int scanCode, unsigned int unAction) {
	m_mpMouseScanCodesToAction[scanCode] = unAction;
}

bool ActionSet::GetActionFromKeyboardScanCode(int nScanCode, unsigned int* punAction) const {
	auto findAction = m_mpScanCodeToAction.find(nScanCode);
	if (findAction == m_mpScanCodeToAction.end()) {
		return false;
	}
	*punAction = findAction->second;
	return true;
}

bool ActionSet::GetActionFromMouseScanCode(int nScanCode, unsigned int* punAction) const {
	auto findAction = m_mpMouseScanCodesToAction.find(nScanCode);
	if (findAction == m_mpMouseScanCodesToAction.end()) {
		return false;
	}
	*punAction = findAction->second;
	return true;
}

void ActionSet::SetKeyActionTriggeredState(unsigned int unAction, bool bTriggered) {
	m_mpActionTriggeredFlags[unAction] = bTriggered;
}

void ActionSet::SetMouseActionTriggeredState(unsigned int unAction, bool bTriggered) {
	m_mpMouseActionTriggeredFlags[unAction] = bTriggered;
}

bool ActionSet::GetKeyboardActionTriggeredState(unsigned int unAction) const { 
	auto find = m_mpActionTriggeredFlags.find(unAction);
	if (find == m_mpActionTriggeredFlags.end()) {
		return false;
	}
	return find->second;
}

bool ActionSet::GetMouseActionTriggeredState(unsigned int unAction) const { 
	auto find = m_mpMouseActionTriggeredFlags.find(unAction);
	if (find == m_mpMouseActionTriggeredFlags.end()) {
		return false;
	}
	return find->second;
}

void ActionSet::RemoveTouchTriggerForAction(unsigned int unAction) {
	auto find = m_mpActionToTouchTriggers.find(unAction);
	if (find != m_mpActionToTouchTriggers.end()) {
		delete find->second;
		m_mpActionToTouchTriggers.erase(find);
	}
}

void ActionSet::CreateNewTouchTriggerForAction(unsigned int unAction, ITouchTarget* pTarget, int nInvokeValue) {
	m_mpActionToTouchTriggers[unAction] = new TouchTrigger{ true, pTarget, true, nInvokeValue };
}

TouchTrigger* ActionSet::GetTouchTriggerForAction(unsigned int unAction) {
	auto findExistingTouch = m_mpActionToTouchTriggers.find(unAction);
	if (findExistingTouch != m_mpActionToTouchTriggers.end()) {
		return findExistingTouch->second;
	}
	return nullptr;
}

void ActionSet::PopulateGlyphFilenamesForController(InputHandle_t hController, ESteamInputGlyphSize eSize) {
	m_mpDigitalActionToGlypthFilename.clear();
	for (auto a : m_mpControllerDigitalActionHandles) {
		EInputActionOrigin* pOrigins = new EInputActionOrigin[STEAM_INPUT_MAX_ORIGINS];
		int count = SteamInput()->GetDigitalActionOrigins(hController, m_hSteamHandle, a.second, pOrigins);
		if (count > 0) {
			m_mpDigitalActionToGlypthFilename[a.first] =  SteamInput()->GetGlyphPNGForActionOrigin(pOrigins[0], eSize, 0);
		}
	}
	m_mpAnalogActionToGlypthFilename.clear();
	for (auto a : m_mpControllerAnalogActionHandles) {
		EInputActionOrigin* pOrigins = new EInputActionOrigin[STEAM_INPUT_MAX_ORIGINS];
		int count = SteamInput()->GetAnalogActionOrigins(hController, m_hSteamHandle, a.second, pOrigins);
		if (count > 0) {
			m_mpAnalogActionToGlypthFilename[a.first] = SteamInput()->GetGlyphPNGForActionOrigin(pOrigins[0], eSize, 0);
		}
	}
}

const char* ActionSet::GetGlyphFilenameForDigitalAction(unsigned int unAction, ESteamInputGlyphSize eSize) {
	auto find = m_mpDigitalActionToGlypthFilename.find(unAction);
	if (find == m_mpDigitalActionToGlypthFilename.end()) {
		return nullptr;
	}
	return find->second;
}

const char* ActionSet::GetGlyphFilenameForAnalogAction(unsigned int unAction, ESteamInputGlyphSize eSize) {
	auto find = m_mpAnalogActionToGlypthFilename.find(unAction);
	if (find == m_mpAnalogActionToGlypthFilename.end()) {
		return nullptr;
	}
	return find->second;
}
