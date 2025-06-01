#include "Overlay.h"
#include "OverlayView.h"
#include "OverlayTab.h"
#include "TouchLayer.h"
#include "IGameInput.h"
#include "IEngineInputIntl.h"
#include "ITextInputTarget.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define TAB_SPACING 15
#define OVERLAY_MARGIN 10

std::atomic<int> Overlay::s_nNextElementId = 1;
IEngineInputIntl* Overlay::s_pEngineInput;

Overlay::Overlay(IOverlayEnvIntl* pEnv)
    : m_fHeight(0.0f), m_fWidth(0.0f), m_unTabDisplayed(0), m_xOrigin(0), m_xTabOffset(0.0f), m_yOrigin(0), m_pTouchLayer(nullptr), m_pTabBarTouchLayer(nullptr) {
	m_eLabelAligment = eOLALeft;
	m_bKeepCentred = true;
	m_pAlertOverlay = nullptr;
	m_pEnvironment = pEnv;
	m_bRenderBackdrop = true;
	m_bTabsNotViews = true;
}

Overlay::~Overlay() {
	delete m_pTouchLayer;
	m_pTouchLayer = nullptr;

	delete m_pTabBarTouchLayer;
	m_pTabBarTouchLayer = nullptr;

	delete m_pAlertOverlay;
	m_pAlertOverlay = nullptr;

	for (auto t : m_vcTabs) {
		delete t;
	}
	m_vcTabs.clear();
	for (auto v : m_vcViews) {
		delete v;
	}
	m_vcViews.clear();
}

// IOverlayIntl implementation
//
unsigned int Overlay::AddTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, bool bNoScroll, int nInvokeValue) {
	unsigned int id = pTL->AddTouchArea(xOrigin, yOrigin, fWidth, fHeight, pTarget, unAction, nInvokeValue);
	pTL->SetScrollingBehaviour(id, bNoScroll, bNoScroll);
	return id;
}

unsigned int Overlay::AddVerticallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue) {
	unsigned int id = pTL->AddTouchArea(xOrigin, yOrigin, fWidth, fHeight, pTarget, unAction, nInvokeValue);
	pTL->SetScrollingBehaviour(id, true, false);
	return id;
}

unsigned int Overlay::AddHorizontallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue) {
	unsigned int id = pTL->AddTouchArea(xOrigin, yOrigin, fWidth, fHeight, pTarget, unAction, nInvokeValue);
	pTL->SetScrollingBehaviour(id, false, true);
	return id;
}

void Overlay::SetTouchAreaToNeedContinuousUpdates(ITouchLayer* pTL, unsigned int unId, bool bContinuousUpdates) {
	pTL->SetTouchTargetNeedsContinuousUpdates(unId, bContinuousUpdates);
}


bool Overlay::ApplyTabPressed() { 
	if (TabDataAltered()) {
		return SaveTabData();
	}
	return true;
}

bool Overlay::ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTarget, const IntRect& rcTargetArea) {
	return s_pEngineInput->ActivateKeyboardInput(eCharSet, pTarget, rcTargetArea);
}

int Overlay::NextElementId() {
	return s_nNextElementId++;
}
//
// IOverlayIntl implementation

// IOverlay Implementation
//
void Overlay::Render(float fDeltaTime) {
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (m_bRenderBackdrop) {
		RenderBackdrop();
	}
	if (m_vcViews.size() > 0) {
		AnimateViews(fDeltaTime);
		RenderViews();
	}
	if (m_vcTabs.size() > 0) {
		AnimateTabs(fDeltaTime);
		RenderTabs();
	}
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if (m_pAlertOverlay) {
		m_pAlertOverlay->Render(fDeltaTime);
	}
}

