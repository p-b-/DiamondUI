#pragma once

#include "ITextRenderer.h" // For FontFaceHandle
#include "OverlayDefs.h"
#include "ColourDefs.h"

class IOverlayEnv {
public:
	virtual ~IOverlayEnv() {}

	virtual void SetPreferredControlWidth(int nWidth) = 0;
	virtual void SetGlyphTextureAtlasId(unsigned int unGlyphAtlasId) = 0;
	virtual void SetMoveActions(unsigned int unLeft, unsigned int unRight, unsigned int unUp, unsigned int unDown) = 0;
	virtual void SetHighlightElementAction(unsigned int unHighlight) = 0;
	virtual void SetTabActions(unsigned int unNextTab, unsigned int unPrevTab, unsigned int unChooseTab) = 0;
	virtual void SetOkCloseActions(unsigned int unOk, unsigned int unClose) = 0;
	virtual void SetOutsideOverlayAction(unsigned int unOutsideOverlay) = 0;
	virtual void SetScrollbarColour(rgba rgbaBarColour) = 0;
	virtual void SetTabColours(rgb rgbTitle, rgb rgbTitleHighlighted, rgba rgbaBackground) = 0;
	virtual void SetTextColour(rgb rgbText) = 0;
	virtual void SetDisabledTextColour(rgb rgbDisabledText) = 0;
	virtual void SetElementBackgroundColour(rgba rgbaElementBackground, rgba rgbaElementHighlightedBackground) = 0;
	virtual void SetOverlayBackgroundColour(rgba rgbaOverlay) = 0;
	virtual void SetTabTitleFont(FontFaceHandle hTabTitleFace) = 0;
	virtual void SetTextFont(FontFaceHandle hTextFace) = 0;

	virtual FontFaceHandle GetTextFontFace() const = 0;
	virtual FontFaceHandle GetTitleFontFace() const = 0;

	virtual rgba GetOverlayBackgroundRGBA() const = 0;
	virtual rgba GetVectorColour(bool bHighlighted) const = 0;
	virtual rgba GetDebugBackgroundRGBA(bool bMainBackground) const = 0;
	virtual rgba GetElementBackgroundRGBA() const = 0;
	virtual rgba GetElementHighlightedBackgroundRGBA() const = 0;
	virtual rgb GetTextRGB() const = 0;
	virtual rgb GetDisabledTextRGB() const = 0;
	virtual rgba GetTabBackgroundRGBA() const = 0;
	virtual rgb GetTitleHighlightedRGB() const = 0;
	virtual rgb GetTitleRGB() const = 0;
	virtual rgba GetScrollbarColour() const = 0;
};