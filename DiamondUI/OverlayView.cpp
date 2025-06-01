#include "OverlayView.h"
#include "IOverlayIntl.h"
#include "IOverlayEnvIntl.h"
#include "IOverlayElement.h"
#include "ScrollbarElement.h"
#include "TextElement.h"
#include "TouchLayer.h"

// TODO Add this to the environment
#define SCROLLBAR_INVOKE_VALUE 0x10000

OverlayView::OverlayView(IOverlayIntl* pOverlay, float fXOrigin, float fYOrigin, float fWidth, float fHeight, bool bVerticalNotHoriz) :
	m_elements(this) {
	m_pOverlay = pOverlay;
	m_fXOrigin = fXOrigin;
	m_fYOrigin = fYOrigin;
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_bVerticalNotHoriz = bVerticalNotHoriz;

	m_pEnvironment = m_pOverlay->GetInternalOverlayEnvironment();

	m_fTitleBackgroundHeight = 0.0f;

	m_pScrollbar = nullptr;
	m_bDisplayScrollbar = false;
	m_fContentHeight = 0.0f;
	m_yScrollOffset = 0.0f;
}

OverlayView::~OverlayView() {
	for (auto e : m_vcElements) {
		delete e;
	}
	m_vcElements.clear();
	if (m_pScrollbar != nullptr) {
		delete m_pScrollbar;
		m_pScrollbar = nullptr;
	}
}

// IOverlayView implementation
//
int OverlayView::AddTextLabel(std::string sLabel, float fWidth /*=0.0f*/) {
	int elementId = m_pOverlay->NextElementId();
	TextElement* pElement = new TextElement(m_pOverlay, elementId, sLabel, true);
	if (fWidth > 0.1f) {
		pElement->SetPreferredControlWidth(fWidth - 2 * m_pEnvironment->GetControlHorizMargin(false));
	}
	m_vcElements.push_back(pElement);
	m_mpIdToElement[elementId] = pElement;
	m_elements.AddElement(pElement);

	if (m_bVerticalNotHoriz) {
		float width = GetDisplayableViewWidth();
		pElement->InitialiseForWidth(width - 2 * m_pEnvironment->GetControlHorizMargin(false));

		if (m_bDisplayScrollbar) {
			UpdateScrollableContentHeight();
		}
		else {
			ConsiderDisplayingScrollbars();
		}
	}
	else {
		pElement->InitialiseForWidth(fWidth - 2 * m_pEnvironment->GetControlHorizMargin(false));
	}
	return elementId;

}

int OverlayView::AddTextField(std::string sText, float fWidth /*=0.0f*/) {
	// fWidth is the width of the control and includes in control margins

	int elementId = m_pOverlay->NextElementId();
	TextElement* pElement = new TextElement(m_pOverlay, elementId, sText, false);
	if (fWidth > 0.1f) {
		// Preferred control width does not include control margins
		pElement->SetPreferredControlWidth(fWidth - 2 * m_pEnvironment->GetControlHorizMargin(false));
	}
	m_vcElements.push_back(pElement);
	m_elements.AddElement(pElement);
	m_mpIdToElement[elementId] = pElement;

	if (m_bVerticalNotHoriz) {
		float fMarginInsideControl = m_pEnvironment->GetControlHorizMargin(false);
		float fWidthToUse = GetDisplayableViewWidth() - 2 * fMarginInsideControl;
		fWidth -= 2 * fMarginInsideControl;
		if (fWidth < fWidthToUse && fWidth>0.1f) {
			fWidthToUse = fWidth;
		}
		//pElement->InitialiseForWidth(fWidthToUse);

		//if (m_bDisplayScrollbar) {
		//	UpdateScrollableContentHeight();
		//}
		//else {
		//	ConsiderDisplayingScrollbars();
		//}
	}
	else {
		pElement->InitialiseForWidth(fWidth - 2 * m_pEnvironment->GetControlHorizMargin(false));
	}
	return elementId;
}

//
// IOverlayView implementation

void OverlayView::Animate(float fDeltaTime) {
	m_elements.Animate(fDeltaTime);
}

