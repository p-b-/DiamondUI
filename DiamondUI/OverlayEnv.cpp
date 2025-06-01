#include "OverlayEnv.h"
#include "OverlayGraphics.h"

int OverlayEnv::s_nInternalActionStart;

OverlayEnv::OverlayEnv() :
	m_nCloseAction(-1), m_nMoveDownAction(-1), m_nMoveLeftAction(-1),
	m_nMoveRightAction(-1), m_nMoveUpAction(-1), m_nNextTabAction(-1), m_nOkAction(-1), m_nChooseTabAction(-1), m_nHighlightElementAction(-1),
	m_nInternalMoveUpAction(-1), m_nInternalMoveDownAction(-1),
	m_nPrevTabAction(-1), m_nOutsideOverlayAction(-1) {
	m_fControlWidth = 250.0f;
	m_nApplyTabAction = s_nInternalActionStart + 0;
}

// IOverlayEnvIntl implementation
//
rgba OverlayEnv::GetVectorColour(bool bHighlighted) const {
	if (bHighlighted) {
		return m_rgbaHighlightedVector;
	}
	else {
		return m_rgbaVector;
	}
}

rgba OverlayEnv::GetDebugBackgroundRGBA(bool bMainBackground) const {
	// TODO Make this colour configurable
	if (bMainBackground) {
		return MakeRGBA(0.1f, 0.8f, 0.5f, 0.4f);
	}
	else {
		return MakeRGBA(0.8f, 0.1f, 0.1f, 0.4f);
	}
}

float OverlayEnv::GetPreferredBarWidthForControlType(OverlayControlType eControlType, int nControlCount, int nControlSpacing) const {
	if (nControlCount == 0) {
		return 50.0f;
	}
	switch (eControlType) {
	case eOCTButton:
		// TODO Determine this by looking at what the text in the buttons, or at least expected text.
		return (GetPreferredControlWidth() - (nControlCount - 1) * nControlSpacing) / nControlCount;
	}
	return GetPreferredControlWidth();
}
//
// IOverlayEnvIntl implementation

void OverlayEnv::SetPreferredControlWidth(int nWidth) {
	m_fControlWidth = static_cast<float>(nWidth);
}

void OverlayEnv::SetGlyphTextureAtlasId(unsigned int unGlyphAtlasId) {
	OverlayGraphics::SetGlyphAtlasId(unGlyphAtlasId);
}

void OverlayEnv::SetMoveActions(unsigned int unLeft, unsigned int unRight, unsigned int unUp, unsigned int unDown) {
	m_nMoveLeftAction = unLeft;
	m_nMoveRightAction = unRight;
	m_nMoveUpAction = unUp;
	m_nMoveDownAction = unDown;
}

void OverlayEnv::SetHighlightElementAction(unsigned int unHighlight) {
	m_nHighlightElementAction = unHighlight;
}

void OverlayEnv::SetTabActions(unsigned int unNextTab, unsigned int unPrevTab, unsigned int unChooseTab) {
	m_nNextTabAction = unNextTab;
	m_nPrevTabAction = unPrevTab;
	m_nChooseTabAction = unChooseTab;
}

void OverlayEnv::SetOkCloseActions(unsigned int unOk, unsigned int unClose) {
	m_nOkAction = unOk;
	m_nCloseAction = unClose;
}

void OverlayEnv::SetOutsideOverlayAction(unsigned int unOutsideOverlay) {
	m_nOutsideOverlayAction = unOutsideOverlay;
}

void OverlayEnv::SetScrollbarColour(rgba rgbaBarColour) {
	m_rgbaScrollbar = rgbaBarColour;
}

void OverlayEnv::SetTabColours(rgb rgbTitle, rgb rgbTitleHighlighted, rgba rgbaBackground) {
	m_rgbTabTitle = rgbTitle;
	m_rgbTabTitleHighlighted = rgbTitleHighlighted;
	m_rgbaTabBackground = rgbaBackground;
}

void OverlayEnv::SetTextColour(rgb rgbText) {
	m_rgbText = rgbText;
	// No need to ask tabs to update - the text colour will update on next frame
}

void OverlayEnv::SetDisabledTextColour(rgb rgbDisabledText) {
	m_rgbDisabledText = rgbDisabledText;
}

void OverlayEnv::SetElementBackgroundColour(rgba rgbaElementBackground, rgba rgbaElementHighlightedBackground) {
	m_rgbaElementBackground = rgbaElementBackground;
	m_rgbaElementHighlightedBackground = rgbaElementHighlightedBackground;
}

void OverlayEnv::SetOverlayBackgroundColour(rgba rgbaOverlay) {
	m_rgbaOverlay = rgbaOverlay;
}

void OverlayEnv::SetTabTitleFont(FontFaceHandle hTabTitleFace) {
	m_hTabTitleFace = hTabTitleFace;
	/*for (auto t : m_vcTabs) {
		t->TitleFontUpdated();
	}*/
}

void OverlayEnv::SetTextFont(FontFaceHandle hTextFace) {
	m_hTextFace = hTextFace;
	/*for (auto t : m_vcTabs) {
		t->TextFontUpdated();
	}*/
}

void OverlayEnv::Initialise(Textures* pTextureCtrl) {
	OverlayGraphics::SetTextureCtrl(pTextureCtrl);
	m_fTabMargin = 10.0f;
	m_fViewMargin = 0.0f;
	m_fHorizControlMargin = 5.0f;
	m_fVertControlMargin = 5.0f;
	m_fHorizInterElementSpacing = 5.0f;
	m_fVerticalInterElementSpacing_ForView = 10.0f;
	m_fVerticalInterElementSpacing_ForTab = 2.0f;
	SetDefaultColours();
}

void OverlayEnv::SetDefaultColours() {
	SetTextColour(std::make_tuple(1.0f, 1.0f, 1.0f));
	SetDisabledTextColour(std::make_tuple(0.5f, 0.5f, 0.5f));

	auto rgbaOverlay = std::make_tuple(0.0f, 0.0f, 0.2f, 0.8f);
	auto rgbaHighlighted = std::make_tuple(0.0f, 0.0f, 1.0f, 1.0f);

	auto rgbaTabBackground = std::make_tuple(0.0f, 0.0f, 0.4f, 0.8f);

	// TODO Make this colour configurable
	m_rgbaHighlightedVector = MakeRGBA(0.3f, 0.3f, 1.0f, 1.0f);
	// TODO Make this colour configurable
	m_rgbaVector = MakeRGBA(0.0f, 0.0f, 0.8f, 0.8f);

	SetScrollbarColour(MakeRGBA(0.4f, 0.4f, 0.8f, 0.5f));
	SetTabColours(std::make_tuple(0.6f, 0.6f, 0.6f), std::make_tuple(1.0f, 1.0f, 1.0f), rgbaOverlay);
	SetElementBackgroundColour(std::make_tuple(0.0f, 0.0f, 0.4f, 0.8f), rgbaHighlighted);
}

// Static class methods implementation
//
void OverlayEnv::SetInternalActionStartIndex(int nIndex) {
	s_nInternalActionStart = nIndex;
}
//
// Static class methods implementation
