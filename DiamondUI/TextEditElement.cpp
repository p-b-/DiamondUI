#include "TextEditElement.h"
#include "IOverlayIntl.h"
#include "IOverlayEnvIntl.h"
#include "TouchLayer.h"
#include <iostream>

#define FRAMES_BEFORE_KB_DISPLAY (144/6)
#define FRAMES_BEFORE_KB_DISPLAY_QUICKLY 10

TextEditElement::TextEditElement(IOverlayIntl* pOverlay, int nElementId, std::string sLabel, std::string& sText, OverlayLabelAlignment eLabelAlignment, int nMaxLength /*=-1*/)
	: ElementBase(pOverlay, nElementId, sLabel, eLabelAlignment), m_sOriginalText(sText) {
	m_fPreferredWidth = 250;
	m_fCtrlWidth = 0.0f;
	m_xCtrlOffset = 0;
	m_sText = sText;
	m_nMaxLength = nMaxLength; 
	m_nCursorPos = 0;
	m_fCursorXOffset = 0.0f;
	m_bCentred = false;

	// When element is not highlighted but is being touched to drag where the caret will be
	m_bDrawTemporaryCaret = false;

	m_bStartKeyboardInputOnRender = false;
	m_bKeyboardDisplayed = false;
	m_nFramesBeforeKeyboardDisplay = FRAMES_BEFORE_KB_DISPLAY;
	m_pTLText = nullptr;

	HideText(false, '\0');
}

TextEditElement::~TextEditElement() {
	delete m_pTLText;
	m_pTLText = nullptr;
}

void TextEditElement::HideText(bool bHideText, char cPasswordChar) {
	m_bHideText = bHideText;
	m_cPasswordChar = cPasswordChar;
}

// IOverlayElement Implementation
//
void TextEditElement::Deinitialise() {
	FreeSpace();
}

void TextEditElement::FreeSpace() {
	ElementBase::FreeSpace();
	delete m_pTLText;
	m_pTLText = nullptr;
}

void TextEditElement::InitialiseForWidth(float fWidth) {
	InitialiseContainerForWidth(fWidth);
	InitialiseControlDimensions();
	UpdateText();
	CalculateCursorXOffset();
}

float TextEditElement::GetHeight() const {
	if (m_eLabelAlignment == eOLALeft) {
		return GetHeightIncludingLabel(m_fLineSpacing * 2);
	}
	else {
		return GetHeightIncludingLabel(m_fLineSpacing + m_fTextDescender);
	}
}

void TextEditElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	ElementBase::Render(xOrigin, yOrigin, fWidth, fHeight);
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	if (m_bShowDebugBackground) {
		rgba debugBackground = pPublicEnv->GetDebugBackgroundRGBA(false);
		RenderDebugBackground(m_nElementId, m_fXBackground + xOrigin, m_fBackgroundHeight + yOrigin, m_fBackgroundWidth, m_fBackgroundHeight, debugBackground);
	}

	rgb textRGB = pPublicEnv->GetTextRGB();
	// TODO Make this colour configurable
	rgba textBgd = MakeRGBA(0.1f, 0.1f, 0.5f, 0.8f);
	bool bHighlight = m_bHighlighted && !m_bDisabled;

	rgba rgbaButton = pPublicEnv->GetVectorColour(bHighlight);

	RenderTextEditField(m_nElementId, m_fXBackground+xOrigin, m_fYBackground+yOrigin, m_fBackgroundWidth, m_fBackgroundHeight - 0.2f, rgbaButton);
	float xTextOrigin = m_fXTextOffset  + xOrigin + m_xTextDrawOffset;

	SetClippingArea(m_fXTextOffset + xOrigin,
		m_fYTextOffset + yOrigin - m_fTextDescender,
		m_fTextWidth, m_fTextHeight);
	s_pTextRenderer->RenderTextLine(m_pTLText, xTextOrigin, m_fYTextOffset + yOrigin, textRGB);
	RemoveClippingArea();
	if (m_bShowDebugBackground && m_bKeyboardDisplayed) {
		RenderTextEditField(m_nElementId, xOrigin,yOrigin, fWidth, fHeight-5.0f, MakeRGBA(1.0f,0.5f,0.5f,1.0f));
	}

	if (bHighlight || m_bDrawTemporaryCaret) {
		// TODO Make this colour configurable
		rgba caretColour = MakeRGBA(1.0f, 1.0f, 1.0f, 1.0f);
		RenderCaret(m_nElementId, xTextOrigin + m_fCursorXOffset, m_fYBackground + yOrigin + m_fTextDescender, m_fFontHeight + m_fTextDescender, caretColour);
	}

	if (m_bStartKeyboardInputOnRender) {
		if (m_nFramesBeforeKeyboardDisplay > 0) {
			--m_nFramesBeforeKeyboardDisplay;
		}
		else {
			m_bStartKeyboardInputOnRender = false;
			IntRect rcTargetArea = IntRect(static_cast<int>(xOrigin), static_cast<int>(yOrigin), static_cast<int>(fWidth), static_cast<int>(fHeight));
			m_bKeyboardDisplayed = m_pOverlayWithin->ActivateKeyboardInput(eCS_Text, this, rcTargetArea);
		}
	}
}