void Overlay::Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float fWidth, float fHeight) {
	m_eLabelAligment = eAlignment;

	m_sUnsavedDataText = sUnsavedDataText;

	m_bRenderBackdrop = true;
	m_bKeepFullScreen = false;

	m_xTabOffset = 0.0f;
	
	m_bKeepCentred = true;
	CentreInScreen(fWidth, fHeight);
	int nOutsideOverlayAction = m_pEnvironment->GetOutsideOverlayAction();
	m_pTouchLayer = new TouchLayer(static_cast<int>(m_xOrigin), static_cast<int>(m_yOrigin), static_cast<int>(fWidth), static_cast<int>(fHeight), this, nOutsideOverlayAction);
	m_bTabsNotViews = true;

	InitialiseInternalActions();
}

void Overlay::Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float xOrigin, float yOrigin, float fWidth, float fHeight) {
	m_eLabelAligment = eAlignment;

	m_sUnsavedDataText = sUnsavedDataText;

	m_bRenderBackdrop = true;
	m_bKeepFullScreen = false;

	m_bKeepCentred = false;
	m_xOrigin = static_cast<float>(xOrigin);
	m_yOrigin = static_cast<float>(yOrigin);
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	int nOutsideOverlayAction = m_pEnvironment->GetOutsideOverlayAction();
	m_pTouchLayer = new TouchLayer(static_cast<int>(m_xOrigin), static_cast<int>(m_yOrigin), static_cast<int>(fWidth), static_cast<int>(fHeight), this, nOutsideOverlayAction);
	m_bTabsNotViews = true;

	InitialiseInternalActions();
}

void Overlay::InitialiseFullScreen(OverlayLabelAlignment eAlignment, bool bRenderBackdrop, float fWidth, float fHeight) {
	m_eLabelAligment = eAlignment;
	m_bRenderBackdrop = bRenderBackdrop;
	m_bKeepFullScreen = true;
	m_xOrigin = 0.0f;
	m_yOrigin = 0.0f;
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_bTabsNotViews = false;

	InitialiseInternalActions();
}

void Overlay::WindowSizeChanged(int nWidth, int nHeight) {
	float xCurrentOrigin = m_xOrigin;
	float yCurrentOrigin = m_yOrigin;

	OverlayGraphics::InitialiseOverlayGraphics(nWidth, nHeight);
	if (m_bKeepCentred) {
		CentreInScreen(m_fWidth, m_fHeight);
		float dx = m_xOrigin - xCurrentOrigin;
		float dy = m_yOrigin - yCurrentOrigin;

		if (m_bTabsNotViews) {
			if (m_pTabBarTouchLayer != nullptr) {
				m_pTabBarTouchLayer->MoveLayer(dx, dy);
			}
			if (m_pTouchLayer != nullptr) {
				m_pTouchLayer->MoveLayer(dx, dy);
			}

			for (auto t : m_vcTabs) {
				t->Move(dx, dy);
			}
		}
	}
	if (!m_bTabsNotViews) {
		for (auto v : m_vcViews) {
			// TODO Implement.
			// v->WindowSizeChanged(nWidth, nHeight);
		}
	}
}

void Overlay::InitialiseTouchAreas() {
	if (m_bTabsNotViews) {
		InitialiseTouchAreasForTabs();
	}
	else {
		InitialiseTouchAreasForViews();
	}
}

bool Overlay::CanClose() const {
	if (m_bTabsNotViews) {
		bool bCanClose = true;
		for (const OverlayTab* t : m_vcTabs) {
			if (!t->CanClose()) {
				bCanClose = false;
			}
		}
		return bCanClose;
	}
	return true;
}

bool Overlay::SaveData() {
	if (!CanClose()) {
		return false;
	}
	if (m_bTabsNotViews) {
		for (auto t : m_vcTabs) {
			t->SaveTabData();
		}
	}
	return true;
}

