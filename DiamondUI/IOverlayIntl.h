#pragma once
#include "InputDefs.h"

class ITouchTarget;
class IGameInput;
class ITouchLayer;
class ITextInputTarget;
class IOverlayEnvIntl;

class IOverlayIntl
{
public:
	virtual ~IOverlayIntl() {}

	virtual const IOverlayEnvIntl* GetInternalOverlayEnvironment() const = 0;

	virtual unsigned int AddTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, bool bNoScroll, int nInvokeValue) = 0;
	virtual unsigned int AddVerticallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue) = 0;
	virtual unsigned int AddHorizontallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue) = 0;
	virtual void SetTouchAreaToNeedContinuousUpdates(ITouchLayer* pTL, unsigned int unId, bool bContinuousUpdates) = 0;

	virtual bool ApplyTabPressed() = 0;
	virtual bool ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTarget, const IntRect& rcTargetArea) = 0;

	virtual int NextElementId() = 0;
};

