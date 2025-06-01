#include "OverlayTab.h"
#include "TextElement.h"
#include "TextEditElement.h"
#include "SpacerElement.h"
#include "ScrollbarElement.h"
#include "NumericSelectorElement.h"
#include "ButtonElement.h"
#include "ButtonBarElement.h"
#include "TouchLayer.h"
#include "IOverlayEnv.h"

#include <iostream>

// Space around displayable area within tab
#define TAB_MARGIN 10
// Space around text in titles
#define TAB_TITLE_MARGIN 15
// Area between selected tab title, and the main tab body
#define TITLE_TO_TAB_CONNECTOR_HEIGHT 5
//#define SCROLLBAR_WIDTH 20
//#define INTRA_ELEMENT_SPACING 2
// Space above and below text in title
#define TAB_TITLE_VERTICAL_MARGIN 20

#define SCROLLBAR_INVOKE_VALUE 0x10000
//#define TABBACKDROP_SCROLLTARGETVALUE 0x10001

// Width of buttons for shift left/right tabs
#define TAB_BUTTON_WIDTH (30+TAB_TITLE_VERTICAL_MARGIN *2)

OverlayTab::OverlayTab(std::string sTitle, float fWidth, float fHeight, OverlayLabelAlignment eLabelAlignment, IOverlayIntl* pOverlay) :
	m_elements(this) {
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_xTitleOriginFromLastRender = -1;
	m_eLabelAlignment = eLabelAlignment;
	m_pOverlay = pOverlay;
	m_pOverlayEnv = m_pOverlay->GetInternalOverlayEnvironment();

	m_bDisplayed = false;

	m_yLastRenderedOrigin = 0;
	//m_yScrollOffset = 0.0f;
	m_pTouchLayer = nullptr;
	m_bHighlightedAsPressed = false;

	m_fContentHeight = 0.0f;

	m_pTLTitle = nullptr;

	SetTitle(sTitle);
}

OverlayTab::~OverlayTab() {
	delete m_pTLTitle;
}

void OverlayTab::SetTitle(const std::string& sTitle)  {
	m_sTitle = sTitle;
	UpdateTitleTextLine();
}

int OverlayTab::GetOverlayTabButtonWidth() {
	return TAB_BUTTON_WIDTH;
}

int OverlayTab::GetOverlayTabButtonConnectingHeight() {
	return TITLE_TO_TAB_CONNECTOR_HEIGHT;
}

bool OverlayTab::TabDataAltered() {
	return m_elements.DataAltered();
}

void OverlayTab::RevertTabData() {
	m_elements.RevertData();
}

bool OverlayTab::SaveTabData() {
	return m_elements.SaveData();
}

bool OverlayTab::CanClose() const {
	return m_elements.CanClose();
}

void OverlayTab::NotRendering() {
	m_xTitleOriginFromLastRender = -1;
}

void OverlayTab::Render(bool bTabDisplayed, float xTitleOffset, float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight) {
	m_yLastRenderedOrigin = static_cast<int>(yDrawPortalOrigin);
	int heightUsedForTitle;
	if (m_bSoleTab) {
		SetClippingArea(xDrawPortalOrigin, yDrawPortalOrigin, fDrawPortalWidth, fDrawPortalHeight);
		heightUsedForTitle = RenderCentredTitle(xDrawPortalOrigin, yDrawPortalOrigin, fDrawPortalWidth, fDrawPortalHeight);
		RemoveClippingArea();
	}
	else {
		if (bTabDisplayed) {
			// Render next/prev tab buttons
			rgba rgbaButton = std::make_tuple(1.0f, 0.0f, 0.0f, 1.0f);
			rgba rgbaButton2 = std::make_tuple(0.0f, 1.0f, 0.0f, 1.0f);
			float yButtonOrigin = yDrawPortalOrigin + fDrawPortalHeight - TAB_BUTTON_WIDTH;
			float fButtonWidth = TAB_BUTTON_WIDTH;
			RenderGlypth(xDrawPortalOrigin, yButtonOrigin, fButtonWidth, fButtonWidth, m_pOverlayEnv->GetPrevTabAction());
			RenderGlypth(xDrawPortalOrigin + fDrawPortalWidth - fButtonWidth, yButtonOrigin, fButtonWidth, fButtonWidth, m_pOverlayEnv->GetNextTabAction());
		}
		SetClippingArea(xDrawPortalOrigin + TAB_BUTTON_WIDTH, yDrawPortalOrigin, fDrawPortalWidth - TAB_BUTTON_WIDTH * 2, fDrawPortalHeight);
		heightUsedForTitle = RenderTabTitle(bTabDisplayed, xTitleOffset, xDrawPortalOrigin, yDrawPortalOrigin, fDrawPortalWidth, fDrawPortalHeight);
		RemoveClippingArea();
	}
	SetClippingArea(xDrawPortalOrigin, yDrawPortalOrigin, fDrawPortalWidth, fDrawPortalHeight);
	if (bTabDisplayed) {
		float xTabOrigin = xDrawPortalOrigin;
		float yTabOrigin = yDrawPortalOrigin;
		float tabWidth = fDrawPortalWidth;
		float tabHeight = fDrawPortalHeight - heightUsedForTitle;
		RenderTab(xTabOrigin, yTabOrigin, tabWidth, tabHeight);
	}
	RemoveClippingArea();
}