void Overlay::ChangeDisplayState(bool bDisplayingNotHiding) {
	if (m_bTabsNotViews) {
		auto pTab = GetTabWithIndex(m_unTabDisplayed);
		if (bDisplayingNotHiding) {
			s_pEngineInput->ActivateTopWindowLayer(false);
			s_pEngineInput->NewWindowLayer();

			m_pTouchLayer->SetLayerActive(true);
			m_pTabBarTouchLayer->SetLayerActive(true);
			m_pTouchLayer->AddToInputEngine();
			m_pTabBarTouchLayer->AddToInputEngine();
			pTab->Displaying(bDisplayingNotHiding);
			ResetTabDisplayState();
		}
		else {
			m_pTouchLayer->RemoveFromInputEngine();
			m_pTabBarTouchLayer->RemoveFromInputEngine();
			m_pTouchLayer->SetLayerActive(false);
			m_pTabBarTouchLayer->SetLayerActive(false);
			// This has to be called before ActivateTopWindowLayer() and RemoveTopWindowLayer()
			pTab->Displaying(false);
			s_pEngineInput->RemoveTopWindowLayer(true);
			s_pEngineInput->ActivateTopWindowLayer(true);
		}
		
	}
	else {
		if (bDisplayingNotHiding) {
			s_pEngineInput->NewWindowLayer();
		}
		for (auto v : m_vcViews) {
			v->Displaying(bDisplayingNotHiding);
		}
		if (!bDisplayingNotHiding) {
			s_pEngineInput->RemoveTopWindowLayer(true);
			s_pEngineInput->ActivateTopWindowLayer(true);

		}
	}
}

ActionResult Overlay::Action(unsigned int unAction) {
	if (unAction == m_pEnvironment->GetNextTabAction()) {
		return DisplayNextTab();
	}
	else if (unAction == m_pEnvironment->GetPrevTabAction()) {
		return DisplayPreviousTab();
	}
	else {
		OverlayTab* pTabDisplayed = GetTabWithIndex(m_unTabDisplayed);
		return pTabDisplayed->Action(unAction);
	}
}

IOverlayTab* Overlay::AddTab(std::string sTabTitle) {
	if (!m_bTabsNotViews) {
		// TODO Set SDK Error condition
		return nullptr;
	}
	float fTabWidth = m_fWidth - OVERLAY_MARGIN * 2;
	float fTabHeight = m_fHeight - OVERLAY_MARGIN * 2;
	OverlayTab* pTab = new OverlayTab(sTabTitle, fTabWidth, fTabHeight, m_eLabelAligment, this);
	if (m_vcTabs.size() == 0) {
		pTab->SetSoleTab(true);
	}
	else if (m_vcTabs.size() == 1) {
		m_vcTabs[0]->SetSoleTab(false);
	}
	m_vcTabs.push_back(pTab);
	if (m_vcTabs.size() == 1) {
		pTab->Displaying(true);
	}
	return pTab;
}

IOverlayView* Overlay::AddView(float fXOrigin, float fYOrigin, float fWidth, float fHeight, bool bVerticalNotHoriz) {
	if (m_bTabsNotViews) {
		// TODO Set SDK Error condition
		return nullptr;
	}
	float fMargin = m_pEnvironment->GetViewMargin();

	OverlayView* pView = new OverlayView(this, fXOrigin + fMargin, fYOrigin + fMargin, fWidth - 2 * fMargin, fHeight - 2 * fMargin, bVerticalNotHoriz);
	m_vcViews.push_back(pView);

	return pView;
}

void Overlay::ShowDebugBackgroundOnControl(OverlayControlType eControlType, bool bShow) {
	int nTabcount = GetTabCount();
	for (int nIndex = 0; nIndex < nTabcount; ++nIndex) {
		auto pTab = GetTabWithIndex(nIndex);
		pTab->ShowDebugBackgroundOnControlType(eControlType, bShow);
	}
}

void Overlay::ShowDebugBackgroundOnControlInTab(unsigned int unTab, OverlayControlType eControlType, bool bShow) {
	if (!CheckTabExists(unTab)) {
		return;
	}
	OverlayTab* pTab = GetTabWithIndex(unTab);
	pTab->ShowDebugBackgroundOnControlType(eControlType, bShow);
}

void Overlay::DisplayAlert(std::string sText, std::string sButtonText, std::string sCancelText) {
	m_pAlertOverlay = new Overlay(m_pEnvironment);
	m_pAlertOverlay->Initialise( eOLANone, "", 400, 300);
}
//
// IOverlay Implementation

