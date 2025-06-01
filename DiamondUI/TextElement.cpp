#include "TextElement.h"
#include "IOverlayEnvIntl.h"

TextElement::TextElement(IOverlayIntl* pOverlay, int nElementId, std::string sText, bool bSingleLine /*=false*/) {
	m_pOverlayWithin = pOverlay;
	m_nElementId = nElementId;
	m_sText = sText;
	m_bSingleLine = bSingleLine;

	m_bDisabled = false;
	m_bHighlighted = false;
	m_bRenderedAtLeastOnce = false;
	m_yLastRenderedOrigin = 0;

	m_bShowDebugBackground = false;

	m_fFontHeight = 0.0f;
	m_fLineSpacing = 0.0f;
	m_fTextDescender = 0.0f;

	m_pOverlayEnv = m_pOverlayWithin->GetInternalOverlayEnvironment();
}

TextElement::~TextElement() {
	// Cannot call freespace, or deinitialise, as they are virtual methods.
	// This code should be put into a shared method.
	std::vector<ITextLine*> vcTL = m_vcTextLines;
	m_vcTextLines.clear();
	for (auto pTL : vcTL) {
		delete pTL;
	}
}

// IOverlayElement Implementation
//
void TextElement::Deinitialise() {
	FreeSpace();
}

void TextElement::FreeSpace() {
	// TODO If freeing space up can take place during rendering, need a mutex here
	std::vector<ITextLine*> vcTL = m_vcTextLines;
	m_vcTextLines.clear();
	for (auto pTL : vcTL) {
		delete pTL;
	}
}

void TextElement::InitialiseForWidth(float fWidth) {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	if (!hTextFont.IsValid()) {
		return;
	}

	if (m_bSingleLine) {
		ITextLine* pTL = s_pTextRenderer->CreateTextLine(hTextFont, m_sText, fWidth, 1.0f);
		m_vcTextLines.push_back(pTL);
	}
	else {
		m_vcTextLines = s_pTextRenderer->CreateMultipleTextLines(hTextFont, m_sText, fWidth, 1.0f);
	}
	m_fFontHeight = s_pTextRenderer->GetFontHeight(hTextFont);
	m_fLineSpacing = m_fFontHeight * 1.25f;
	m_fTextDescender = s_pTextRenderer->GetFontDescenderHeight(hTextFont);
}

float TextElement::GetHeight() const {
	return m_vcTextLines.size() * m_fLineSpacing;
}

float TextElement::GetPreferredControlWidth() const {
	if (m_fPreferredWidth > 0.1f) {
		return m_fPreferredWidth;
	}
	else {
		return m_pOverlayEnv->GetPreferredControlWidth();
	}
}

void TextElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	m_yLastRenderedOrigin = yOrigin;
	m_bRenderedAtLeastOnce = true;
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
	rgb textRGB = pPublicEnv->GetTextRGB();

	float y = yOrigin + fHeight - m_fFontHeight;// +m_fTextDescender;
	for (const ITextLine* pTL : m_vcTextLines) {
		s_pTextRenderer->RenderTextLine(pTL, xOrigin, y, textRGB);
		y -= m_fLineSpacing;
	}
}

void TextElement::Disable(bool bDisable) {
	m_bDisabled = bDisable;
}

void TextElement::Highlight(bool bHighlight) {
	m_bHighlighted = bHighlight;
}

float TextElement::GetScrollIncrement() {
	return m_fLineSpacing;
}

void TextElement::ShowDebugBackground(bool bShow) {
	m_bShowDebugBackground = bShow;
}
//
// IOverlayElement Implementation