void OverlayTab::SetSoleTab(bool bSoleTab) {
	m_bSoleTab = bSoleTab;
}

int OverlayTab::RenderTabTitle(bool bHighlighted, float xTitleOffset, float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight) {
	// Render background
	float xOrigin = static_cast<float>(xDrawPortalOrigin + xTitleOffset + TAB_BUTTON_WIDTH);
	float yOrigin = yDrawPortalOrigin + fDrawPortalHeight - m_fTitleBackgroundHeight; // m_fTitleBackgroundHeight already have vertical margins
	float fWidth = m_fTitleBackgroundWidth;
	float fHeight = m_fTitleBackgroundHeight;

	if (bHighlighted) {
		yOrigin -= TITLE_TO_TAB_CONNECTOR_HEIGHT;
		fHeight += TITLE_TO_TAB_CONNECTOR_HEIGHT;
	}
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	rgba rgbaTabBackground = pPublicEnv->GetTabBackgroundRGBA();
	if (bHighlighted) {
		rgbaTabBackground = MakeRGBA(0.0f, 0.0f, 1.0f, 1.0f);
	}

	SetClippingArea(xOrigin, yOrigin, fWidth, fHeight);
	RenderTranslucentOverlay(xOrigin, yOrigin, fWidth, fHeight, rgbaTabBackground);

	rgb rgbTitle;
	if (bHighlighted || m_bHighlightedAsPressed) {
		rgbTitle = pPublicEnv->GetTitleHighlightedRGB();
	}
	else {
		rgbTitle = pPublicEnv->GetTitleRGB();
	}

	m_xTitleOriginFromLastRender = xOrigin;
	s_pTextRenderer->RenderTextLine(m_pTLTitle, static_cast<float>(xOrigin + TAB_TITLE_MARGIN), yDrawPortalOrigin + fDrawPortalHeight - m_fTitleHeight - TAB_TITLE_VERTICAL_MARGIN, rgbTitle);
	RemoveClippingArea();

	return static_cast<int>(fHeight);
}

int OverlayTab::RenderCentredTitle(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight) {
	// Render background
	float xOrigin = xDrawPortalOrigin+(fDrawPortalWidth - m_fTitleBackgroundWidth)/2.0f;
	float yOrigin = yDrawPortalOrigin + fDrawPortalHeight - m_fTitleBackgroundHeight;
	float fWidth = m_fTitleBackgroundWidth;
	float fHeight = m_fTitleBackgroundHeight;

	yOrigin -= TITLE_TO_TAB_CONNECTOR_HEIGHT;
	fHeight += TITLE_TO_TAB_CONNECTOR_HEIGHT;

	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	rgba rgbaTabBackground = pPublicEnv->GetTabBackgroundRGBA();

	SetClippingArea(xOrigin, yOrigin, fWidth, fHeight);
	RenderTranslucentOverlay(xOrigin, yOrigin, fWidth, fHeight, rgbaTabBackground);

	rgb rgbTitle = pPublicEnv->GetTitleHighlightedRGB();
	m_xTitleOriginFromLastRender = xOrigin;
	s_pTextRenderer->RenderTextLine(m_pTLTitle, static_cast<float>(xOrigin + TAB_TITLE_MARGIN), static_cast<float>(yDrawPortalOrigin + fDrawPortalHeight - m_fTitleHeight - TAB_TITLE_VERTICAL_MARGIN), rgbTitle);
	RemoveClippingArea();

	return static_cast<int>(fHeight);
}

void OverlayTab::RenderTab(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight) {
	m_elements.Render(xDrawPortalOrigin, yDrawPortalOrigin, fDrawPortalWidth, fDrawPortalHeight);
}

void OverlayTab::TitleFontUpdated() {
	UpdateTitleTextLine();
}

void OverlayTab::TextFontUpdated() {
}