void TextEditElement::Highlight(bool bHighlight) {
	m_bHighlighted = bHighlight;

	
	if (bHighlight) {
		if (!m_bDrawTemporaryCaret) {
			m_nCursorPos = 0;
		}
		CalculateCursorXOffset();
		m_pOverlayWithin->ActivateKeyboardInput(eCS_Text, this, IntRect(0, 0, 0, 0));
	}
	else {
		m_pOverlayWithin->ActivateKeyboardInput(eCS_None, nullptr, IntRect(0,0,0,0));
	}
	m_bDrawTemporaryCaret = false;

}

ActionResult TextEditElement::Action(unsigned int unAction) {
	if (unAction == m_pOverlayEnv->GetMoveLeftAction()) {
		KeyPressed(eTIK_LeftArrow, false, false, false);

	}
	else if (unAction == m_pOverlayEnv->GetMoveRightAction()) {
		KeyPressed(eTIK_RightArrow, false, false, false);
	}
	else if (unAction == m_pOverlayEnv->GetOkAction()) {
		if (!m_bKeyboardDisplayed) {
			DisplayKeyboardSoon(true);
		}
	}
	return false;
}

float TextEditElement::GetScrollIncrement() {
	return m_fLineSpacing * 2;
}

void TextEditElement::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {
	int id = pTouchLayer->AddTouchArea(m_fXBackground+xOrigin, m_fYBackground+yOrigin,
		m_fBackgroundWidth, m_fBackgroundHeight, pTarget, m_pOverlayEnv->GetOkAction(), nInvokeValue);
	pTouchLayer->SetTouchTargetNeedsContinuousUpdates(id, true);
	pTouchLayer->SetScrollingBehaviour(id, true, false);
}

bool TextEditElement::Altered() const {
	if (m_sText == m_sOriginalText) {
		return false;
	}
	return true;
}

void TextEditElement::Save() {
	m_sOriginalText = m_sText;
}

void TextEditElement::Revert() {
	m_sText = m_sOriginalText;
	UpdateText();
	CalculateCursorXOffset();
}

void TextEditElement::ResetDisplayState() {
	m_nCursorPos = 0;
	m_bStartKeyboardInputOnRender = false;
	m_bDrawTemporaryCaret = false;
	CalculateCursorXOffset();
}
//
// IOverlayElement Implementation

// ITouchTarget implementation
//
bool TextEditElement::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	return false;
}

bool TextEditElement::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) {
	float fX = static_cast<float>(x);
	if (!bPressed) {
		return true;
	}
	if (!m_bHighlighted) {
		m_bDrawTemporaryCaret = true;
	}
	if (m_bCentred) {
		if (fX < static_cast<int>(m_xTextDrawOffset)) {
			m_nCursorPos = 0;
		}
		else if (fX > m_xTextDrawOffset + m_pTLText->GetWidth()) {
			m_nCursorPos = static_cast<int>(m_sText.length());
		}
		else {
			for (int i = 1; i <= m_sText.length(); ++i) {
				float xCharOffset = m_pTLText->GetCharOffset(i);
				if (fX < m_xTextDrawOffset + xCharOffset) {
					m_nCursorPos = i - 1;
					break;
				}
			}
		}
		CalculateCursorXOffset();

		return true;
	}
	else {
		// TODO Animate this so not reliant on touch/mouse moving to generate update events
		// Consider making it accelerate the further away from the input box
		if (fX < 0) {
			if (m_nCursorPos > 0) {
				KeyPressed(eTIK_LeftArrow, false, false, false);
			}
		}
		else if (fX > m_fPreferredWidth) {
			if (m_nCursorPos < m_sText.length()) {
				KeyPressed(eTIK_RightArrow, false, false, false);
			}
		}
		else {
			for (int i = 1; i <= m_sText.length(); ++i) {
				float xCharOffset = m_pTLText->GetCharOffset(i);
				if (fX < m_xTextDrawOffset + xCharOffset) {
					m_nCursorPos = i - 1;
					break;
				}
			}
			CalculateCursorXOffset();
			return true;
		}
	}
	return false;
}
//
// ITouchTarget implementation

