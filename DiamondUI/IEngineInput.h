#pragma once

class ITouchLayer;
class ITouchTarget;

#include "InputDefs.h"

class IEngineInput
{
public:
	virtual ~IEngineInput() {}
	virtual bool IsDigitalActionActive(unsigned int unAction, ITouchTarget** ppTarget, int* pnInvokeValue) = 0;
	virtual bool GetControllerAnalogAction(unsigned int unAction, float* pfX, float* pfY) = 0;
	virtual bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, int nGLFWKeyToken, DigitalActionType eActionType) = 0;
	virtual bool RegisterKeyboardAction(unsigned int unAction, unsigned int unActionSet, const char key, DigitalActionType eActionType) = 0;
	virtual bool RegisterMouseAction(unsigned int unAction, unsigned int unActionSet, int nGLFWMouseToken, DigitalActionType eActionType) = 0;
	virtual bool RegisterTouchAction(unsigned int unAction, DigitalActionType eActionType) = 0;
	virtual bool RegisterControllerDigitalAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction, DigitalActionType eActionType) = 0;
	virtual bool RegisterControllerAnalogAction(unsigned int unAction, unsigned int unActionSet, const char* pszAction) = 0;
	virtual bool RegisterActionSet(unsigned int unActionSet, const char* pszActionSet) = 0;
	virtual void SetSteamControllerActionSet(unsigned int unActionSet) = 0;
	virtual void CancelEvent(unsigned int unAction) = 0;
	virtual void CancelCurrentTouchEvent(bool bInformTarget) = 0;
	virtual void CancelAllEvents() = 0;
	virtual bool KeyPressed(unsigned int unKeyCode) = 0;

	virtual bool LoadGlypthsForActionSet(unsigned int unActionSet, int nWidth) = 0;
	virtual const char* GetGlyphFilenameForDigitalAction(unsigned int unAction, unsigned int unActionSet, int nWidth) = 0;
	virtual const char* GetGlyphFilenameForAnalogAction(unsigned int unAction, unsigned int unActionSet, int nWidth) = 0;
	virtual void GetActionCountForActionSet(unsigned int unActionSet, unsigned int* pnDigitalActionCount, unsigned int* pnAnalogActionCount) = 0;
	virtual unsigned int GetDigitalActionAtIndexForActionSet(unsigned int unActionSet, int nIndex) = 0;
	virtual unsigned int GetAnalogActionAtIndexForActionSet(unsigned int unActionSet, int nIndex) = 0;
};

