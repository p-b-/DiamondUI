#include "NumericSelectorElement.h"
#include "TouchLayer.h"
#include "IOverlayEnvIntl.h"
#include <sstream>
#include <algorithm>

#define SELECTOR_BUTTON_WIDTH 50
#define SELECTOR_BUTTON_HEIGHT 30

NumericSelectorElement::~NumericSelectorElement() {
	delete m_pTLValue;
	m_pTLValue = nullptr;
}

NumericSelectorElement::NumericSelectorElement(IOverlayIntl* pOverlay, int nElementId, std::string sLabelText, OverlayLabelAlignment eLabelAlignment, int* pnValue, int nMin, int nMaxInclusive, int nStep)
: ElementBase(pOverlay, nElementId, sLabelText, eLabelAlignment) {
	m_pnValue = pnValue;
	m_nMin = nMin;
	m_nMaxInclusive = nMaxInclusive;
	m_nStep = nStep;

	m_nValue = *m_pnValue;

	m_fValueWidth = 0;

	m_fCtrlWidth = 0.0f;
	m_xCtrlOffset = 0;
	m_pTLValue = nullptr;
}

//void NumericSelectorElement::GetValueLabelOffset(float* pfXOffset, float* pfYOffset) {
//	float fControlWidth = m_pOverlayEnv->GetPreferredControlWidth();;
////	float fXCentreOffset = static_cast<float>(m_xCtrlOffset + fControlWidth / 2);
//	float fXLeftOffset = m_xCtrlOffset + (m_fCtrlWidth - m_fValueWidth) / 2;
//	float fontHeightWithoutDescender = m_fFontHeight - m_fTextDescender;
//	float fOverallCtrlHeight = GetHeight();
//
////	*pfXOffset = fXCentreOffset - m_nValueWidth / 2;
//	*pfXOffset = fXLeftOffset;
//	if (m_eLabelAlignment == eOLALeft || m_eLabelAlignment == eOLANone) {
//		*pfYOffset = (fOverallCtrlHeight - fontHeightWithoutDescender) / 2;
//	}
//	else {
//		*pfYOffset = m_fTextDescender*2;
//	}
//}

void NumericSelectorElement::GetBackgroundOffsetAndSize(float* pfXOffset, float* pfYOffset, float* pfWidth, float* pfHeight) {
	float fOverallCtrlHeight = GetHeight();
	float fControlWidth = m_pOverlayEnv->GetPreferredControlWidth();;

	float fXLeftOffset = m_xCtrlOffset + (m_fCtrlWidth - fControlWidth) / 2;

	*pfXOffset = fXLeftOffset;
	*pfWidth = fControlWidth;
	*pfHeight = m_fLineSpacing;
	if (m_eLabelAlignment == eOLALeft || m_eLabelAlignment == eOLANone) {
		*pfYOffset = (fOverallCtrlHeight - *pfHeight) / 2;
	}
	else {
		*pfYOffset = m_fTextDescender;
	}
}

// IOverlayElement Implementation
//
void NumericSelectorElement::Deinitialise() {
	FreeSpace();
}

void NumericSelectorElement::FreeSpace() {
	ElementBase::FreeSpace();
	delete m_pTLValue;
	m_pTLValue = nullptr;
}

void NumericSelectorElement::InitialiseForWidth(float fWidth) {
	InitialiseContainerForWidth(fWidth);
	UpdateValueText();
}

float NumericSelectorElement::GetHeight() const {
	if (m_eLabelAlignment == eOLALeft) {
		return GetHeightIncludingLabel(m_fLineSpacing * 2);
	}
	else {
		return GetHeightIncludingLabel(m_fLineSpacing+m_fTextDescender);
	}
}

void NumericSelectorElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	ElementBase::Render(xOrigin, yOrigin, fWidth, fHeight);
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	rgb textRGB = pPublicEnv->GetTextRGB();

	rgba rgbaVector = pPublicEnv->GetVectorColour(m_bHighlighted);

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

	float yUnusedVerticalPaddingInArrow = 7;
	float yArrowPoint = fYBackground - yUnusedVerticalPaddingInArrow + m_fTextDescender;

	/*float fXLabel;
	float fYLabel;
	GetValueLabelOffset(&fXLabel, &fYLabel);
	fXLabel += xOrigin;
	fYLabel += yOrigin;*/

	OverlayElementState elementState;
	elementState.m_bEnabled = !m_bDisabled;
	elementState.m_bHighlighted = m_bHighlighted;
	elementState.m_bActivated = false;
	RenderSelector(pPublicEnv, m_nElementId, fXBackground, fYBackground, fBackgroundWidth, fBackgroundHeight - 0.2f, elementState, m_pTLValue);
}

void NumericSelectorElement::Highlight(bool bHighlight) {
	m_bHighlighted = bHighlight;
}

float NumericSelectorElement::GetScrollIncrement() {
	return m_fLineSpacing * 2;
}

void NumericSelectorElement::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {
	float fXBackground;
	float fYBackground;
	float fBackgroundWidth;
	float fBackgroundHeight;

	GetBackgroundOffsetAndSize(&fXBackground, &fYBackground, &fBackgroundWidth, &fBackgroundHeight);
	fXBackground += xOrigin;
	fYBackground += yOrigin;

	int id = pTouchLayer->AddTouchArea(fXBackground, fYBackground,
		SELECTOR_BUTTON_WIDTH, SELECTOR_BUTTON_HEIGHT, pTarget, m_pOverlayEnv->GetMoveLeftAction(), nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, true, false);
	id = pTouchLayer->AddTouchArea(fXBackground+fBackgroundWidth-SELECTOR_BUTTON_WIDTH, fYBackground, 
		SELECTOR_BUTTON_WIDTH, SELECTOR_BUTTON_HEIGHT, pTarget, m_pOverlayEnv->GetMoveRightAction(), nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, true, false);
	id = pTouchLayer->AddTouchArea(fXBackground + SELECTOR_BUTTON_WIDTH, fYBackground, 
		fBackgroundWidth - 2 * SELECTOR_BUTTON_WIDTH,
		fBackgroundHeight, pTarget, m_pOverlayEnv->GetHighlightElementAction(), nInvokeValue);
	pTouchLayer->SetScrollingBehaviour(id, true, false);
}

ActionResult NumericSelectorElement::Action(unsigned int unAction) {
	if (unAction == m_pOverlayEnv->GetMoveLeftAction()) {
		ActionLeft();
		return true;
	}
	else if (unAction == m_pOverlayEnv->GetMoveRightAction()) {
		ActionRight();
		return true;
	}
	else if (unAction == m_pOverlayEnv->GetHighlightElementAction()) {
		return true;
	}
	return false;
}

bool NumericSelectorElement::Altered() const { 
	return m_nValue != *m_pnValue;
}

bool NumericSelectorElement::CanSave() const {
	return (m_nValue >= m_nMin && m_nValue <= m_nMaxInclusive);
}

void NumericSelectorElement::Save() { 
	*m_pnValue = m_nValue;
}

void NumericSelectorElement::Revert() { 
	m_nValue = *m_pnValue;
}
//
// IOverlayElement Implementation

// ITouchTarget implementation
//
bool NumericSelectorElement::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	return false;
}
//
// ITouchTarget implementation

bool NumericSelectorElement::ActionRight() {
	if (m_nValue < m_nMaxInclusive) {
		m_nValue = std::min(m_nValue+m_nStep, m_nMaxInclusive);
		UpdateValueText();
		return true;
	}
	return false;
}

bool NumericSelectorElement::ActionLeft() {
	if (m_nValue > m_nMin) {
		m_nValue = std::max(m_nValue-m_nStep, m_nMin);
		UpdateValueText();
		return true;
	}
	return false;
}

void NumericSelectorElement::UpdateValueText() {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	if (!hTextFont.IsValid()) {
		return;
	}
	std::ostringstream ss;
	ss << m_nValue;

	auto sz = s_pTextRenderer->MeasureText(hTextFont, ss.str(), 1.0f);
	m_fValueWidth = static_cast<float>(std::get<2>(sz));
	delete m_pTLValue;
	m_pTLValue = nullptr;
	m_pTLValue = s_pTextRenderer->CreateTextLine(hTextFont, ss.str(), m_fCtrlWidth, 1.0f);
}