// ITextInputTarget implementation
//
void TextEditElement::CharInput(char ch) {
	if (m_nMaxLength != -1 && m_sText.length() >= m_nMaxLength) {
		return;
	}
	if (m_nCursorPos == 0) {
		m_sText = ch + m_sText;
	}
	else if (m_nCursorPos < m_sText.length()) {
		m_sText = m_sText.substr(0, m_nCursorPos) + ch + m_sText.substr(m_nCursorPos);
	}
	else {
		m_sText += ch;
	}
	++m_nCursorPos;
	UpdateText();
	CalculateCursorXOffset();
}

void TextEditElement::KeyPressed(TextInputKey eKey, bool bShiftPressed, bool bCtrlPressed, bool bAltPressed) {
	switch (eKey) {
	case eTIK_LeftArrow:
		if (bCtrlPressed) {
			JumpLeftToWhitespace();
		}
		else if (m_nCursorPos > 0) {
			--m_nCursorPos;
			CalculateCursorXOffset();
		}
		break;
	case eTIK_RightArrow:
		if (bCtrlPressed) {
			JumpRightToWhitespace();
		}
		else if (m_nCursorPos < m_sText.length()) {
			++m_nCursorPos;
			CalculateCursorXOffset();
		}
		break;
	case eTIK_Home:
		m_nCursorPos = 0;
		CalculateCursorXOffset();
		break;
	case eTIK_End:
		m_nCursorPos = static_cast<int>(m_sText.length());
		CalculateCursorXOffset();
		break;
	case eTIK_Delete:
		if (m_nCursorPos < m_sText.length()) {
			if (m_nCursorPos == 0) {
				m_sText = m_sText.substr(1);
			}
			else {
				m_sText = m_sText.substr(0, m_nCursorPos) + m_sText.substr(m_nCursorPos + 1);
			}
			UpdateText();
			CalculateCursorXOffset();
		}
		break;
	case eTIK_Backspace:
		if (m_nCursorPos > 0) {
			if (m_nCursorPos > 1) {
				if (m_nCursorPos < m_sText.length()) {
					// Delete a middle character
					m_sText = m_sText.substr(0, m_nCursorPos - 1) + m_sText.substr(m_nCursorPos);
				}
				else {
					// Delete the last character
					m_sText = m_sText.substr(0, m_nCursorPos - 1);
				}
			}
			else {
				// Delete the first character
				m_sText = m_sText.substr(m_nCursorPos);
			}
			--m_nCursorPos;
			UpdateText();
			CalculateCursorXOffset();
		}
		break;
	}
}

void TextEditElement::KeyboardDismissed() {
	m_bKeyboardDisplayed = false;
}
//
// ITextInputTarget implementation

void TextEditElement::InitialiseControlDimensions() {
	m_fTextMargin = 2 * m_fTextDescender;

	float fHeight = GetHeight();
	float fontHeightWithoutDescender = m_fFontHeight - m_fTextDescender;

	float fYLabelOffset = m_fTextDescender * 2;

	m_fXBackground = m_xCtrlOffset + (m_fCtrlWidth - GetPreferredControlWidth()) / 2;
	// The background is 'two descenders' below the label origin, and extends 'two descenders' above the label.
	m_fYBackground = fYLabelOffset - m_fTextDescender * 2;
	m_fBackgroundWidth = m_fPreferredWidth;
	m_fBackgroundHeight = fontHeightWithoutDescender + 4 * m_fTextDescender;

	m_fXTextOffset = m_fTextMargin + m_fXBackground;
	m_fYTextOffset = m_fTextDescender * 2 + m_fYBackground;
	m_fTextWidth = m_fPreferredWidth - 2 * m_fTextMargin;
	m_fTextHeight = m_fFontHeight;
}

void TextEditElement::UpdateText() {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	if (!hTextFont.IsValid()) {
		return;
	}
	auto sz = s_pTextRenderer->MeasureText(hTextFont, m_sText, 1.0f);
	delete m_pTLText;
	m_pTLText = nullptr;
	m_pTLText = s_pTextRenderer->CreateEditableTextLine(hTextFont, m_sText, m_fCtrlWidth, 1.0);
}

