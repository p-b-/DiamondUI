#pragma once
#include "OverlayEntity.h"
#include "ITextRenderer.h"
#include "ColourDefs.h"
#include <string>
#include <tuple>

class OverlayTextLabel : public OverlayEntity
{
public:
	OverlayTextLabel(std::string sText, int nWidth, int nHeight, FontFaceHandle hFontFace, rgb rgbText, rgba rgbaOverlay);

	OverlayTextLabel(std::string sText, int nMaxTextLength, FontFaceHandle hFontFace, rgb rgbText, rgba rgbaOverlay);
	virtual ~OverlayTextLabel();
	// Prevent copy and moving this class - the destructor will deinitialise the textLine object containing the vertex array and buffer.
	OverlayTextLabel(const OverlayTextLabel& toCopy) = delete;
	OverlayTextLabel(OverlayTextLabel&& toMove) = delete;


	virtual void Render();
	virtual void SetText(std::string sText);
	
private:
	int m_nyDrop; // How far the tails of 'g's and 'y's drop below the drawing origin. Need to shift text rendering origin up by this amount
	std::string m_sText;
	FontFaceHandle m_hFontFace;
	rgb m_rgbText;

	ITextLine* m_pTextLine;

	int m_nXMargin;
	int m_nYMargin;
	int m_yTextOffset;
};

