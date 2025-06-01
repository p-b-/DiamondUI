#pragma once

// TODO Add 'l' to end of this filename
#include "OverlayDefs.h"
#include "OverlayDefsInt.h"
#include "ITouchTarget.h"
#include "OverlayGraphics.h"

#include <vector>
#include <map>

class IElementCollectionContainer;
class IOverlayElement;
class ScrollbarElement;
class TouchLayer;

class ElementCollection : 
	public ScrollTarget, public OverlayGraphics, public ITouchTarget
{
public:
	ElementCollection(IElementCollectionContainer* pContainer);
	~ElementCollection();

	void ResetDisplayState();
	void Displaying(bool bDisplay);

	// ScrollTarget declarations
	void SetVerticalScrollOffset(float fScrollOffset);

	// ITouchTarget Declarations
public:
	virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
	virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

	void Render(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight);
	void Animate(float fDeltaTime);
	void AddElement(IOverlayElement* pElement);
	void AddElementToMap(IOverlayElement* pElement);
	IOverlayElement* GetElement(int nElementId);
	ActionResult MoveUp();
	ActionResult MoveUp_NoHighlights();
	ActionResult MoveDown();

	bool HighlightFirstElement();
	bool HighlightLastElement();
	bool HighlightElement(IOverlayElement* pElement, int nIndex);
	bool HighlightElementWithIndex(int nIndex);
	bool GetIsValidIndex(int nIndex) const;
	bool HasHighlightedElement() const { return m_pHighlightedElement != nullptr; }
	void EnsureHighlightedElementFullyDisplayed();
	ActionResult Action(unsigned int unAction);
	bool IsElementIndexHighlighted(int nIndex) const { return nIndex == m_nHighlightedIndex; }

	void ScrollByDelta(float nDelta);
	bool DisplayingScrollbar() const { return m_bDisplayScrollbar; }
	ScrollbarElement* GetScrollbar() const { return m_pScrollbar; }
	void BackdropTouched(int x, int y, bool bPressed);

	void ControlResized();
	void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, float fWidth, float fHeight);

	bool DataAltered() const;
	void RevertData();
	bool SaveData();
	bool CanClose() const;

	void EnableElement(int nElementId, bool bEnabled);
	void ShowDebugBackgroundOnControlType(OverlayControlType eType, bool bShow);

protected:
	void RenderElements(float xOrigin, float yOrigin, float fWidth, float fHeight);
	void EnsureElementFullyDisplayed(IOverlayElement* pElement);
	float CalcScrollDeltaToElementFullyDisplayed(IOverlayElement* pElement);
	int FindFirstDisplayedElementIndex();
	void ConsiderDisplayingScrollbars();
	void UpdateScrollableContentHeight();
	float CalculateTotalHeight() const;

private:
	IElementCollectionContainer* m_pContainer;
	std::vector<IOverlayElement* > m_vcElements;
	std::map<int, IOverlayElement* > m_mpIdToElement;

	bool m_bDisplayed;
	ScrollbarElement* m_pScrollbar;
	float m_fContentHeight;
	bool m_bDisplayScrollbar;
	float m_yScrollOffset;
	TouchLayer* m_pTouchLayer;

	// Animation
	float m_fFrameRate;
	float m_fPixelsPerSecond;

	float m_fScrollOffsetAnimateTarget;
	float m_fScrollOffsetAnimateCurrent;
	float m_fScrollAnimationStepPerSec;
	bool m_bScrollAnimating;
	bool m_bBackdropScrolling;
	int m_yBackdropScrollStart;

	IOverlayElement* m_pHighlightedElement;
	int m_nHighlightedIndex;

	float m_yLastRenderedOrigin;
};

