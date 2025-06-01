#include "ButtonElement.h"
#include "TouchLayer.h"
#include "IOverlayEnvIntl.h"

ButtonElement::ButtonElement(IOverlayIntl* pOverlay, int nElementId, std::string sText, OverlayLabelAlignment eLabelAlignment)
	: ElementBase(pOverlay, nElementId, eLabelAlignment), m_nTargetAction(-1) {
	m_sButtonLabel = sText;
	m_nButtonTextWidth = 0;

	m_fPreferredWidth = m_pOverlayEnv->GetPreferredControlWidth();
	m_fCtrlWidth = 0.0f;
	m_xCtrlOffset = 0;
	m_bRenderPressed = false;
	m_eButtonType = eTABTNoneTabAction;
	m_pTLLabel = nullptr;
}

ButtonElement::~ButtonElement() {
	delete m_pTLLabel;
	m_pTLLabel = nullptr;
}

void ButtonElement::SetTargetAction(int nTargetAction) {
	m_nTargetAction = nTargetAction;
}

void ButtonElement::SetActionType(TabActionButtonType eButtonType) {
	m_eButtonType = eButtonType;
}

void ButtonElement::GetLabelOffset(float* pfXOffset, float* pfYOffset) {
	float fHeight = GetHeight();
	float fXLeftOffset = m_xCtrlOffset + (m_fCtrlWidth - m_nButtonTextWidth) / 2;
	float fontHeightWithoutDescender = m_fFontHeight - m_fTextDescender;

	*pfXOffset = fXLeftOffset;
	*pfYOffset = (fHeight - fontHeightWithoutDescender) / 2;
}

void ButtonElement::GetBackgroundOffsetAndSize(float* pfXOffset, float* pfYOffset, float* pfWidth, float* pfHeight) {
	float fHeight = GetHeight();
	float fontHeightWithoutDescender = m_fFontHeight - m_fTextDescender;

	// Background is based on the label.
	// The label is centred in the height of the control, but not including the font descenders.
	float fYLabelOffset = (fHeight - fontHeightWithoutDescender) / 2;
	// The background is 'two descenders' below the label origin, and extends 'two descenders' above the label.
	*pfYOffset = fYLabelOffset - m_fTextDescender - m_fTextDescender;

	*pfWidth = m_fPreferredWidth;

	float fXLeftOffset = m_xCtrlOffset + (m_fCtrlWidth - GetPreferredControlWidth()) / 2;
	*pfXOffset = fXLeftOffset;
	*pfHeight = fontHeightWithoutDescender + 4 * m_fTextDescender;
}

// IOverlayElement Implementation
//
void ButtonElement::Deinitialise() {
	FreeSpace();
}

void ButtonElement::FreeSpace() {
	ElementBase::FreeSpace();
	delete m_pTLLabel;
	m_pTLLabel = nullptr;
}

void ButtonElement::InitialiseForWidth(float fWidth) {
	InitialiseContainerForWidth(fWidth);
	if (m_eButtonType != eTABTNoneTabAction) {
		m_xCtrlOffset = 0;
		m_fCtrlWidth = m_fPreferredWidth;
	}
	UpdateButtonText();
}

float ButtonElement::GetHeight() const {
	if (m_eButtonType == eTABTNoneTabAction) {
		return GetHeightIncludingLabel(m_fLineSpacing * 2);
	}
	else {
		return m_fLineSpacing * 2;
	}
}

void ButtonElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	ElementBase::Render(xOrigin, yOrigin, fWidth, fHeight);
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	float fXBackground;
	float fYBackground;
	float fBackgroundWidth;
	float fBackgroundHeight;

	GetBackgroundOffsetAndSize(&fXBackground, &fYBackground, &fBackgroundWidth, &fBackgroundHeight);
	fXBackground += xOrigin;
	fYBackground += yOrigin;
	if (m_bShowDebugBackground) {
		RenderTranslucentOverlay(fXBackground, fYBackground, fBackgroundWidth, fBackgroundHeight, pPublicEnv->GetDebugBackgroundRGBA(false));
	}

	//float fXLabel;
	//float fYLabel;
	//GetLabelOffset(&fXLabel, &fYLabel);
	//fXLabel += xOrigin;
	//fYLabel += yOrigin;

	OverlayElementState elementState;
	elementState.m_bEnabled = !m_bDisabled;
	elementState.m_bHighlighted = m_bHighlighted;
	elementState.m_bActivated = m_bRenderPressed;
	RenderButton(pPublicEnv, m_nElementId, fXBackground, fYBackground, fBackgroundWidth, fBackgroundHeight - 0.2f, elementState, eOA_None, 0.1f, m_pTLLabel);
}

void ButtonElement::Highlight(bool bHighlight) {
	m_bHighlighted = bHighlight;
}

ActionResult ButtonElement::Action(unsigned int unAction) {
	if (unAction == m_pOverlayEnv->GetOkAction()) {
		if (m_nTargetAction < m_pOverlayEnv->GetInternalActionStart()) {
			return ActionResult(m_nTargetAction);
		}
		else {
			switch (m_eButtonType)
			case eTABTApply:
				return m_pOverlayWithin->ApplyTabPressed();
		}
	}
	return false;
}

float ButtonElement::GetScrollIncrement() {
	return m_fLineSpacing * 2;
}

void ButtonElement::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {
	float fXBackground;
	float fYBackground;
	float fBackgroundWidth;
	float fBackgroundHeight;

	GetBackgroundOffsetAndSize(&fXBackground, &fYBackground, &fBackgroundWidth, &fBackgroundHeight);
	fXBackground += xOrigin;
	fYBackground += yOrigin;

	unsigned int id = pTouchLayer->AddTouchArea(fXBackground, fYBackground, fBackgroundWidth,fBackgroundHeight, pTarget, m_nTargetAction, nInvokeValue);
	//if (m_eButtonType == eTABTNoneTabAction) {
		pTouchLayer->SetScrollingBehaviour(id, true, false);
	//}
	pTouchLayer->SetTouchTargetNeedsContinuousUpdates(id, true);
}
//
// IOverlayElement Implementation

// ITouchTarget implementation
//
bool ButtonElement::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	return false;
}

bool ButtonElement::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) {
	m_bRenderPressed = bPressed && bOverTarget;
	return true;
}
//
// ITouchTarget implementation

void ButtonElement::UpdateButtonText() {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	if (!hTextFont.IsValid()) {
		return;
	}
	auto sz = s_pTextRenderer->MeasureText(hTextFont, m_sButtonLabel, 1.0f);
	m_nButtonTextWidth = std::get<2>(sz);
	delete m_pTLLabel;
	m_pTLLabel = nullptr;
	m_pTLLabel = s_pTextRenderer->CreateTextLine(hTextFont, m_sButtonLabel, m_fCtrlWidth, 1.0f);
}

void ButtonElement::ActionOkay() {
}