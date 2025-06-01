#pragma once
#include "IOverlayEnv.h"
#include "IOverlayEnvIntl.h"

class Textures;

class OverlayEnv :
    public IOverlayEnv, public IOverlayEnvIntl
{
public:
	OverlayEnv();
	virtual ~OverlayEnv() {}

	// IOverlayEnvIntl declarations
	virtual void Initialise(Textures* pTextureCtrl);
	virtual IOverlayEnv* GetPublicInterface() { return dynamic_cast<IOverlayEnv*>(this); }
	virtual const IOverlayEnv* GetConstPublicInterface() const { return dynamic_cast<const IOverlayEnv*>(this); }
	virtual rgba GetVectorColour(bool bHighlighted) const;
	virtual rgba GetDebugBackgroundRGBA(bool bMainBackground) const;
	virtual rgba GetOverlayBackgroundRGBA() const { return m_rgbaOverlay; }
	virtual FontFaceHandle GetTextFontFace() const { return m_hTextFace; }
	virtual FontFaceHandle GetTitleFontFace() const { return m_hTabTitleFace; }
	virtual rgba GetElementBackgroundRGBA() const { return m_rgbaElementBackground; }
	virtual rgba GetElementHighlightedBackgroundRGBA() const { return m_rgbaElementHighlightedBackground; }
	virtual rgb GetTextRGB() const { return m_rgbText; }
	virtual rgb GetDisabledTextRGB() const { return m_rgbDisabledText; }
	virtual rgba GetTabBackgroundRGBA() const { return m_rgbaTabBackground; }
	virtual rgb GetTitleHighlightedRGB() const { return m_rgbTabTitleHighlighted; }
	virtual rgb GetTitleRGB() const { return m_rgbTabTitle; }
	virtual rgba GetScrollbarColour() const { return m_rgbaScrollbar; }
	virtual float GetTextToElementRatio() const { return 0.25f; }

	virtual float GetControlVerticalMargin(bool bForTabNotView) const { return bForTabNotView ? 0.0f : m_fVertControlMargin; }
	virtual float GetControlHorizMargin(bool bForTabNotView) const { return bForTabNotView ? 0.0f : m_fHorizControlMargin; }
	virtual float GetInternalMarginForTabOrView(bool bForTabNotView) const { return bForTabNotView ? m_fTabMargin : m_fViewMargin; }
	virtual float GetPreferredControlWidth() const { return m_fControlWidth; }
	virtual float GetPreferredBarWidthForControlType(OverlayControlType eControlType, int nControlCount, int nControlSpacing) const;
	virtual float GetTabMargin() const { return m_fTabMargin; }
	virtual float GetViewMargin() const { return m_fViewMargin; }
	virtual float GetHorizInterElementSpacing() const { return m_fHorizInterElementSpacing; }
	virtual float GetVerticalInterElementSpacing(bool bForTabNotView) const { return bForTabNotView?m_fVerticalInterElementSpacing_ForTab: m_fVerticalInterElementSpacing_ForView; }

	virtual int GetOkAction() const { return m_nOkAction; }
	virtual int GetCloseAction() const { return m_nCloseAction; }
	virtual int GetOutsideOverlayAction() const { return m_nOutsideOverlayAction; }
	virtual int GetMoveLeftAction() const { return m_nMoveLeftAction; }
	virtual int GetMoveRightAction() const { return m_nMoveRightAction; }
	virtual int GetMoveUpAction() const { return m_nMoveUpAction; }
	virtual int GetMoveDownAction() const { return m_nMoveDownAction; }
	virtual int GetInternalMoveUpAction() const { return m_nInternalMoveUpAction;  }
	virtual int GetInternalMoveDownAction() const { return m_nInternalMoveDownAction;}
	virtual int GetNextTabAction() const { return m_nNextTabAction; }
	virtual int GetPrevTabAction() const { return m_nPrevTabAction; }
	virtual int GetHighlightElementAction() const { return m_nHighlightElementAction; }
	virtual int GetChooseTabAction() const { return m_nChooseTabAction; }
	virtual int GetInternalActionStart() const { return s_nInternalActionStart; }
	virtual int GetApplyTabAction() const { return m_nApplyTabAction; }

	virtual void SetInternalMoveDownAction(int nAction) { m_nInternalMoveDownAction = nAction; }
	virtual void SetInternalMoveUpAction(int nAction) { m_nInternalMoveUpAction = nAction; }


	// IOverlayEnv declarations
	virtual void SetPreferredControlWidth(int nWidth) ;
	virtual void SetGlyphTextureAtlasId(unsigned int unGlyphAtlasId);
	virtual void SetMoveActions(unsigned int unLeft, unsigned int unRight, unsigned int unUp, unsigned int unDown);
	virtual void SetHighlightElementAction(unsigned int unHighlight);
	virtual void SetTabActions(unsigned int unNextTab, unsigned int unPrevTab, unsigned int unChooseTab);
	virtual void SetOkCloseActions(unsigned int unOk, unsigned int unClose);
	virtual void SetOutsideOverlayAction(unsigned int unOutsideOverlay);
	virtual void SetScrollbarColour(rgba rgbaBarColour);
	virtual void SetTabColours(rgb rgbTitle, rgb rgbTitleHighlighted, rgba rgbaBackground);
	virtual void SetTextColour(rgb rgbText);
	virtual void SetDisabledTextColour(rgb rgbDisabledText);
	virtual void SetElementBackgroundColour(rgba rgbaElementBackground, rgba rgbaElementHighlightedBackground);
	virtual void SetOverlayBackgroundColour(rgba rgbaOverlay);
	virtual void SetTabTitleFont(FontFaceHandle hTabTitleFace);
	virtual void SetTextFont(FontFaceHandle hTextFace);

	// Static method declarations
	static void SetInternalActionStartIndex(int nIndex);

private:
	void SetDefaultColours();

private:
	float m_fControlWidth;

	rgba m_rgbaOverlay;
	rgb m_rgbTabTitle;
	rgb m_rgbTabTitleHighlighted;
	rgba m_rgbaTabBackground;
	rgb m_rgbText;
	rgb m_rgbDisabledText;
	rgba m_rgbaElementBackground;
	rgba m_rgbaElementHighlightedBackground;
	rgba m_rgbaScrollbar;
	rgba m_rgbaHighlightedVector;
	rgba m_rgbaVector;
	FontFaceHandle m_hTabTitleFace;
	FontFaceHandle m_hTextFace;

	float m_fViewMargin;
	float m_fTabMargin;
	float m_fHorizControlMargin; 
	float m_fVertControlMargin;
	float m_fHorizInterElementSpacing;

	float m_fVerticalInterElementSpacing_ForView;
	float m_fVerticalInterElementSpacing_ForTab;

	int m_nOkAction;
	int m_nCloseAction;
	int m_nMoveLeftAction;
	int m_nMoveRightAction;
	int m_nMoveUpAction;
	int m_nMoveDownAction;
	int m_nInternalMoveUpAction;
	int m_nInternalMoveDownAction;
	int m_nNextTabAction;
	int m_nPrevTabAction;
	int m_nHighlightElementAction;
	int m_nChooseTabAction;
	int m_nOutsideOverlayAction;
	int m_nApplyTabAction;

	static int s_nInternalActionStart;
};