void OverlayView::Render(float fXOrigin, float fYOrigin, float fWidth, float fHeight) {
	m_elements.Render(m_fXOrigin + fXOrigin, m_fYOrigin + fYOrigin, m_fWidth, m_fHeight - m_fTitleBackgroundHeight);
	return;
	// TODO Set Clipping areas
	// TODO Display title
	//float fHorizControlMargin = m_pEnvironment->GetControlHorizMargin(false);
	//float fVertControlMargin = m_pEnvironment->GetControlVerticalMargin(false);

	//float xOffset = fXOrigin + m_fXOrigin;
	//float yOffset = fYOrigin + m_fYOrigin + m_fHeight - m_fTitleBackgroundHeight + m_yScrollOffset;
	//float fSpacing;
	//if (m_bVerticalNotHoriz) {
	//	fSpacing = m_pEnvironment->GetVerticalInterElementSpacing(false);
	//}
	//else {
	//	fSpacing = m_pEnvironment->GetHorizInterElementSpacing();
	//}

	//for (IOverlayElement* pElement : m_vcElements) {
	//	// ->GetPreferredControlWidth() does not include the control margins
	//	float fWidth = pElement->GetPreferredControlWidth() + 2 * fHorizControlMargin;
	//	float fHeight = pElement->GetHeight() + 2 * fVertControlMargin;
	//	yOffset -= fHeight;
	//	RenderTranslucentOverlay(xOffset, yOffset, fWidth, fHeight, m_pEnvironment->GetOverlayBackgroundRGBA());
	//	pElement->Render(xOffset + fHorizControlMargin, yOffset + fVertControlMargin, fWidth - 2 * fHorizControlMargin, fHeight - 2 * fVertControlMargin);
	//	if (m_bVerticalNotHoriz) {
	//		yOffset -= fSpacing;
	//	}
	//	else {
	//		xOffset += fWidth + fSpacing;
	//		yOffset += fHeight;
	//	}
	//}

	//if (m_bDisplayScrollbar) {
	//	float viewWidth = m_fWidth - ScrollbarElement::GetWidth();;
	//	float viewHeight = GetDisplayableViewHeight();

	//	m_pScrollbar->Render(fXOrigin + m_fXOrigin + viewWidth, fYOrigin, ScrollbarElement::GetWidth(), viewHeight);
	//}
}

// ITouchTarget implementation
//
bool OverlayView::Invoke(unsigned int unAction, int nValue, float fDeltaTime) { 
	if (nValue == SCROLLBAR_INVOKE_VALUE) {
		return Action(unAction).m_bSuccess;
	}
	return false;
}

bool OverlayView::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) { 
	return false;
}
//
// ITouchTarget implementation



// ScrollTarget implementation
//
void OverlayView::SetVerticalScrollOffset(float fYScrollOffset) {
	m_yScrollOffset = fYScrollOffset;
	/*if (m_pTouchLayer != nullptr) {
		m_pTouchLayer->SetYScrollDelta(m_yScrollOffset);
	}*/

}
//
// ScrollTarget implementation

float OverlayView::GetDisplayableViewWidth() {
	float fMargin = m_pEnvironment->GetViewMargin();
	float fWidth = m_fWidth - 2 * fMargin;
	if (m_elements.DisplayingScrollbar()) {
		fWidth -= ScrollbarElement::GetWidth();
	}
	return fWidth;
}

float OverlayView::GetDisplayableViewHeight() {
	float fMargin = m_pEnvironment->GetViewMargin();
	float fHeight = m_fHeight - m_fTitleBackgroundHeight - fMargin * 2;
	return fHeight;

}

void OverlayView::ConsiderDisplayingScrollbars() {
	if (m_bDisplayScrollbar) {
		return;
	}
	m_fContentHeight = CalculateTotalHeight();

	if (m_fContentHeight > GetDisplayableViewHeight()) {
		if (m_pScrollbar == nullptr) {
			int nElementId = m_pOverlay->NextElementId();
			m_pScrollbar = new ScrollbarElement(m_pOverlay, this, nElementId, GetDisplayableViewHeight(), static_cast<int>(m_fContentHeight), true);
		}
		else {
			m_pScrollbar->SetTotalContentHeight(static_cast<int>(m_fContentHeight));
		}
		m_bDisplayScrollbar = true;

		float width = GetDisplayableViewWidth();
		for (auto e : m_vcElements) {
			e->InitialiseForWidth(width);
		}
	}
}

void OverlayView::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin) {
	// This object gains control of TouchLayer's lifetime
	m_elements.InitialiseTouchAreas(pTouchLayer, m_fXOrigin + xOrigin, m_fYOrigin + yOrigin, m_fWidth, m_fHeight);
}

void OverlayView::Displaying(bool bDisplaying) {
	m_elements.Displaying(bDisplaying);
}

void OverlayView::UpdateScrollableContentHeight() {
	m_fContentHeight = CalculateTotalHeight();
	if (m_pScrollbar != nullptr) {
		m_pScrollbar->SetTotalContentHeight(static_cast<int>(m_fContentHeight));
	}
}

float OverlayView::CalculateTotalHeight() const {
	float cumulativeHeight = 0.0f;
	float fInterElementSpacing = m_pEnvironment->GetVerticalInterElementSpacing(false);

	for (auto e : m_vcElements) {
		cumulativeHeight += e->GetHeight();
		cumulativeHeight += fInterElementSpacing;
	}
	cumulativeHeight -= fInterElementSpacing;
	return cumulativeHeight;
}

ActionResult OverlayView::Action(unsigned int unAction) {
	// Action comes from the controller, whereas an Invoke comes 'directly' from the the touch area
	if (unAction == m_pEnvironment->GetMoveUpAction() || unAction == m_pEnvironment->GetInternalActionStart() + 2) {
		//return MoveUp();
		int a = 1;
		a++;
	}
	else if (unAction == m_pEnvironment->GetMoveDownAction() || unAction == m_pEnvironment->GetInternalActionStart() + 1) {
		//		return MoveDown();
		int a = 1;
		a++;
	}
	//else if (m_pHighlightedElement != nullptr) {
		//EnsureElementFullyDisplayed(m_pHighlightedElement);
		//return m_pHighlightedElement->Action(unAction);
	//}
	return ActionResult(false);
}