// ITouchTarget implementation
//
bool Overlay::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	if (unAction == m_pEnvironment->GetPrevTabAction()) {
		return DisplayPreviousTab();
	}
	else if (unAction == m_pEnvironment->GetNextTabAction()) {
		return DisplayNextTab();
	}
	else if (unAction == m_pEnvironment->GetChooseTabAction()) {
		DisplayTab(nValue);
		return true;
	}
	return false;
}

bool Overlay::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) {
	if (unAction == m_pEnvironment->GetChooseTabAction()) {
		HighlightTabTitleAsPressed(nValue, bPressed);
	}
	return true;
}
//
// ITouchTarget implementation

// IElementRendering implementation
//
void Overlay::SetButtonRenderFunc(fnRenderButton pFunc) {
	OverlayGraphics::SetButtonRenderFunc(pFunc);
}

void Overlay::SetSelectorRenderFunc(fnRenderSelector pFunc) {
	OverlayGraphics::SetSelectorRenderFunc(pFunc);
}

//
// IElementRendering implementation

void Overlay::CentreInScreen(float fWidth, float fHeight) {
	if (fWidth != 0.0f) {
		m_fWidth = fWidth;
	}
	if (fHeight != 0) {
		m_fHeight = fHeight;
	}

	m_xOrigin = (s_nScreenWidth - m_fWidth) / 2;
	m_yOrigin = (s_nScreenHeight - m_fHeight) / 2;
}

void Overlay::AnimateTabs(float fDeltaTime) {
	auto pTab = GetTabWithIndex(m_unTabDisplayed);
	pTab->Animate(fDeltaTime);
}

void Overlay::RenderTabs() {
	float fxTabOffset = m_xTabOffset;
	float xMargin = OVERLAY_MARGIN;
	float yMargin = OVERLAY_MARGIN;

	float xOrigin = m_xOrigin + xMargin;
	float yOrigin = m_yOrigin + yMargin;
	float width = m_fWidth - 2 * xMargin;
	float height = m_fHeight - 2 * yMargin;

	// Extremely unlikely to overflow
	int nTabCount = static_cast<int>(m_vcTabs.size());
	for(int nTabIndex = 0; nTabIndex < nTabCount; ++nTabIndex) {
		auto t = m_vcTabs[nTabIndex];
		bool bTabDisplayed = nTabIndex == m_unTabDisplayed;
		t->Render(bTabDisplayed, fxTabOffset, xOrigin, yOrigin, width, height);
		float titleWidth = t->GetTabTitleWidth();
		fxTabOffset += titleWidth + TAB_SPACING;
		if (fxTabOffset >= width) {
			nTabIndex++;
			for (; nTabIndex < nTabCount; ++nTabIndex) {
				t = m_vcTabs[nTabIndex];
				// Ensure these tabs title xOrigins are set to -1, indicating that they need to be shifted over
				//  Not doing this would shift them over relative to their last xOrigin, which unless this is done would be inaccurate.
				t->NotRendering();
			}
			break;
		}
	}
}

void Overlay::AnimateViews(float fDeltaTime) {
	for (OverlayView* pView : m_vcViews) {
		pView->Animate(fDeltaTime);
	}
}

void Overlay::RenderViews() {
	float fMargin = m_pEnvironment->GetViewMargin();

	for (OverlayView* pView : m_vcViews) {
		pView->Render(m_xOrigin+fMargin, m_yOrigin+fMargin, m_fWidth, m_fHeight);
	}
}

