#include "ScrollbarElement.h"
#include "IOverlayEnvIntl.h"
#include "TouchLayer.h"
#include "OverlayDefsInt.h"

#define BAR_WIDTH 10

#define SCROLLBAR_BUTTON_WIDTH 30
#define SCROLLBAR_BUTTON_HEIGHT 50
#define BAR_Y_MARGIN 5

ScrollbarElement::ScrollbarElement(IOverlayIntl* pOverlay, ScrollTarget* pTarget, int nElementId, float fHeight, int nTotalContentHeight, bool bForViewNotTab) {
	m_pOverlayWithin = pOverlay;
	m_pScrollTarget = pTarget;
	m_nElementId = nElementId;
	m_fHeight = fHeight;
	m_nTotalContentHeight = nTotalContentHeight;
	m_bForViewNotTab = bForViewNotTab;
	m_pOverlayEnv = m_pOverlayWithin->GetInternalOverlayEnvironment();
	// The same because the content is sat next to the scrollbar
	m_nDisplayableContentHeight = static_cast<int>(m_fHeight); 
	m_fYOffset = 0.0f;
	m_yLastRenderedOrigin = 0;
	m_bRenderedAtLeastOnce = false;

	m_bShowDebugBackground = false;
	CalcBarOffset();
	m_bScrolling = false;
	m_yScrollStart = 0.0f;
	m_nScrollUpAction = m_nScrollDownAction = 0;
}

ScrollbarElement::~ScrollbarElement() {
	// Nothing to free up
}

// IOverlayElement Implementation
//
void ScrollbarElement::Deinitialise() {
	FreeSpace();
}

void ScrollbarElement::FreeSpace() {
}

void ScrollbarElement::InitialiseForWidth(float fWidth) {
}

float ScrollbarElement::GetHeight() const {
	return m_fHeight;
}

float ScrollbarElement::GetPreferredControlWidth() const {
	return m_pOverlayEnv->GetPreferredControlWidth(); 
}

void ScrollbarElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	rgba rgbaVector = pPublicEnv->GetVectorColour(false);
	rgba rgbaScrollBar = pPublicEnv->GetScrollbarColour();

	m_yLastRenderedOrigin = yOrigin;
	m_bRenderedAtLeastOnce = true;
	RenderTranslucentOverlay(xOrigin + m_xBarOffset, static_cast<float>(yOrigin + m_dYBarOffset), m_fBarWidth, static_cast<float>(m_dBarHeight), rgbaScrollBar);

	float xGlyphOrigin = xOrigin + (fWidth - SCROLLBAR_BUTTON_WIDTH) / 2;

	//	RenderGlypth(xGlyphOrigin, yOrigin, SCROLLBAR_BUTTON_WIDTH, SCROLLBAR_BUTTON_HEIGHT, m_pOverlayEnv->GetMoveDownAction());
	//	RenderGlypth(xGlyphOrigin, yOrigin + nHeight - SCROLLBAR_BUTTON_HEIGHT, SCROLLBAR_BUTTON_WIDTH, SCROLLBAR_BUTTON_HEIGHT, m_pOverlayEnv->GetMoveUpAction());

	RenderUpArrow(xGlyphOrigin, yOrigin + fHeight - SCROLLBAR_BUTTON_HEIGHT, SCROLLBAR_BUTTON_WIDTH, SCROLLBAR_BUTTON_HEIGHT, rgbaVector);
	RenderDownArrow(xGlyphOrigin, yOrigin, SCROLLBAR_BUTTON_WIDTH, SCROLLBAR_BUTTON_HEIGHT, rgbaVector);
	//RenderVerticalScrollbar(pPublicEnv, xOrigin, yOrigin, fWidth, fHeight, );
}

void ScrollbarElement::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {
	if (m_bForViewNotTab) {
		m_nScrollDownAction = m_pOverlayEnv->GetInternalMoveDownAction();
		m_nScrollUpAction = m_pOverlayEnv->GetInternalMoveUpAction();
	}
	else {
		m_nScrollDownAction = m_pOverlayEnv->GetMoveDownAction();
		m_nScrollUpAction = m_pOverlayEnv->GetMoveUpAction();
	}
	unsigned int id = pTouchLayer->AddTouchArea(xOrigin, yOrigin, GetWidth(), SCROLLBAR_BUTTON_HEIGHT, pTarget, m_nScrollDownAction, nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, false, false);
	id = pTouchLayer->AddTouchArea(xOrigin, yOrigin + m_fHeight - SCROLLBAR_BUTTON_HEIGHT, GetWidth(), SCROLLBAR_BUTTON_HEIGHT, pTarget, m_nScrollUpAction, nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, false, false);

	// TODO choose an action and invoke value
	id = pTouchLayer->AddTouchArea(xOrigin, yOrigin + SCROLLBAR_BUTTON_HEIGHT, GetWidth(), m_fHeight - 2 * SCROLLBAR_BUTTON_HEIGHT, this, m_pOverlayEnv->GetInternalActionStart(), nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, false, false);
	pTouchLayer->SetTouchTargetNeedsContinuousUpdates(id, true);
}

