#pragma once

class IOverlayTab {
public:
	virtual ~IOverlayTab() {}
	virtual int AddText(std::string sText) = 0;
	virtual int AddTextEdit(std::string sLabel, std::string& sExisting, int nMaxChars) = 0;
	virtual int AddTextEdit(std::string sLabel, std::string& sExisting, char cPasswordChar, int nMaxChars) = 0;
	virtual void AddSpacer(unsigned int unHeight, unsigned int unDividerType = 0) = 0;
	virtual int AddNumericSelector(std::string sLabel, int* pnValue, int nMin, int nMaxInclusive, int nStep) = 0;
	virtual int AddButton(std::string sButtonLabel) = 0;
	virtual int AddButton(std::string sButtonLabel, int nTargetAction) = 0;
	virtual int AddButton(TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault) = 0;
	virtual int AddButtonBar() = 0;
	virtual int AddButtonToButtonBar(int nButtonBarId, std::string sButtonLabel, int nTargetAction) = 0;
	virtual int AddButtonToButtonBar(int nButtonBarId, TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault) = 0;
	virtual void EnableElement(int nElementId, bool bEnabled) = 0;
	virtual void ShowDebugBackgroundOnControlType(OverlayControlType eType, bool bShow) = 0;
};