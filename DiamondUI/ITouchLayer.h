#pragma once
#include "InputDefs.h"

class ITouchTarget;
class ITouchLayer {
public:
	virtual ~ITouchLayer() {}
	virtual void SetLayerActive(bool bActive) = 0;
	virtual bool GetLayerActive() const = 0;
	virtual void AddToInputEngine() = 0;
	virtual void RemoveFromInputEngine() = 0;
	virtual void SetYScrollDelta(float yScrollDelta) = 0;
	virtual void SetXScrollDelta(float xScrollDelta) = 0;
	virtual unsigned int AddTouchArea(float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nTouchValue) = 0;
	virtual void SetScrollingBehaviour(int unId, bool bScrollsVertically, bool bScrollsHorizontally) = 0;
	virtual void SetTouchTargetNeedsContinuousUpdates(int unId, bool bContinuousUpdates) = 0;
	virtual void SetTouchAreaActive(unsigned int unId, bool bActive) = 0;
	virtual Touch GetTouchTarget(int x, int y) = 0;
	virtual void MoveLayer(float fXOffset, float fYOffset) = 0;
};