void ScrollbarElement::ShowDebugBackground(bool bShow) {
	m_bShowDebugBackground = bShow;
}

void ScrollbarElement::ResetDisplayState() {
	m_bScrolling = false;
}
//
// IOverlayElement Implementation

// ITouchTarget implementation
//
bool ScrollbarElement::Invoke(unsigned int unAction, int nValue, float fDeltaTime) { 
	return false;
}

bool ScrollbarElement::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) { 
	if (bPressed && !m_bScrolling) {
		if (CoordsOnBar(x,y)) {
			m_bScrolling = true;
			m_yScrollStart = static_cast<float>(y);
		}
	}
	else if (m_bScrolling && !bPressed) {
		m_bScrolling = false;
	}
	else if (m_bScrolling) {
		int diff = static_cast<int>(y - m_yScrollStart);
		if (m_dYBarOffset + m_dBarHeight + diff > m_fHeight - SCROLLBAR_BUTTON_HEIGHT) {
			m_dYBarOffset = m_fHeight - SCROLLBAR_BUTTON_HEIGHT - m_dBarHeight;
		}
		else if (m_dYBarOffset + diff < SCROLLBAR_BUTTON_HEIGHT) {
			m_dYBarOffset = SCROLLBAR_BUTTON_HEIGHT;
		}
		else {
			m_dYBarOffset += diff;
			m_yScrollStart = static_cast<float>(y);
		}
		CalcScrollOffsetFromBarOffset();
		m_pScrollTarget->SetVerticalScrollOffset(-m_fYOffset);
	}
	return true;
}
//
// ITouchTarget implementation

void ScrollbarElement::SetTotalContentHeight(int nTotalContentHeight) {
	m_nTotalContentHeight = nTotalContentHeight;
	CalcBarOffset();
}

void ScrollbarElement::SetHeight(float fHeight) {
	m_fHeight = fHeight;
	CalcBarOffset();
}

void ScrollbarElement::SetYOffset(float nYOffset) {
	m_fYOffset = nYOffset;
	CalcBarOffset();
}

void ScrollbarElement::AlterYOffset(int nDelta) {
	m_fYOffset += nDelta;
	CalcBarOffset();
}

void ScrollbarElement::CalcBarOffset() {
	double dUsuableScrollHeight = static_cast<double>(m_fHeight - SCROLLBAR_BUTTON_HEIGHT * 2);
	m_dYBarOffset = SCROLLBAR_BUTTON_HEIGHT;
	m_fBarWidth = BAR_WIDTH;
	m_xBarOffset = (GetWidth() - m_fBarWidth) / 2;
	m_dBarHeight = dUsuableScrollHeight * static_cast<double>(m_fHeight) / static_cast<double>(m_nTotalContentHeight);

	double yOffset = dUsuableScrollHeight * static_cast<double>(m_fYOffset) / static_cast<double>(m_nTotalContentHeight);

	m_dYBarOffset += dUsuableScrollHeight;
	m_dYBarOffset -= m_dBarHeight;
	double scrollOffset = static_cast<double>(yOffset);
	m_dYBarOffset += scrollOffset;
}

void ScrollbarElement::CalcScrollOffsetFromBarOffset() {
	double dUsuableScrollHeight = static_cast<double>(m_fHeight - SCROLLBAR_BUTTON_HEIGHT * 2);

    // m_dYBarOffset = SCROLLBAR_BUTTON_HEIGHT +dUsuableScrollHeight + (dUsuableScrollHeight * m_fYOffset / m_nTotalContentHeight) - m_dBarHeight;

	// (m_dYBarOffset + m_dBarHeight - SCROLLBAR_BUTTON_HEIGHT  - dUsuableScrollHeight) * m_nTotalContentHeight/dUsuableScrollHeight = m_fYOffset;

	m_fYOffset = static_cast<float>((m_dYBarOffset + m_dBarHeight - SCROLLBAR_BUTTON_HEIGHT - dUsuableScrollHeight)* m_nTotalContentHeight / dUsuableScrollHeight);
}

bool ScrollbarElement::CoordsOnBar(int x, int y) {
	y += SCROLLBAR_BUTTON_HEIGHT;
	return y >= m_dYBarOffset && y < m_dYBarOffset + m_dBarHeight;
}