void OverlayTab::UpdateTitleTextLine() {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	FontFaceHandle hTitleFont = pPublicEnv->GetTitleFontFace();
	if (!hTitleFont.IsValid()) {
		return;
	}
	if (m_pTLTitle != nullptr) {
		delete m_pTLTitle;
		m_pTLTitle = nullptr;
	}
	// TODO Determine max title width from overlay size
	m_pTLTitle = s_pTextRenderer->CreateTextLine(hTitleFont, m_sTitle, (float)400, 1.0f);
	auto sz = s_pTextRenderer->MeasureText(hTitleFont, m_sTitle, 1.0f);
	m_fTitleHeight = s_pTextRenderer->GetFontHeight(hTitleFont);
	m_fTitleWidth = (float)std::get<2>(sz);

	m_fTitleBackgroundWidth = m_fTitleWidth + TAB_TITLE_MARGIN * 2;
	m_fTitleBackgroundHeight = m_fTitleHeight  + TAB_TITLE_VERTICAL_MARGIN * 2;
}

// TODO Separate the following into an interface
// Called by IOverlay and other internals
//
void OverlayTab::Move(float fXOffset, float fYOffset) {
	if (m_pTouchLayer != nullptr) {
		m_pTouchLayer->MoveLayer(fXOffset, fYOffset);
	}
}

float OverlayTab::GetTabTitleWidth() const {
	return m_fTitleBackgroundWidth;
}

float OverlayTab::GetTabTitleHeight() const {
	return m_fTitleBackgroundHeight;
}

float OverlayTab::GetXTitleOriginFromLastRender() const {
	return m_xTitleOriginFromLastRender;
}

void OverlayTab::ResetDisplayState() {
	m_elements.ResetDisplayState();
}

void OverlayTab::Displaying(bool bDisplay) {
	m_elements.Displaying(bDisplay);
	m_bDisplayed = bDisplay;
}

void OverlayTab::Animate(float fDeltaTime) {
	m_elements.Animate(fDeltaTime);
}
//
// Called by IOverlay and other internals

// IOverlayTab implementation
//
int OverlayTab::AddText(std::string sText) {
	int elementId = m_pOverlay->NextElementId();
	TextElement* pElement = new TextElement(m_pOverlay, elementId, sText);
	
	m_elements.AddElement(pElement);

	return elementId;
}

int OverlayTab::AddTextEdit(std::string sLabel, std::string& sExisting, int nMaxChars) { 
	int elementId = m_pOverlay->NextElementId();
	TextEditElement* pElement = new TextEditElement(m_pOverlay, elementId, sLabel, sExisting, m_eLabelAlignment, nMaxChars);
	pElement->HideText(false, '\0');
	
	m_elements.AddElement(pElement);

	return elementId;
}

int OverlayTab::AddTextEdit(std::string sLabel, std::string& sExisting, char cPasswordChar, int nMaxChars) { 
	int elementId = m_pOverlay->NextElementId();
	TextEditElement* pElement = new TextEditElement(m_pOverlay, elementId, sLabel, sExisting, m_eLabelAlignment, nMaxChars);
	pElement->HideText(true, cPasswordChar);
	
	m_elements.AddElement(pElement);
	return elementId;
}

void OverlayTab::AddSpacer(unsigned int unHeight, unsigned int unDividerType /*= 0*/) {
	SpacerElement* pElement = new SpacerElement(m_pOverlay, unHeight, unDividerType);
	m_elements.AddElement(pElement);
}

int OverlayTab::AddNumericSelector(std::string sLabel, int* pnValue, int nMin, int nMaxInclusive, int nStep) {
	int elementId = m_pOverlay->NextElementId();
	NumericSelectorElement* pElement = new NumericSelectorElement(m_pOverlay, elementId, sLabel, m_eLabelAlignment, pnValue, nMin, nMaxInclusive, nStep);
	
	m_elements.AddElement(pElement);

	return elementId;
}

int OverlayTab::AddButton(std::string sButtonLabel) {
	int elementId = m_pOverlay->NextElementId();
	// Although buttons are not labelled separately, the element needs to know where a label would go for other elements, to ensure it matches the same style
	ButtonElement* pElement = new ButtonElement(m_pOverlay, elementId, sButtonLabel, m_eLabelAlignment);
	
	m_elements.AddElement(pElement);

	return elementId;
}

int OverlayTab::AddButton(std::string sButtonLabel, int nTargetAction) {
	// Although buttons are not labelled separately, the element needs to know where a label would go for other elements, to ensure it matches the same style
	int elementId = m_pOverlay->NextElementId();
	ButtonElement* pElement = new ButtonElement(m_pOverlay, elementId, sButtonLabel, m_eLabelAlignment);
	if (nTargetAction!=-1) {
		pElement->SetTargetAction(nTargetAction);
	}
	
	m_elements.AddElement(pElement);

	return elementId;
}

int OverlayTab::AddButton(TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault) {
	return -1;
}

int OverlayTab::AddButtonBar() {
	int elementId = m_pOverlay->NextElementId();
	ButtonBarElement* pElement = new ButtonBarElement(m_pOverlay, elementId, m_eLabelAlignment);
	
	m_elements.AddElement(pElement);
	return elementId;
}