void TextEditElement::CalculateCursorXOffset() {
	float fCursorWidth = 2.0f;

	float fCtrlWidth = m_fTextWidth - fCursorWidth;
	if (fCtrlWidth > m_pTLText->GetWidth()) {
		m_bCentred = true;
		float xMargin = m_fTextMargin * 2;

		m_xTextDrawOffset = (fCtrlWidth - m_pTLText->GetWidth()) / 2;
		m_fCursorXOffset = m_pTLText->GetCharOffset(m_nCursorPos);
	}
	else {
		m_bCentred = false;
		bool bAfterLastChar = m_nCursorPos == m_sText.length();
		if (m_nCursorPos == 0) {
			m_fCursorXOffset = 0.0f;
			m_xTextDrawOffset = 0.0f;
		}
		else if (m_nCursorPos == m_sText.length()) {
			m_xTextDrawOffset = m_fTextWidth - m_pTLText->GetCharOffset(m_nCursorPos);
			m_fCursorXOffset = m_pTLText->GetCharOffset(m_nCursorPos) - fCursorWidth;
		}
		else {
			m_fCursorXOffset = m_pTLText->GetCharOffset(m_nCursorPos);
			if (m_pTLText->GetCharOffset(m_nCursorPos + 1) + m_xTextDrawOffset > fCtrlWidth) {
				// Ensure character to right of cursor fits in edit box
				float diff = m_fTextWidth - m_pTLText->GetCharOffset(m_nCursorPos + 1);
				m_xTextDrawOffset = diff;
			}
			else if (m_pTLText->GetCharOffset(m_nCursorPos - 1) + m_xTextDrawOffset < 0.0f) {
				float diff = -m_xTextDrawOffset - m_pTLText->GetCharOffset(m_nCursorPos - 1);
				m_xTextDrawOffset += diff;
			}
			else if (m_xTextDrawOffset + m_fCursorXOffset > fCtrlWidth) {
				float diff = m_fTextWidth - m_fCursorXOffset;
				m_xTextDrawOffset = diff;
				m_fCursorXOffset = m_pTLText->GetCharOffset(m_nCursorPos);
			}
		}
	}
}

void TextEditElement::JumpRightToWhitespace(){ 
	bool bMoved = false;
	bool bFoundWhitespace = false;
	while(m_nCursorPos<m_sText.length()) {
		if (m_sText[m_nCursorPos] == ' ' || m_sText[m_nCursorPos] == '\t') {
			bFoundWhitespace = true;
		}
		++m_nCursorPos;
		bMoved = true;
		if (m_nCursorPos < m_sText.length()) {
			if (bFoundWhitespace &&
				(m_sText[m_nCursorPos] != ' ' && m_sText[m_nCursorPos] != '\t')) {
				break;
			}
		}
	}
	if (bMoved) {
		CalculateCursorXOffset();
	}
}

void TextEditElement::JumpLeftToWhitespace() {
	if (m_nCursorPos == 0) {
		return;
	}
	bool bMoved = false;

	if (m_sText[m_nCursorPos - 1] == ' ' || m_sText[m_nCursorPos - 1] == '\t') {
		--m_nCursorPos;
		bMoved = true;
		// abc abc    abcd
		//       ^    ^
		//     end    start
		//    here    here
		// 
		// ( before pointed at character)
		// 
		// Skip to left of whitespace
		while (m_nCursorPos > 0) {
			if (m_sText[m_nCursorPos - 1] != ' ' && m_sText[m_nCursorPos - 1] != '\t') {
				break;
			}
			--m_nCursorPos;
		}

	}
	//       start here
	//       |
	// abc abc    abcd
	//     ^
	//     end here   
	// 
	// ( before pointed at character)
	// 
	// Skip to left of non-whitespace
	while (m_nCursorPos > 0) {
		if (m_sText[m_nCursorPos - 1] == ' ' || m_sText[m_nCursorPos - 1] == '\t') {
			break;
		}
		--m_nCursorPos;
		bMoved = true;
	}
	if (bMoved) {
		CalculateCursorXOffset();
	}
}

void TextEditElement::DisplayKeyboardSoon(bool bQuickly) {
	m_bStartKeyboardInputOnRender = true;
	if (bQuickly) {
		m_nFramesBeforeKeyboardDisplay = FRAMES_BEFORE_KB_DISPLAY_QUICKLY;
	}
	else {
		m_nFramesBeforeKeyboardDisplay = FRAMES_BEFORE_KB_DISPLAY;
	}
}