void Overlay::InitialiseTouchAreasForTabs() {
	float tabButtonWidth = static_cast<float>(OverlayTab::GetOverlayTabButtonWidth());
	float xLeftButtonOrigin = m_xOrigin + OVERLAY_MARGIN;
	float xRightButtonOrigin = m_xOrigin + m_fWidth - tabButtonWidth - OVERLAY_MARGIN;
	float yButtonOrigin = m_yOrigin + m_fHeight - tabButtonWidth - OVERLAY_MARGIN;
	if (m_vcTabs.size() > 0 && m_pTouchLayer != nullptr) {
		AddTouchArea(m_pTouchLayer, xLeftButtonOrigin, yButtonOrigin, tabButtonWidth, tabButtonWidth, this, m_pEnvironment->GetPrevTabAction(), true, 0);
		AddTouchArea(m_pTouchLayer, xRightButtonOrigin, yButtonOrigin, tabButtonWidth, tabButtonWidth, this, m_pEnvironment->GetNextTabAction(), true, 0);
	}
	float xTabOffset = m_xTabOffset;
	int xMargin = static_cast<int>(m_pEnvironment->GetTabMargin());
	int yMargin = xMargin;

	float xTabOrigin = m_xOrigin + xMargin;
	float yTabOrigin = m_yOrigin + yMargin;
	float fTabWidth = m_fWidth - 2 * xMargin;
	float titleHeight = 0.0f;
	if (m_vcTabs.size() > 0) {
		// All title heights should be the same (otherwise it would look a mess)
		titleHeight = static_cast<float>(m_vcTabs[0]->GetTabTitleHeight());
		yButtonOrigin = m_yOrigin + m_fHeight - titleHeight - OVERLAY_MARGIN;
	}

	if (m_pTabBarTouchLayer == nullptr) {
		m_pTabBarTouchLayer = new TouchLayer(xLeftButtonOrigin + tabButtonWidth, yButtonOrigin, m_fWidth - 2 * xMargin - 2 * tabButtonWidth, titleHeight, this, 0);
	}
	// Extremely unlikely to overflow
	int nTabCount = static_cast<int>(m_vcTabs.size());
	for (int nTabIndex = 0; nTabIndex < nTabCount; ++nTabIndex) {
		auto t = m_vcTabs[nTabIndex];

		float titleWidth = static_cast<float>(t->GetTabTitleWidth());
		float xTitleOrigin = m_xOrigin + xMargin + xTabOffset + OverlayTab::GetOverlayTabButtonWidth();
		float yTitleOrigin = m_yOrigin + m_fHeight - yMargin - titleHeight;

		// Add touch area for Tab headers.
		// *this* (the overlay object) is the target of the touch area, not the tab
		unsigned int id = AddHorizontallyScrollingTouchArea(m_pTabBarTouchLayer, xTitleOrigin, yTitleOrigin, titleWidth, titleHeight, this, m_pEnvironment->GetChooseTabAction(), nTabIndex);
		SetTouchAreaToNeedContinuousUpdates(m_pTabBarTouchLayer, id, true);

		xTabOffset += titleWidth + TAB_SPACING;
		// Add layer for tab (not including title area)
		float fTabHeight = m_fHeight - yMargin * 2 - titleHeight - OverlayTab::GetOverlayTabButtonConnectingHeight();

		TouchLayer* pTabLayer = new TouchLayer(xTabOrigin, yTabOrigin, fTabWidth, fTabHeight, nullptr, 0);
		// Tab gains control of pTabLayer lifetime
		t->InitialiseTouchAreas(pTabLayer, xTabOrigin, yTabOrigin);
	}
}

void Overlay::InitialiseTouchAreasForViews() {
	int viewCount = static_cast<int>(m_vcViews.size());
	float xMargin = m_pEnvironment->GetViewMargin();
	float yMargin = xMargin;
	for (int nViewIndex = 0; nViewIndex < viewCount; ++nViewIndex) {
		auto v = m_vcViews[nViewIndex];
		float xViewOrigin = m_xOrigin + xMargin;
		float yViewOrigin = m_yOrigin + yMargin;
		float fViewWidth = m_fWidth - 2 * xMargin;
		float fViewHeight = m_fHeight - 2 * yMargin;
		TouchLayer* pViewLayer = new TouchLayer(xViewOrigin, yViewOrigin, fViewWidth, fViewHeight, v, 0);
		// Tab gains control of pTabLayer lifetime
		v->InitialiseTouchAreas(pViewLayer, xViewOrigin, yViewOrigin);
	}
}

