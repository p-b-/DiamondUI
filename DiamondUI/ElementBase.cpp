#include "ElementBase.h"
#include "IOverlayIntl.h"
#include "IOverlayEnvIntl.h"

#define LABEL_MARGIN 10

ElementBase::ElementBase(IOverlayIntl* pOverlay, int nElementId, std::string sLabelText, OverlayLabelAlignment eLabelAlignment) :
	m_fCtrlWidth(0.0f), m_xCtrlOffset(0), m_fFontHeight(0.0f), m_fLineSpacing(0.0f), m_fTextDescender(0.0f) {
	m_pOverlayWithin = pOverlay;
	m_nElementId = nElementId;
	m_sLabelText = sLabelText;
	m_eLabelAlignment = eLabelAlignment;

	m_pOverlayEnv = m_pOverlayWithin->GetInternalOverlayEnvironment();

	m_bHasLabel = true;

	m_yLastRenderedOrigin = 0;
	m_bRenderedAtLeastOnce = false;

	m_bDisabled = false;
	m_bHighlighted = false;

	m_bShowDebugBackground = false;
}

ElementBase::ElementBase(IOverlayIntl* pOverlay, int nElementId, OverlayLabelAlignment eLabelAlignment) :
	m_fCtrlWidth(0.0f), m_xCtrlOffset(0), m_fFontHeight(0.0f), m_fLineSpacing(0.0f), m_fTextDescender(0.0f) {
	m_pOverlayWithin = pOverlay;
	m_nElementId = nElementId;
	m_eLabelAlignment = eLabelAlignment;

	m_pOverlayEnv = m_pOverlayWithin->GetInternalOverlayEnvironment();

	m_bHasLabel = false;

	m_yLastRenderedOrigin = 0;
	m_bRenderedAtLeastOnce = false;

	m_bDisabled = false;
	m_bHighlighted = false;

	m_bShowDebugBackground = false;
}

ElementBase::~ElementBase() {
	if (m_bHasLabel) {
		std::vector<ITextLine*> vcTL = m_vcLabelTextLines;
		m_vcLabelTextLines.clear();
		for (auto pTL : vcTL) {
			delete pTL;
		}
	}
}

// IOverlayElement implementation
//
void ElementBase::Deinitialise() {
	FreeSpace();
}

void ElementBase::FreeSpace() {
	if (m_bHasLabel) {
		std::vector<ITextLine*> vcTL = m_vcLabelTextLines;
		m_vcLabelTextLines.clear();
		for (auto tl : vcTL) {
			delete tl;
		}
		m_bHasLabel = false;
	}
}

int ElementBase::GetId() const {
	return m_nElementId;
}

float ElementBase::GetPreferredControlWidth() const {
	return static_cast<float>(m_pOverlayEnv->GetPreferredControlWidth());
}

void ElementBase::SetPreferredControlWidth(float fWidth) {
	// Cannot currently set preferred width for most controls
}

float ElementBase::GetLastRenderedYOrigin() const {
	return m_yLastRenderedOrigin;
}

void ElementBase::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	m_bRenderedAtLeastOnce = true;
	m_yLastRenderedOrigin = yOrigin;
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	if (m_bShowDebugBackground) {
		RenderTranslucentOverlay(xOrigin, yOrigin, m_xCtrlOffset - 1, fHeight, pPublicEnv->GetDebugBackgroundRGBA(true));
		RenderTranslucentOverlay(xOrigin + m_xCtrlOffset, yOrigin, (fWidth - m_xCtrlOffset), fHeight, pPublicEnv->GetDebugBackgroundRGBA(true));
	}
	if (m_bHasLabel && m_vcLabelTextLines.size()>0) {
		rgb textRGB = pPublicEnv->GetTextRGB();

		float fontHeightWithoutDescender = m_fFontHeight - m_fTextDescender;
		float y = yOrigin + fHeight - m_fLineSpacing;
		if (m_eLabelAlignment == eOLAAbove) {
			float fLabelWidth = m_vcLabelTextLines[0]->GetWidth();
			y = yOrigin + fHeight - m_fFontHeight - m_fLineSpacing*0.25f;
			xOrigin += m_xCtrlOffset + (m_fCtrlWidth - fLabelWidth) / 2;

			if (m_bShowDebugBackground) {
				RenderTranslucentOverlay(xOrigin, y, fLabelWidth, 20.0f, pPublicEnv->GetDebugBackgroundRGBA(false));
			}

		}
		else if (m_vcLabelTextLines.size() == 1) {
			y = yOrigin + (fHeight - fontHeightWithoutDescender) / 2;
		}
		else {
			// Only vertically centre the top line (including linespacing) and the bottom line without the linespacing, and then add on the linespacing to 
			//  take this virtual cursor up to the top line.
			y = yOrigin + (fHeight - m_fLineSpacing-fontHeightWithoutDescender)/2+m_fLineSpacing;
		}
		for (ITextLine* pTL : m_vcLabelTextLines) {
			s_pTextRenderer->RenderTextLine(pTL, xOrigin, y, textRGB);
			y -= m_fLineSpacing;
		}
	}
}

