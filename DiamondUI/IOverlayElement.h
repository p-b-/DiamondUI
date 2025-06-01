#pragma once

class TouchLayer;
class ITouchTarget;

class IOverlayElement {
public:
	virtual ~IOverlayElement() {}
	virtual void Deinitialise() = 0;
	virtual void FreeSpace() = 0;
	virtual int GetId() const = 0;
	virtual void InitialiseForWidth(float fWidth) = 0;
	virtual float GetHeight() const = 0;
	virtual float GetPreferredControlWidth() const = 0;
	virtual void SetPreferredControlWidth(float fWidth) = 0;
	virtual float GetLastRenderedYOrigin() const = 0;
	virtual bool RenderedYet() const = 0;
	virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight) = 0;
	virtual bool Disabled() = 0;
	virtual void Disable(bool bDisable) = 0;
	virtual bool CanHighlight() = 0;
	virtual void Highlight(bool bHighlight) = 0;
	virtual float GetScrollIncrement() = 0;
	virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) = 0;
	virtual OverlayControlType GetControlType() const = 0;
	virtual void ShowDebugBackground(bool bShow) = 0;
	virtual ActionResult Action(unsigned int unAction) = 0;
	virtual bool Altered() const = 0;
	virtual bool CanSave() const = 0;
	virtual void Save() = 0;
	virtual void Revert() = 0;
	virtual void ResetDisplayState() = 0;
};