void Overlay::RenderBackdrop() {
	const IOverlayEnv* pPublicEnv = m_pEnvironment->GetConstPublicInterface();

	RenderTranslucentOverlay(m_xOrigin, m_yOrigin, m_fWidth, m_fHeight, pPublicEnv->GetOverlayBackgroundRGBA());
}

void Overlay::ResetTabDisplayState() {
	m_unTabDisplayed = 0;
	m_xTabOffset = 0.0f;

	for (auto t : m_vcTabs) {
		t->ResetDisplayState();
	}
}

bool Overlay::TabDataAltered() {
	OverlayTab* pTabDisplayed = GetTabWithIndex(m_unTabDisplayed);
	return pTabDisplayed->TabDataAltered();
}

void Overlay::RevertTabData() { 
	OverlayTab* pTabDisplayed = GetTabWithIndex(m_unTabDisplayed);
	pTabDisplayed->RevertTabData();
}

bool Overlay::SaveTabData() { 
	OverlayTab* pTabDisplayed = GetTabWithIndex(m_unTabDisplayed);
	return pTabDisplayed->SaveTabData();
}

bool Overlay::CheckTabExists(unsigned int unTabIndex) {
	return unTabIndex < m_vcTabs.size();
}

OverlayTab* Overlay::GetTabWithIndex(unsigned int unTabIndex) {
	return m_vcTabs[unTabIndex];
}

const OverlayTab* Overlay::GetTabWithIndex(unsigned int unTabIndex) const {
	return m_vcTabs[unTabIndex];
}

void Overlay::HighlightTabTitleAsPressed(int nTabIndex, bool bPressed) {
	OverlayTab* pTab = GetTabWithIndex(nTabIndex);
	pTab->HighlightAsPressed(bPressed);
}

void Overlay::DisplayTab(int nTabIndex) {
	if (m_unTabDisplayed == nTabIndex) {
		return;
	}
	if (TabDataAltered()) {
		DisplayAlert(m_sUnsavedDataText, "Okay","");
		return;
	}
	OverlayTab* pPrevTab = m_vcTabs[m_unTabDisplayed];
	pPrevTab->Displaying(false);
	m_unTabDisplayed = nTabIndex;
	OverlayTab* pTab = GetTabWithIndex(m_unTabDisplayed);
	pTab->Displaying(true);


	float xActualOverlayOrigin = m_xOrigin + OVERLAY_MARGIN + OverlayTab::GetOverlayTabButtonWidth();
	float xTabTitleOrigin = pTab->GetXTitleOriginFromLastRender();
	if (xTabTitleOrigin < xActualOverlayOrigin) {
		float xDelta = xActualOverlayOrigin - xTabTitleOrigin;

		m_xTabOffset += xDelta;
		m_pTabBarTouchLayer->SetXScrollDelta(m_xTabOffset);
	}
	else {
		float tabTitleWidth = pTab->GetTabTitleWidth();
		// The whole width of the overlay isn't used for the tabs
		float usableOverlayWidth = m_fWidth - 2 * OVERLAY_MARGIN - 2 * OverlayTab::GetOverlayTabButtonWidth();
		float xActualOverlayOrigin = m_xOrigin + OVERLAY_MARGIN + OverlayTab::GetOverlayTabButtonWidth();

		// Where will the title finish (on x-axis) - is it past the usable area of the overly?
		float xTabTitleRight = xTabTitleOrigin + tabTitleWidth;
		if (xTabTitleRight > xActualOverlayOrigin + usableOverlayWidth) {
			// Tab title is off the right of the overlay - shift it over
			float xOverlayRight = xActualOverlayOrigin + usableOverlayWidth;
			float xDelta = xTabTitleRight - xOverlayRight;
			m_xTabOffset -= xDelta;
			m_pTabBarTouchLayer->SetXScrollDelta(m_xTabOffset);
		}
	}
}