int OverlayTab::AddButtonToButtonBar(int nButtonBarId, std::string sButtonLabel, int nTargetAction) {
	int nElementId = -1;
	ButtonBarElement* pButtonBar = dynamic_cast<ButtonBarElement*>(m_elements.GetElement(nButtonBarId));

	if (pButtonBar != nullptr) {
		nElementId = m_pOverlay->NextElementId();
		ButtonElement* pButton = pButtonBar->AddButton(nElementId, eTABTNoneTabAction, sButtonLabel);
		if (nTargetAction != -1) {
			pButton->SetTargetAction(nTargetAction);
		}
		pButtonBar->InitialiseForWidth(GetDisplayableTabWidth());
		m_elements.AddElementToMap(pButton);

		m_elements.ControlResized();
	}
	else {
		// TODO Create error: No current button bar
		return -1;
	}
	return true;
}

int OverlayTab::AddButtonToButtonBar(int nButtonBarId, TabActionButtonType eButtonType, std::string sButtonLabel, bool bDefault) {
	int nElementId = - 1;
	ButtonBarElement* pButtonBar = dynamic_cast<ButtonBarElement* >(m_elements.GetElement(nButtonBarId));

	if (pButtonBar != nullptr) {
		nElementId = m_pOverlay->NextElementId();
		ButtonElement* pButton = pButtonBar->AddButton(nElementId, eButtonType, sButtonLabel);
		pButtonBar->InitialiseForWidth(GetDisplayableTabWidth());
		m_elements.AddElementToMap(pButton);
		m_elements.ControlResized();
	}
	else {
		// TODO Create error: No current button bar
	}
	return nElementId;
}

void OverlayTab::EnableElement(int nElementId, bool bEnabled) {
	m_elements.EnableElement(nElementId, bEnabled);
}

void OverlayTab::ShowDebugBackgroundOnControlType(OverlayControlType eType, bool bShow) {
	m_elements.ShowDebugBackgroundOnControlType(eType, bShow);
}
//
// IOverlayTab implementation

float OverlayTab::GetDisplayableTabWidth() {
	float fWidth = m_fWidth - 2 * TAB_MARGIN;
	if (m_elements.DisplayingScrollbar()) {
		fWidth -= ScrollbarElement::GetWidth();
	}
	return fWidth;
}

float OverlayTab::GetDisplayableTabHeight() {
	float fHeight = m_fHeight - m_fTitleBackgroundHeight - TITLE_TO_TAB_CONNECTOR_HEIGHT - TAB_MARGIN * 2;
	return fHeight;
}

void OverlayTab::HighlightAsPressed(bool bHighlight) {
	m_bHighlightedAsPressed = bHighlight;
}


ActionResult OverlayTab::Action(unsigned int unAction) {
	// Action comes from the controller, whereas an Invoke comes 'directly' from the the touch area

	if (unAction == m_pOverlayEnv->GetMoveUpAction() || (m_pScrollbar!=nullptr && unAction == m_pScrollbar->GetScrollUpAction())) {
		return m_elements.MoveUp();
	}
	else if (unAction == m_pOverlayEnv->GetMoveDownAction() || (m_pScrollbar != nullptr && unAction == m_pScrollbar->GetScrollDownAction())) {
		return m_elements.MoveDown();
	}
	else if (m_elements.HasHighlightedElement()) {
		m_elements.EnsureHighlightedElementFullyDisplayed();
		return m_elements.Action(unAction);
	}
	return ActionResult(false);
}

void OverlayTab::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin) {
	// This object gains control of TouchLayer's lifetime
	m_pTouchLayer = pTouchLayer;

	float fWidth = m_fWidth - 2 * TAB_MARGIN;

	// To calculate the topY of the drawable tab area, need to know the title height. Which currently is unknown due to 
	//  it not being rendered yet. This is the process used to calculate it:
	float fTitleHeight = m_fTitleBackgroundHeight + TITLE_TO_TAB_CONNECTOR_HEIGHT;
	float fHeight = 0.0f;
	yOrigin += TAB_MARGIN;
	if (m_bSoleTab) {
		//fHeight = m_fTitleBackgroundHeight + 2 * TAB_TITLE_VERTICAL_MARGIN + TITLE_TO_TAB_CONNECTOR_HEIGHT;
		fHeight = m_fHeight - fTitleHeight - 2 * TAB_MARGIN;
	}
	else {
		fHeight = m_fHeight - fTitleHeight - 2 * TAB_MARGIN;
	}

	m_elements.InitialiseTouchAreas(pTouchLayer, xOrigin + TAB_MARGIN, yOrigin, fWidth, fHeight);
}
