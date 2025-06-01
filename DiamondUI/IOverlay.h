#pragma once

#include "OverlayDefs.h"

class IOverlayTab;
class IOverlayView;
class IOverlayEnv;
class IOverlay {
public:
	virtual ~IOverlay() {}
	virtual void Render(float fDeltaTime) = 0;
	virtual void Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float fWidth, float fHeight) = 0;
	virtual void Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float xOrigin, float yOrigin, float fWidth, float fHeight) = 0;
	virtual void InitialiseFullScreen(OverlayLabelAlignment eAlignment, bool bRenderBackdrop, float fWidth, float fHeight) = 0;
	virtual void WindowSizeChanged(int nWidth, int nHeight) = 0; 
	virtual void InitialiseTouchAreas() = 0;

	virtual bool CanClose() const = 0;
	virtual bool SaveData() = 0;
	virtual void ChangeDisplayState(bool bDisplayingNotHiding) = 0;
	virtual IOverlayEnv* GetOverlayEnvironment() const = 0;

	virtual ActionResult Action(unsigned int unAction) = 0;
	virtual IOverlayTab* AddTab(std::string sTabTitle) = 0;
	virtual IOverlayView* AddView(float fXOrigin, float fYOrigin, float fWidth, float fHeight, bool bVerticalNotHoriz) = 0;
	virtual void ShowDebugBackgroundOnControl(OverlayControlType eControlType, bool bShow) = 0;
	virtual void ShowDebugBackgroundOnControlInTab(unsigned int unTab, OverlayControlType eControlType, bool bShow) = 0;
	virtual void DisplayAlert(std::string sText, std::string sButtonText, std::string sCancelText) = 0;

	virtual void SetButtonRenderFunc(fnRenderButton pRenderFunc) = 0;
	virtual void SetSelectorRenderFunc(fnRenderSelector pRenderFunc) = 0;
};
