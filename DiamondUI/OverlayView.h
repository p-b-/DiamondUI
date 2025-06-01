#pragma once
#include "IOverlayViewIntl.h"
#include "IOverlayView.h"
#include "ITouchTarget.h"
#include "OverlayDefsInt.h"
#include "OverlayGraphics.h"
#include "ElementCollection.h"

#include <map>
#include <vector>

class IOverlayIntl;
class IOverlayElement;
class IOverlayEnvIntl;
class ScrollbarElement;
class TouchLayer;

class OverlayView : 
	public OverlayGraphics, public IOverlayViewIntl, public ITouchTarget, public IOverlayView, public ScrollTarget, public IElementCollectionContainer
{
public:
	OverlayView(IOverlayIntl* pOverlay, float fXOrigin, float fYOrigin, float fWidth, float fHeight, bool bVerticalNotHoriz);
	virtual ~OverlayView();

	// IOverlayView declarations
public:
	virtual int AddTextLabel(std::string sLabel, float fWidth = 0.0f);
	virtual int AddTextField(std::string sText, float fWidth = 0.0f);

	// IOverlayViewIntl declarations
public:
	virtual void Animate(float fDeltaTime);
	virtual void Render(float fXOrigin, float fYOrigin, float fWidth, float fHeight);
	virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin);
	virtual void Displaying(bool bDisplaying);

	// ITouchTarget Declarations
public:
	virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
	virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

	// ScrollTarget declarations
public:
	virtual void SetVerticalScrollOffset(float yOffset);

	// IElementCollectionContainer declarations
public:
	virtual float GetDisplayableHeight() { return GetDisplayableViewHeight(); }
	virtual float GetDisplayableWidth() { return GetDisplayableViewWidth(); }
	virtual bool GetCentreELementsHorizontally() const { return false; }
	virtual IOverlayIntl* GetOverlay() { return m_pOverlay; }
	virtual const IOverlayEnvIntl* GetEnvironment() const { return m_pEnvironment; }
	virtual bool GetContainerIsTabNotView() { return false; }
	virtual bool GetContainerAspectIsVertical() { return m_bVerticalNotHoriz; }
	
protected:
	float GetDisplayableViewWidth();
	float GetDisplayableViewHeight();
	void UpdateScrollableContentHeight();
	void ConsiderDisplayingScrollbars();
	float CalculateTotalHeight() const;
	ActionResult Action(unsigned int unAction);

private:
	IOverlayIntl* m_pOverlay;
	const IOverlayEnvIntl* m_pEnvironment;
	ElementCollection m_elements;
	std::vector<IOverlayElement* > m_vcElements;
	std::map<int, IOverlayElement* > m_mpIdToElement;

	float m_fXOrigin;
	float m_fYOrigin;
	float m_fWidth;
	float m_fHeight;
	float m_fTitleBackgroundHeight;
	bool m_bVerticalNotHoriz;

	bool m_bDisplayScrollbar;
	ScrollbarElement* m_pScrollbar;
	float m_fContentHeight;
	float m_yScrollOffset;
};