bool Overlay::DisplayNextTab() {
	if (m_unTabDisplayed < m_vcTabs.size() - 1) {
		if (TabDataAltered()) {
			DisplayAlert(m_sUnsavedDataText, "Okay", "");
			return false;
		}
		OverlayTab* pPrevTab = m_vcTabs[m_unTabDisplayed];
		pPrevTab->Displaying(false);
		++m_unTabDisplayed;
		OverlayTab* pTab = GetTabWithIndex(m_unTabDisplayed);
		pTab->Displaying(true);

		// Returns -1 if this tab title has not yet been rendered (as it was off screen)
		float xTabTitleOrigin = pTab->GetXTitleOriginFromLastRender();

		if (xTabTitleOrigin == -1) {
			// Tab title was not yet displayed, shift tabs left by previous tab's title width
			OverlayTab* pPreviousTab = GetTabWithIndex(m_unTabDisplayed - 1);
			float nShiftLeftBy = pPreviousTab->GetTabTitleWidth() + OVERLAY_MARGIN;
			m_xTabOffset -= nShiftLeftBy;
			m_pTabBarTouchLayer->SetXScrollDelta(m_xTabOffset);
			std::cout << "Next tab(A), delta " << (-nShiftLeftBy) << std::endl;
		}
		else {
			// Determine if the tab title being moved to, is off the right of the overlay
			float tabTitleWidth = pTab->GetTabTitleWidth();
			// The whole width of the overlay isn't used for the tabs
			float usableOverlayWidth = m_fWidth - 2 * OVERLAY_MARGIN - 2 * OverlayTab::GetOverlayTabButtonWidth();
			float xActualOverlayOrigin = m_xOrigin + OVERLAY_MARGIN + OverlayTab::GetOverlayTabButtonWidth();

			// Where will the title finish (on x-axis) - is it past the usable area of the overly?
			float xTabTitleRight = xTabTitleOrigin + tabTitleWidth;
			if (xTabTitleRight > xActualOverlayOrigin + usableOverlayWidth) {
				// Tab title is off the right of the overlay - shift it over
				float xOverlayRight = xActualOverlayOrigin + usableOverlayWidth;
				float xDelta = xTabTitleRight - xOverlayRight;
				std::cout << "Next tab(B), delta " << (-xDelta) << std::endl;
				m_xTabOffset -= xDelta;
				m_pTabBarTouchLayer->SetXScrollDelta(m_xTabOffset);
			}
		}
		return true;
	}
	return false;
}

bool Overlay::DisplayPreviousTab() {
	if (m_unTabDisplayed > 0) {
		if (TabDataAltered()) {
			DisplayAlert(m_sUnsavedDataText, "Okay", "");
			return false;
		}
		OverlayTab* pPrevTab = m_vcTabs[m_unTabDisplayed];
		pPrevTab->Displaying(false);

		--m_unTabDisplayed;
		OverlayTab* pTab = GetTabWithIndex(m_unTabDisplayed);
		pTab->Displaying(true);

		int xActualOverlayOrigin = static_cast<int>(m_xOrigin + OVERLAY_MARGIN + OverlayTab::GetOverlayTabButtonWidth());
		float xTabTitleOrigin = pTab->GetXTitleOriginFromLastRender();
		if (xTabTitleOrigin < xActualOverlayOrigin) {
			float xDelta = xActualOverlayOrigin - xTabTitleOrigin;

			m_xTabOffset += xDelta;
			m_pTabBarTouchLayer->SetXScrollDelta(m_xTabOffset);
		}
		return true;
	}
	return false;
}

void Overlay::InitialiseInternalActions() {
	int nActionStart = m_pEnvironment->GetInternalActionStart();
	int nInternalDown = nActionStart + 1;
	int nInternalUp = nActionStart + 2;
	s_pEngineInput->RegisterInternalTouchAction(nInternalDown, eDAT_EventAutoRepeatTwoLevels);
	s_pEngineInput->RegisterInternalTouchAction(nInternalUp, eDAT_EventAutoRepeatTwoLevels);
	m_pEnvironment->SetInternalMoveDownAction(nInternalDown);
	m_pEnvironment->SetInternalMoveUpAction(nInternalUp);
}

// Static class methods implementation
//
void Overlay::SetEngineInput(IEngineInputIntl* pEngineInput) {
	s_pEngineInput = pEngineInput;
}
//
// Static class methods implementation
