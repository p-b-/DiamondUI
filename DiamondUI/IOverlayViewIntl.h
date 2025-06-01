#pragma once

class TouchLayer;
class IOverlayViewIntl {
public:
	~IOverlayViewIntl() {}
	virtual void Animate(float fDeltaTime) = 0;
	virtual void Render(float fXOrigin, float fYOrigin, float fWidth, float fHeight) = 0;
	virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin) = 0;
	virtual void Displaying(bool bDisplaying) = 0;
};