void ElementBase::ShowDebugBackground(bool bShow) {
	m_bShowDebugBackground = bShow;
}
//
// IOverlayElement implementation

// ITouchTarget implementation
//
bool ElementBase::Invoke(unsigned int unAction, int nValue, float fDeltaTime) {
	return false;
}
//
// ITouchTarget implementation

float ElementBase::GetHeightIncludingLabel(float fHeightOfControl) const {
	if (!m_bHasLabel) {
		return fHeightOfControl;
	}
	if (m_eLabelAlignment == eOLALeft) {
		return std::max(fHeightOfControl, m_fLineSpacing * 2);
	}
	else if (m_eLabelAlignment == eOLAAbove) {
		return fHeightOfControl + m_fLineSpacing*1.5f;
	}
	else {
		return fHeightOfControl;
	}
}

void ElementBase::InitialiseContainerForWidth(float fWidth) {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	if (!hTextFont.IsValid()) {
		return;
	}
	InitialiseFontMetrics();

	switch (m_eLabelAlignment) {
	case eOLALeft:
		InitialiseLeftLabelForWidth(fWidth, hTextFont);
		break;
	case eOLAAbove:
		InitialiseAboveLabelForWidth(fWidth, hTextFont);
		break;
	case eOLANone:
	default:
		InitialiseWidthForNoLabel(fWidth, hTextFont);
	}
}

void ElementBase::InitialiseFontMetrics() {
	const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();

	FontFaceHandle hTextFont = pPublicEnv->GetTextFontFace();
	m_fFontHeight = s_pTextRenderer->GetFontHeight(hTextFont);
	m_fLineSpacing = m_fFontHeight * 1.25f;
	m_fTextDescender = s_pTextRenderer->GetFontDescenderHeight(hTextFont);
}

void ElementBase::InitialiseLeftLabelForWidth(float fWidth, FontFaceHandle hTextFont) {
	float ratio = m_pOverlayEnv->GetTextToElementRatio();
	
	float labelWidth = ratio * fWidth - 2 * LABEL_MARGIN; // margin before and after
	m_xCtrlOffset = labelWidth + 2 * LABEL_MARGIN;
	m_fCtrlWidth = fWidth - m_xCtrlOffset;

	if (m_bHasLabel) {
		CreateGraphicalTextLinesForLabel(hTextFont, m_sLabelText, labelWidth, 2);
	}
}

void ElementBase::InitialiseAboveLabelForWidth(float fWidth, FontFaceHandle hTextFont) {
	float ctrlWidth = m_pOverlayEnv->GetPreferredControlWidth();

	m_xCtrlOffset = 0;
	m_fCtrlWidth = fWidth;

	if (m_bHasLabel) {
		CreateGraphicalTextLinesForLabel(hTextFont, m_sLabelText, fWidth, 1);
	}
}

void ElementBase::InitialiseWidthForNoLabel(float fWidth, FontFaceHandle hTextFont) {
	m_fCtrlWidth = GetPreferredControlWidth();
	m_xCtrlOffset = (fWidth - m_fCtrlWidth) / 2;
}

void ElementBase::CreateGraphicalTextLinesForLabel(FontFaceHandle hTextFont, std::string sLabel, float fLabelWidth, int nMaxLineCount ) {
	m_vcLabelTextLines = s_pTextRenderer->CreateMultipleTextLines(hTextFont, m_sLabelText, fLabelWidth - LABEL_MARGIN, 1.0f);
	while (m_vcLabelTextLines.size() > nMaxLineCount) {
		ITextLine* pTL = m_vcLabelTextLines.back();
		m_vcLabelTextLines.pop_back();
		delete pTL;
	}
}