#include "ButtonBarElement.h"
#include "ButtonElement.h"
#include "IOverlayEnvIntl.h"

#define INTERBUTTON_HORIZ_SPACING 10

std::map<unsigned int, ButtonBarElement* > ButtonBarElement::s_mpInvokeHandleToButtonBar;

ButtonBarElement::ButtonBarElement(IOverlayIntl* pOverlay, int nElementId, OverlayLabelAlignment eLabelAlignment)
	: ElementBase(pOverlay, nElementId, eLabelAlignment) {
	m_fCtrlWidth = 0.0f;
	m_xCtrlOffset = 0;
	m_nHighlightedElementIndex = -1;

	m_unInvokeHandle = RegisterButtonBar(this);
}

ButtonBarElement::~ButtonBarElement() {
	for (auto e : m_vcElements) {
		delete e;
	}
	m_vcElements.clear();
}


// IOverlayElement Implementation
//
void ButtonBarElement::Deinitialise() { 
	FreeSpace();
}

void ButtonBarElement::FreeSpace() { 
	ElementBase::FreeSpace();
	for (auto e : m_vcElements) {
		e->FreeSpace();
	}
}

void ButtonBarElement::InitialiseForWidth(float fWidth) { 
	InitialiseContainerForWidth(fWidth);
	int buttonCount = static_cast<int>(m_vcElements.size());

	float fButtonWidth = m_pOverlayEnv->GetPreferredBarWidthForControlType(eOCTButton, buttonCount, INTERBUTTON_HORIZ_SPACING);
	float x = static_cast<float>(m_xCtrlOffset);

	for (auto e : m_vcElements) {
		e->SetPreferredControlWidth(fButtonWidth);
		e->InitialiseForWidth(fButtonWidth);
	}
}

float ButtonBarElement::GetHeight() const { 
	float controlHeight = 20.0f;
	if (m_vcElements.size() != 0) {
		controlHeight = m_vcElements[0]->GetHeight();
	}
	return controlHeight;
}

void ButtonBarElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) { 
	ElementBase::Render(xOrigin, yOrigin, fWidth, fHeight);
	if (m_bShowDebugBackground) {
		const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
		RenderTranslucentOverlay(xOrigin, yOrigin, fWidth, fHeight, pPublicEnv->GetDebugBackgroundRGBA(true));
	}

	float x = xOrigin + fWidth;
	for (auto e : m_vcElements) {
		float fCtrlWidth = e->GetPreferredControlWidth();
		e->Render(x-fCtrlWidth, yOrigin, fCtrlWidth, fHeight);
		x -= fCtrlWidth;
		x -= INTERBUTTON_HORIZ_SPACING;
	}
}

void ButtonBarElement::Highlight(bool bHighlight) { 
	if (m_vcElements.size() == 0) {
		m_nHighlightedElementIndex = -1;
		return;
	}
	if (bHighlight) {
		if (m_nHighlightedElementIndex == -1) {
			// TODO Determine last non-disabled element
			m_nHighlightedElementIndex = static_cast<int>(m_vcElements.size() - 1);
		}
		m_vcElements[m_nHighlightedElementIndex]->Highlight(true);
	}
	else {
		if (m_nHighlightedElementIndex != -1) {
			m_vcElements[m_nHighlightedElementIndex]->Highlight(false);
			m_nHighlightedElementIndex = -1;
		}
	}
}

float ButtonBarElement::GetScrollIncrement() { 
	return GetHeight();
}

void ButtonBarElement::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) { 
	nInvokeValue = 0;
	float x = xOrigin + m_xCtrlOffset + m_fCtrlWidth;
	for (auto e : m_vcElements) {
		float fControlWidth = e->GetPreferredControlWidth();
		e->InitialiseTouchAreas(pTouchLayer, x - fControlWidth, yOrigin, this, nInvokeValue);
		x -= fControlWidth;
		x -= INTERBUTTON_HORIZ_SPACING;
		++nInvokeValue;
	}
}

ActionResult ButtonBarElement::Action(unsigned int unAction) {
	if (unAction == m_pOverlayEnv->GetMoveLeftAction()) {
		return MoveHighlightLeft();
	}
	else if (unAction == m_pOverlayEnv->GetMoveRightAction()) {
		return MoveHighlightRight();
	}
	else if (unAction == m_pOverlayEnv->GetOkAction()) {
		if (m_nHighlightedElementIndex >= 0 && m_nHighlightedElementIndex < m_vcElements.size()) {
			return m_vcElements[m_nHighlightedElementIndex]->Action(unAction);
		}
	}
	return false;
}

