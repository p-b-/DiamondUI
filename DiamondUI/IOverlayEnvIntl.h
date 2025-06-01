#pragma once

#include "ITextRenderer.h" // For FontFaceHandle
#include "OverlayDefs.h"
#include "ColourDefs.h"
#include "IOverlayEnv.h"

class Textures;

class IOverlayEnvIntl {
public:
	~IOverlayEnvIntl() { }

	virtual void Initialise(Textures* pTextureCtrl) = 0;
	virtual IOverlayEnv* GetPublicInterface() = 0;
	virtual const IOverlayEnv* GetConstPublicInterface() const = 0;
	virtual float GetTextToElementRatio() const = 0;

	virtual float GetControlVerticalMargin(bool bForTabNotView) const = 0;
	// Margin inside a control that is in a horizontal view
	virtual float GetControlHorizMargin(bool bForTabNotView) const = 0;
	virtual float GetInternalMarginForTabOrView(bool bForTabNotView) const = 0;
	virtual float GetPreferredControlWidth() const = 0;
	virtual float GetPreferredBarWidthForControlType(OverlayControlType eControlType, int nControlCount, int nControlSpacing) const = 0;
	virtual float GetTabMargin() const = 0;
	virtual float GetViewMargin() const = 0;
	// Spacing between adjacent horizontal elements
	virtual float GetHorizInterElementSpacing() const = 0;
	virtual float GetVerticalInterElementSpacing(bool bForTabNotView) const = 0;

	virtual int GetOkAction() const = 0;
	virtual int GetCloseAction() const = 0;
	virtual int GetOutsideOverlayAction() const = 0;
	virtual int GetMoveLeftAction() const = 0;
	virtual int GetMoveRightAction() const = 0;
	virtual int GetMoveUpAction() const = 0;
	virtual int GetMoveDownAction() const = 0;
	virtual int GetInternalMoveUpAction() const = 0;
	virtual int GetInternalMoveDownAction() const = 0;
	virtual int GetNextTabAction() const = 0;
	virtual int GetPrevTabAction() const = 0;
	virtual int GetHighlightElementAction() const = 0;
	virtual int GetChooseTabAction() const = 0;
	virtual int GetInternalActionStart() const = 0;
	virtual int GetApplyTabAction() const = 0;

	virtual void SetInternalMoveDownAction(int nAction) = 0;
	virtual void SetInternalMoveUpAction(int nAction) = 0;
};