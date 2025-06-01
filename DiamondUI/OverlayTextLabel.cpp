#include "OverlayTextLabel.h"

#define X_MARGIN 5
#define Y_MARGIN 5

OverlayTextLabel::OverlayTextLabel(std::string sText, int nWidth, int nHeight, FontFaceHandle hFontFace, rgb rgbText, rgba rgbaOverlay) {
	m_sText = "";
	m_hFontFace = hFontFace;
	m_rgbText = rgbText;
	m_rgbaOverlay = rgbaOverlay;

	auto sz = s_pTextRenderer->MeasureText(hFontFace, "gyXY", 1.0f);
	m_nyDrop = std::get<1>(sz);

	m_nWidth = nWidth;
	m_nHeight = nHeight;

	m_nXMargin = X_MARGIN;
	m_nYMargin = Y_MARGIN;
	if (-m_nyDrop > m_nYMargin) {
		m_nYMargin = -m_nyDrop;
	}
	m_yTextOffset = m_nYMargin;

	float remainingSpace = (float)(m_nHeight - std::get<3>(sz) - 2 * m_nYMargin);
	if (remainingSpace > 1.0f) {
		m_yTextOffset += (int)(remainingSpace / 2.0f);
	}
	SetText(sText);
}

OverlayTextLabel::OverlayTextLabel(std::string sText, int nMaxTextLength, FontFaceHandle hFontFace, rgb rgbText, rgba rgbaOverlay) {
	m_sText = "";
	m_hFontFace = hFontFace;
	m_rgbText = rgbText;
	m_rgbaOverlay = rgbaOverlay;

	std::string sMaxText;
	if (nMaxTextLength < sText.length()) {
		nMaxTextLength = static_cast<int>(sText.length());
	}
	if (nMaxTextLength<2) {
		sMaxText = "g";
	}
	else {
		sMaxText = std::string(nMaxTextLength / 2, 'g');
		nMaxTextLength -= nMaxTextLength / 2;
		sMaxText += std::string(nMaxTextLength , 'X');
	}
	
	auto sz = s_pTextRenderer->MeasureText(hFontFace, sMaxText, 1.0f);
	m_nyDrop = std::get<1>(sz);
	int width = std::get<2>(sz);
	int height = std::get<3>(sz);
	m_nWidth = width;
	m_nHeight = height;
	if (m_nyDrop < 0) {
		m_nHeight -= m_nyDrop;
	}
	m_nXMargin = X_MARGIN;
	m_nYMargin = Y_MARGIN;
	// Note here that m_nyDrop<0
	m_yTextOffset = - m_nyDrop + m_nYMargin;

	m_nWidth += 2 * m_nXMargin;
	m_nHeight += 2 * m_nYMargin;
	m_pTextLine = nullptr;
	SetText(sText);
}

OverlayTextLabel::~OverlayTextLabel() {
	delete m_pTextLine;
	m_pTextLine = nullptr;
}

void OverlayTextLabel::Render() {
	OverlayEntity::Render();
	s_pTextRenderer->RenderTextLine(m_pTextLine, (float)(m_nX + m_nXMargin),(float)(m_nY+ m_yTextOffset), m_rgbText);
}

void OverlayTextLabel::SetText(std::string sText) {
	if (m_sText != sText) {
		m_sText = sText;
		delete m_pTextLine;
		m_pTextLine = nullptr;
		m_pTextLine = s_pTextRenderer->CreateTextLine(m_hFontFace, m_sText, (float)m_nWidth, 1.0f);
	}
}