void ButtonBarElement::ResetDisplayState() {
	if (m_nHighlightedElementIndex != -1) {
		m_vcElements[m_nHighlightedElementIndex]->Highlight(false);
		m_nHighlightedElementIndex = -1;
	}
}
//
// IOverlayElement Implementation

// ITouchTarget Implementation
//
bool ButtonBarElement::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	if (nValue >= 0 && nValue < m_vcElements.size()) {
		auto e = m_vcElements[nValue];
		ButtonElement* pButton = dynamic_cast<ButtonElement*>(e);
		TabActionButtonType eButtonType = pButton->GetActionType();

		switch (eButtonType) {
		case eTABTOkay:
		case eTABTClose:
			// These two should be handled directly with 'unpause', as closing the UI has to go through the gamelogic 
			//  (validating state, saving state, change stean controller action set, closing UI), so should start
			//  processing from GameInput
			break;
		case eTABTApply:
			return m_pOverlayWithin->ApplyTabPressed();
		case eTABTCancel:
			// TODO Handle this via a digital action
			// This needs to be handled directly with a digital action enumeration value - so it is processed via game input

			//return m_pOverlayWithin->CancelTabPressed();
			break;
		}
	}
	return false;
}

bool ButtonBarElement::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) {
	if (nValue >= 0 && nValue < m_vcElements.size()) {
		auto e = m_vcElements[nValue];
		ButtonElement* pButton = dynamic_cast<ButtonElement*>(e);
		return pButton->TouchUpdate(unAction, nValue, bPressed, bOverTarget, x, y);
	}
	return false;
}
//
// ITouchTarget Implementation

ButtonElement* ButtonBarElement::AddButton(int nElementId, TabActionButtonType eButtonType, std::string sLabel) {
	ButtonElement* pButton = new ButtonElement(m_pOverlayWithin, nElementId, sLabel, eOLANone);
	pButton->SetActionType(eButtonType);
	int nInvokeValue = static_cast<int>(m_vcElements.size() + 1);
	int nAction = m_pOverlayEnv->GetInternalActionStart();
	switch(eButtonType) {
	case eTABTOkay:
	case eTABTClose:
		nAction = m_pOverlayEnv->GetCloseAction();
		break;
	case eTABTApply:
		nAction = m_pOverlayEnv->GetApplyTabAction();
		break;
	}
	pButton->SetTargetAction(nAction);
	m_vcElements.push_back(pButton);
	return pButton;
}

bool ButtonBarElement::MoveHighlightLeft() {
	if (m_nHighlightedElementIndex == -1) {
		return false;
	}
	if (m_nHighlightedElementIndex < m_vcElements.size()-1) {
		m_vcElements[m_nHighlightedElementIndex]->Highlight(false);
		++m_nHighlightedElementIndex;
		m_vcElements[m_nHighlightedElementIndex]->Highlight(true);
	}
	return true;
}

bool ButtonBarElement::MoveHighlightRight() {
	if (m_nHighlightedElementIndex == -1) {
		return false;
	}
	if (m_nHighlightedElementIndex > 0) {
		m_vcElements[m_nHighlightedElementIndex]->Highlight(false);
		--m_nHighlightedElementIndex;
		m_vcElements[m_nHighlightedElementIndex]->Highlight(true);
	}
	return true;
}

bool ButtonBarElement::InvokeAction(int nAction, unsigned int unInvokeValue) {
	return true;
}

unsigned int ButtonBarElement::RegisterButtonBar(ButtonBarElement* pBar) {
	unsigned int unHandle = static_cast<unsigned int>(s_mpInvokeHandleToButtonBar.size());
	s_mpInvokeHandleToButtonBar[unHandle] = pBar;
	return unHandle;
}

bool ButtonBarElement::InvokeTargetAction(unsigned int unTargetHandle, int nAction, unsigned int unInvokeValue) {
	ButtonBarElement* pBar = s_mpInvokeHandleToButtonBar[unTargetHandle];
	return pBar->InvokeAction(nAction, unInvokeValue);
}
