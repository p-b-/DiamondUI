#pragma once
#include "ColourDefs.h"
#include "Overlay.h"
#include "OverlayGraphics.h"
#include "IOverlayTab.h"
#include "ITextRenderer.h"
#include "ElementCollection.h"
#include <string>
#include <tuple>

class IOverlayElement;
class ScrollbarElement;
class ButtonBarElement;
class TouchLayer;
class IOverlayEnvIntl;

class OverlayTab : 
	public OverlayGraphics, public IOverlayTab, public IElementCollectionContainer {
public:
	OverlayTab(std::string sTitle, float fWidth, float fHeight, OverlayLabelAlignment eLabelAlignment, IOverlayIntl* pOverlay);
	virtual ~OverlayTab();
	const std::string& GetTitle() const { return m_sTitle; }
	void SetTitle(const std::string& sTitle);
	void SetSoleTab(bool bSoleTab);

	static int GetOverlayTabButtonWidth();
	static int GetOverlayTabButtonConnectingHeight();

	// For IOverlay and IScrollbarElement to call:
	void Render(bool bTabDisplayed, float fxTitleOffset, float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight);
	void NotRendering();
	void TitleFontUpdated();
	void TextFontUpdated();
	bool TabDataAltered();
	void RevertTabData();
	bool SaveTabData();
	bool CanClose() const;
	void Move(float fXOffset, float fYOffset);
	float GetTabTitleWidth() const;
	float GetTabTitleHeight() const;
	float GetXTitleOriginFromLastRender() const;
	void ResetDisplayState();
	void Displaying(bool bDisplay);
	void Animate(float fDeltaTime);

	// IOverlayTab declarations
	virtual int AddText(std::string sText);
	virtual int AddTextEdit(std::string sLabel, std::string& sExisting, int nMaxChars);
	virtual int AddTextEdit(std::string sLabel, std::string& sExisting, char cPasswordChar, int nMaxChars);
	virtual void AddSpacer(unsigned int unHeight, unsigned int unDividerType = 0);
	virtual int AddNumericSelector(std::string sLabel, int* pnValue, int nMin, int nMaxInclusive, int nStep);
	virtual int AddButton(std::string sButtonLabel);
	virtual int AddButton(std::string sButtonLabel, int nTargetAction);
	virtual int AddButton(TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault);
	virtual int AddButtonBar();
	virtual int AddButtonToButtonBar(int nButtonBarId, std::string sButtonLabel, int nTargetAction);
	virtual int AddButtonToButtonBar(int nButtonBarId, TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault);
	//virtual int AddButtonToButtonBar(std::string sButtonLabel, int nTargetAction);
	//virtual int AddButtonToButtonBar(TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault);
	virtual void EnableElement(int nElementId, bool bEnabled);
	virtual void ShowDebugBackgroundOnControlType(OverlayControlType eType, bool bShow);
	
	void HighlightAsPressed(bool bHighlight);
	ActionResult Action(unsigned int unAction);

	void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin);

	// IElementCollectionContainer declarations
public:
	virtual float GetDisplayableHeight() { return GetDisplayableTabHeight(); }
	virtual float GetDisplayableWidth() { return GetDisplayableTabWidth(); }
	virtual bool GetCentreELementsHorizontally() const { return true; }
	virtual IOverlayIntl* GetOverlay() { return m_pOverlay; }
	virtual const IOverlayEnvIntl* GetEnvironment() const { return m_pOverlayEnv; }
	virtual bool GetContainerIsTabNotView() { return true; }
	virtual bool GetContainerAspectIsVertical() { return true; }
	
private:
	int RenderTabTitle(bool bHighlighted, float xTitleOffset, float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight);
	int RenderCentredTitle(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight);
	void RenderTab(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight);

	void UpdateTitleTextLine();
	float GetDisplayableTabWidth();
	float GetDisplayableTabHeight();

private:
	std::string m_sTitle;
	float m_fWidth;
	float m_fHeight;
	OverlayLabelAlignment m_eLabelAlignment;
	IOverlayIntl* m_pOverlay;
	const IOverlayEnvIntl* m_pOverlayEnv;
	bool m_bDisplayed;

	bool m_bSoleTab;

	ITextLine* m_pTLTitle;
	float m_fTitleHeight;
	float m_fTitleBackgroundWidth;
	float m_fTitleBackgroundHeight;
	float m_fTitleWidth;

	float m_xTitleOriginFromLastRender;

	ElementCollection m_elements;

	int m_yLastRenderedOrigin;
	ScrollbarElement* m_pScrollbar;

	TouchLayer* m_pTouchLayer;
	bool m_bHighlightedAsPressed;

	float m_fContentHeight;
};
