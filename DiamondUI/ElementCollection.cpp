#include "ElementCollection.h"
#include "IOverlayElement.h"
#include "ScrollbarElement.h"
#include "IOverlayIntl.h"
#include "IOverlayEnvIntl.h"
#include "TouchLayer.h"

// Space around displayable area within tab
#define TAB_MARGIN 5
#define TABBACKDROP_SCROLLTARGETVALUE 0x10001

ElementCollection::ElementCollection(IElementCollectionContainer* pContainer) {
	m_pContainer = pContainer;
	m_pScrollbar = nullptr;
	m_fContentHeight = 0.0f;
	m_bDisplayScrollbar = false;

	m_yLastRenderedOrigin = 0;
	m_yScrollOffset = 0.0f;
	m_bDisplayed = false;

	m_fFrameRate = 144.0f;
	m_fPixelsPerSecond = 4 * 288.0f;

	m_bScrollAnimating = false;
	m_fScrollAnimationStepPerSec = 0.0f;
	m_fScrollOffsetAnimateTarget = 0.0f;
	m_fScrollOffsetAnimateCurrent = 0.0;
	m_bBackdropScrolling = false;
	m_yBackdropScrollStart = 0;

	m_pHighlightedElement = nullptr;
	m_nHighlightedIndex = -1;

	m_pTouchLayer = nullptr;
}

ElementCollection::~ElementCollection() {
	delete m_pTouchLayer;
	m_pTouchLayer = nullptr;

	for (auto e : m_vcElements) {
		delete e;
	}
	m_vcElements.clear();

	if (m_pScrollbar != nullptr) {
		delete m_pScrollbar;
		m_pScrollbar = nullptr;
	}
}

void ElementCollection::ResetDisplayState() {
	m_bBackdropScrolling = false;
	for (auto e : m_vcElements) {
		e->Revert();
		e->ResetDisplayState();
	}
	if (m_pScrollbar != nullptr) {
		m_pScrollbar->ResetDisplayState();
	}
	m_nHighlightedIndex = -1;
	m_pHighlightedElement = nullptr;
	if (!HighlightFirstElement()) {
		// No highlightable elements - ensure the element at the start is scrolled to the top
		m_yScrollOffset = 0.0f;
		if (m_pScrollbar != nullptr) {
			m_pScrollbar->SetYOffset(-m_yScrollOffset);
		}
	}
}

void ElementCollection::Displaying(bool bDisplay) {
	m_bDisplayed = bDisplay;
	if (bDisplay) {
		m_bBackdropScrolling = false;
		if (m_pHighlightedElement == nullptr) {
			HighlightFirstElement();
		}
		if (m_pTouchLayer != nullptr) {
			m_pTouchLayer->SetLayerActive(true);
			m_pTouchLayer->AddToInputEngine();
		}
	}
	else {
		if (m_pHighlightedElement != nullptr) {
			m_pHighlightedElement->Highlight(false);
			m_nHighlightedIndex = -1;
			m_pHighlightedElement = nullptr;
		}
		if (m_pTouchLayer != nullptr) {
			m_pTouchLayer->RemoveFromInputEngine();
			m_pTouchLayer->SetLayerActive(false);
		}
	}
}

// ScrollTarget implementation
//
void ElementCollection::SetVerticalScrollOffset(float fScrollOffset) {
	m_yScrollOffset = fScrollOffset;
	m_pTouchLayer->SetYScrollDelta(m_yScrollOffset);
}
//
// ScrollTarget implementation

// ITouchTarget implementation
//
bool ElementCollection::Invoke(unsigned int unAction, int nValue, float fDeltaTime) { 
	if (nValue >= 0 && nValue < m_vcElements.size()) {
		if (IsElementIndexHighlighted(nValue) == false) {
			// Clicking another ctrl to the one that is highlighted. Currently the code here or
			//  the code in HighlightElement does not check if the currently highlighted element can give up focus
			if (!HighlightElementWithIndex(nValue)) {
				// Could be disabled, or otherwise unable to highlight
				return false;
			}
		}
		return Action(unAction).m_bSuccess;
	}
	else if (nValue == ScrollbarElement::GetInvokeValue() && m_pScrollbar!=nullptr) {
		if (unAction == m_pScrollbar->GetScrollUpAction()) {
			MoveUp();
		}
		else if (unAction == m_pScrollbar->GetScrollDownAction()) {
			MoveDown();
		}
	}
	return false;
}

bool ElementCollection::TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) { 
	if (GetIsValidIndex(nValue)) {
		// Finishing the clicking on another ctrl to this.
		// Only attempt to highlight if tab is currently displayed. Highlighting a text input field will attempt to display the
		//  keyboard, and we do not want that if the tab is not displayewd. (touchupdate can be sent slightly after tab is removed)
		if (m_bDisplayed && !bPressed && !HighlightElementWithIndex(nValue)) {
			// Could be disable, or otherwise unable to highlight
			return false;
		}
		ITouchTarget* pElement = dynamic_cast<ITouchTarget*>(m_vcElements[nValue]);

		return pElement->TouchUpdate(unAction, nValue, bPressed, bOverTarget, x, y);
	}
	else if (nValue == TABBACKDROP_SCROLLTARGETVALUE && DisplayingScrollbar()) {
		BackdropTouched(x, y, bPressed);
		return true;
	}
	return false;
}
//
// ITouchTarget implementation

void ElementCollection::Render(float xDrawPortalOrigin, float yDrawPortalOrigin, float fDrawPortalWidth, float fDrawPortalHeight) {
	m_yLastRenderedOrigin = yDrawPortalOrigin;
	const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();
	const IOverlayEnv* pPublicEnv = pEnv->GetConstPublicInterface();

	float xOrigin = xDrawPortalOrigin;
	float yOrigin = yDrawPortalOrigin;
	float fWidth = fDrawPortalWidth;
	if (m_bDisplayScrollbar) {
		fWidth -= ScrollbarElement::GetWidth();
	}
	SetClippingArea(xOrigin, yOrigin, fWidth, fDrawPortalHeight);
	bool bIsTabNotView = m_pContainer->GetContainerIsTabNotView();
	if (bIsTabNotView) {
		rgba rgbaBackground = pPublicEnv->GetTabBackgroundRGBA();
		RenderTranslucentOverlay(xOrigin, yOrigin, fWidth, fDrawPortalHeight, rgbaBackground);
	}

	float fInternalMargin = pEnv->GetInternalMarginForTabOrView(bIsTabNotView);

	SetClippingArea(xOrigin, yOrigin + fInternalMargin, fWidth, fDrawPortalHeight - 2 * fInternalMargin);
	RenderElements(xDrawPortalOrigin, yDrawPortalOrigin, fWidth, fDrawPortalHeight);

	RemoveClippingArea();
	RemoveClippingArea();
	if (m_bDisplayScrollbar) {
		m_pScrollbar->Render(xDrawPortalOrigin + fDrawPortalWidth - ScrollbarElement::GetWidth(), yDrawPortalOrigin + TAB_MARGIN, ScrollbarElement::GetWidth(), m_pContainer->GetDisplayableHeight());
	}
}

void ElementCollection::RenderElements(float xOrigin, float yOrigin, float fWidth, float fHeight) {
	bool bIsTabNotView = m_pContainer->GetContainerIsTabNotView();

	const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();
	const IOverlayEnv* pPublicEnv = pEnv->GetConstPublicInterface();

	float fInternalMargin = pEnv->GetInternalMarginForTabOrView(bIsTabNotView);
	float xElementOrigin = xOrigin;
	float width = fWidth;

	float fHorizControlMargin = pEnv->GetControlHorizMargin(bIsTabNotView);
	float fVertControlMargin = pEnv->GetControlVerticalMargin(bIsTabNotView);

	float topY = yOrigin + fHeight + m_yScrollOffset;
	float verticalSpacing = pEnv->GetVerticalInterElementSpacing(bIsTabNotView);
	float horizSpacing = pEnv->GetHorizInterElementSpacing();
	bool bReevaluateWidthPerElement = !m_pContainer->GetContainerIsTabNotView();
	bool bRenderVertically = m_pContainer->GetContainerAspectIsVertical();
	yOrigin += fInternalMargin;

	for (auto e : m_vcElements) {
		float elementHeight = e->GetHeight() + 2 * fVertControlMargin;
		float yElementOrigin = topY - elementHeight;

		if (bReevaluateWidthPerElement) {
			width = e->GetPreferredControlWidth() + 2 * fHorizControlMargin;
		}
		RenderTranslucentOverlay(xElementOrigin, yElementOrigin, width, elementHeight, pPublicEnv->GetOverlayBackgroundRGBA());
		SetClippingArea(xElementOrigin + fHorizControlMargin, yElementOrigin + fVertControlMargin, width - 2 * fHorizControlMargin, elementHeight - 2 * fVertControlMargin);
		e->Render(xElementOrigin + fHorizControlMargin, yElementOrigin + fVertControlMargin, width - 2 * fHorizControlMargin, elementHeight - 2 * fVertControlMargin);

		RemoveClippingArea();
		if (bRenderVertically) {
			topY = static_cast<float>(yElementOrigin - verticalSpacing);
		}
		else {
			xElementOrigin += width + horizSpacing;
		}
	}
}

void ElementCollection::Animate(float fDeltaTime) {
	if (m_bScrollAnimating) {
		m_fScrollOffsetAnimateCurrent += m_fScrollAnimationStepPerSec * fDeltaTime;

		// Has animation overshot?
		if ((m_fScrollAnimationStepPerSec < 0.0f && m_fScrollOffsetAnimateCurrent < m_fScrollOffsetAnimateTarget) ||
			(m_fScrollAnimationStepPerSec > 0.0f && m_fScrollOffsetAnimateCurrent > m_fScrollOffsetAnimateTarget)) {
			m_fScrollOffsetAnimateCurrent = m_fScrollOffsetAnimateTarget;
		}
		m_yScrollOffset = m_fScrollOffsetAnimateCurrent;
		if (m_pScrollbar != nullptr) {
			m_pScrollbar->SetYOffset(-m_yScrollOffset);
		}
		if (m_pTouchLayer != nullptr) {
			m_pTouchLayer->SetYScrollDelta(m_yScrollOffset);
		}

		if (fabs(m_fScrollOffsetAnimateTarget - m_fScrollOffsetAnimateCurrent) < 0.1f) {
			m_yScrollOffset = m_fScrollOffsetAnimateTarget;
			m_bScrollAnimating = false;
		}
	}
}

void ElementCollection::AddElement(IOverlayElement* pElement) {
	m_vcElements.push_back(pElement);
	m_mpIdToElement[pElement->GetId()] = pElement;

	const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();

	float fMarginInsideControl = pEnv->GetControlHorizMargin(m_pContainer->GetContainerIsTabNotView());
	// TODO May need to reinstate this for tabs
	float fWidthToUse;
	float fContainerWidth = m_pContainer->GetDisplayableWidth() - 2 * fMarginInsideControl;
	if (m_pContainer->GetCentreELementsHorizontally()) {
		fWidthToUse = m_pContainer->GetDisplayableWidth();
	}
	else {
		fWidthToUse = pElement->GetPreferredControlWidth();
	}
	pElement->InitialiseForWidth(fWidthToUse);

	if (m_bDisplayScrollbar) {
		UpdateScrollableContentHeight();
	}
	else {
		ConsiderDisplayingScrollbars();
	}
}

void ElementCollection::AddElementToMap(IOverlayElement* pElement) {
	m_mpIdToElement[pElement->GetId()] = pElement;

}

IOverlayElement* ElementCollection::GetElement(int nElementId) {
	auto find = m_mpIdToElement.find(nElementId);
	if (find != m_mpIdToElement.end()) {
		return find->second;
	}
	return nullptr;
}

ActionResult ElementCollection::MoveUp() {
	if (m_pHighlightedElement == nullptr) {
		if (HighlightFirstElement()) {
			return ActionResult(false);
		}
	}
	if (m_nHighlightedIndex == 0) {
		return ActionResult(false);
	}
	if (m_pHighlightedElement == nullptr) {
		return MoveUp_NoHighlights();
	}
	float scrollDelta = CalcScrollDeltaToElementFullyDisplayed(m_pHighlightedElement);
	if (scrollDelta < -m_pHighlightedElement->GetHeight() && m_nHighlightedIndex < m_vcElements.size() - 1) {
		// Element is more than its height off the tab - must have scrolled past it onto a long (or several) non-highlightable elements
		//  ie displayed a wall of text with the highlighted item off the top of the tab
		// Scroll up instead of jumping to the element

		// In theory, the next element could also be above the tab, and the one after. It is possible that
		//  the elements could all have a different scroll increment, so should loop to find the one that is just on the tab (that is scrolling 'more on')
		int nNextElementIndex = m_nHighlightedIndex + 1;
		auto e = m_vcElements[nNextElementIndex];
		float scrollIncrement = e->GetScrollIncrement();
		ScrollByDelta(-scrollIncrement);
		return true;
	}
	for (int nHighlightIndex = m_nHighlightedIndex - 1; nHighlightIndex >= 0; --nHighlightIndex) {
		auto e = m_vcElements[nHighlightIndex];
		if (e->CanHighlight()) {
			HighlightElement(e, nHighlightIndex);
			break;
		}
		else {
			float scrollDelta = CalcScrollDeltaToElementFullyDisplayed(e);
			// Positive scroll delta would move elements up the screen, and we need to move them down whilst we scroll up
			if (scrollDelta < 0.0f) {
				float scrollIncrement = e->GetScrollIncrement();
				if (scrollDelta < -scrollIncrement) {
					scrollDelta = -scrollIncrement;
				}
				ScrollByDelta(scrollDelta);
				break;
			}
		}
	}
	return ActionResult(true);
}

ActionResult ElementCollection::MoveUp_NoHighlights() {
	// No highlightable elements, find the first element on screen and scroll it.
	int index = FindFirstDisplayedElementIndex();
	if (index == -1) {
		// nothing to be done here, apart from reset scroll offset.
		ScrollByDelta(-m_yScrollOffset);
		return ActionResult(false);
	}
	auto e = m_vcElements[index];
	if (index > 0) {
		ScrollByDelta(-e->GetScrollIncrement());
	}
	else {
		float yElementTop = e->GetLastRenderedYOrigin() + e->GetHeight();
		float yTabTop = m_yLastRenderedOrigin + TAB_MARGIN + m_pContainer->GetDisplayableHeight();
		if (yElementTop > yTabTop) {
			float nScrollDelta = e->GetScrollIncrement();
			if (nScrollDelta > yElementTop - yTabTop) {
				nScrollDelta = yElementTop - yTabTop;
			}
			ScrollByDelta(-nScrollDelta);
		}
	}
	return ActionResult(true);
}

ActionResult ElementCollection::MoveDown() {
	if (m_pHighlightedElement == nullptr) {
		if (HighlightFirstElement()) {
			return ActionResult(false);
		}
	}
	if (m_pHighlightedElement != nullptr) {
		float scrollDelta = CalcScrollDeltaToElementFullyDisplayed(m_pHighlightedElement);
		if (scrollDelta > m_pHighlightedElement->GetHeight() && m_nHighlightedIndex > 0) {
			// Element is more than its height off the tab - must have scrolled above it onto a long (or several) non-highlightable elements
			//  ie displayed a wall of text with the highlighted item off the bottom of the tab
			// Scroll down (move content up) instead of jumping to the element

			// See note in MoveUp() about this does not possibly choose the correct Scroll Increment.
			int nNextElementIndex = m_nHighlightedIndex - 1;
			auto e = m_vcElements[nNextElementIndex];
			float scrollIncrement = e->GetScrollIncrement();
			ScrollByDelta(scrollIncrement);
			return ActionResult(true);
		}
	}
	for (int nHighlightIndex = m_nHighlightedIndex + 1; nHighlightIndex < m_vcElements.size(); ++nHighlightIndex) {
		auto e = m_vcElements[nHighlightIndex];
		if (e->CanHighlight()) {
			HighlightElement(e, nHighlightIndex);
			break;
		}
		else {
			float scrollDelta = CalcScrollDeltaToElementFullyDisplayed(e);
			// Negative scroll delta would move elements down the screen, and we are moving them up the screen to scroll down
			if (scrollDelta > 0) {
				float scrollIncrement = e->GetScrollIncrement();
				if (scrollDelta > scrollIncrement) {
					scrollDelta = scrollIncrement;
				}
				ScrollByDelta(scrollDelta);
				break;
			}
		}
	}
	return ActionResult(true);
}

bool ElementCollection::HighlightFirstElement() {
	int n = 0;
	for (auto e : m_vcElements) {
		if (e->CanHighlight()) {
			if (e != m_pHighlightedElement) {
				if (m_pHighlightedElement != nullptr) {
					m_pHighlightedElement->Highlight(false);
				}
				m_pHighlightedElement = e;
				m_nHighlightedIndex = n;
				m_pHighlightedElement->Highlight(true);
				EnsureElementFullyDisplayed(m_pHighlightedElement);
			}
			return true;
		}
		++n;
	}

	return false;
}

bool ElementCollection::HighlightLastElement() {
	if (m_vcElements.size() == 0) {
		return false;
	}
	auto e = m_vcElements.back();
	if (e->CanHighlight() && e != m_pHighlightedElement) {
		if (m_pHighlightedElement != nullptr) {
			m_pHighlightedElement->Highlight(false);
		}
		m_pHighlightedElement = e;
		m_nHighlightedIndex = static_cast<int>(m_vcElements.size() - 1);
		m_pHighlightedElement->Highlight(true);

		EnsureElementFullyDisplayed(m_pHighlightedElement);
	}
	return true;
}

bool ElementCollection::HighlightElement(IOverlayElement* pElement, int nIndex) {
	if (pElement->CanHighlight()) {
		if (pElement != m_pHighlightedElement) {
			if (m_pHighlightedElement != nullptr) {
				m_pHighlightedElement->Highlight(false);
			}
			m_pHighlightedElement = pElement;
			m_nHighlightedIndex = nIndex;
			m_pHighlightedElement->Highlight(true);
			EnsureElementFullyDisplayed(m_pHighlightedElement);
		}
		return true;
	}
	return false;
}

bool ElementCollection::HighlightElementWithIndex(int nIndex) {
	return HighlightElement(m_vcElements[nIndex], nIndex);
}

bool ElementCollection::GetIsValidIndex(int nIndex) const {
	return nIndex >= 0 && nIndex < static_cast<int>(m_vcElements.size());
}

void ElementCollection::EnsureHighlightedElementFullyDisplayed() {
	EnsureElementFullyDisplayed(m_pHighlightedElement);
}

ActionResult ElementCollection::Action(unsigned int unAction) {
	if (m_pHighlightedElement != nullptr) {
		return m_pHighlightedElement->Action(unAction);
	}
	else {
		return false;
	}
}

void ElementCollection::ScrollByDelta(float nDelta) {
	if (nDelta != 0) {
		float fPixelsPerFrame = m_fPixelsPerSecond / m_fFrameRate;

		if (m_bScrollAnimating) {
			m_fScrollOffsetAnimateTarget = m_fScrollOffsetAnimateCurrent + nDelta;
		}
		else {
			m_fScrollOffsetAnimateTarget = static_cast<float>(m_yScrollOffset + nDelta);
			m_fScrollOffsetAnimateCurrent = static_cast<float>(m_yScrollOffset);
		}

		float fDistance = fabs(m_fScrollOffsetAnimateTarget - m_fScrollOffsetAnimateCurrent);

		float fFrames = fDistance / fPixelsPerFrame;
		m_fScrollAnimationStepPerSec = m_fFrameRate * (m_fScrollOffsetAnimateTarget - m_fScrollOffsetAnimateCurrent) / fFrames;
		m_bScrollAnimating = true;
	}
}

void ElementCollection::BackdropTouched(int x, int y, bool bPressed) {
	if (m_bBackdropScrolling) {
		if (bPressed) {
			int diff = y - m_yBackdropScrollStart;
			float tabHeight = m_pContainer->GetDisplayableHeight();
			if (m_yScrollOffset + diff < 0) {
				diff = static_cast<int>(-m_yScrollOffset);
			}
			else if (m_yScrollOffset + diff > m_fContentHeight - tabHeight) {
				diff = static_cast<int>(m_fContentHeight - tabHeight - m_yScrollOffset);
			}
			if (diff != 0) {
				ScrollByDelta(static_cast<float>(diff));
				m_yBackdropScrollStart = y;
			}
		}
		else {
			m_bBackdropScrolling = false;
		}
	}
	else if (bPressed) {
		m_bBackdropScrolling = true;
		m_yBackdropScrollStart = y;
	}
}

void ElementCollection::ControlResized() {
	if (m_bDisplayScrollbar) {
		UpdateScrollableContentHeight();
	}
	else {
		ConsiderDisplayingScrollbars();
	}
}

void ElementCollection::InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, float fWidth, float fHeight) {
	m_pTouchLayer = pTouchLayer;

	float topY = static_cast<float>(yOrigin + fHeight + m_yScrollOffset);
	int nElementIndexAsInvokeValue = 0;
	const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();
	float verticalSpacing = pEnv->GetVerticalInterElementSpacing(true);

	float xElementOrigin = xOrigin;
	float yElementOrigin = 0.0f;

	if (m_pContainer->GetContainerAspectIsVertical()) {
		for (auto e : m_vcElements) {
			float elementHeight = e->GetHeight();
			yElementOrigin = topY - elementHeight;

			e->InitialiseTouchAreas(pTouchLayer, xElementOrigin, yElementOrigin, this, nElementIndexAsInvokeValue);
			topY = yElementOrigin - verticalSpacing;
			nElementIndexAsInvokeValue++;
		}
	}

	if (m_bDisplayScrollbar) {
		m_pScrollbar->InitialiseTouchAreas(pTouchLayer, xOrigin + m_pContainer->GetDisplayableWidth(), yOrigin, this, ScrollbarElement::GetInvokeValue());
	}

	float fBackdropForScrollingWidth = fWidth;
	if (m_bDisplayScrollbar) {
		fBackdropForScrollingWidth -= ScrollbarElement::GetWidth();
	}
	int id = pTouchLayer->AddTouchArea(xOrigin, yOrigin, fBackdropForScrollingWidth, fHeight, this, pEnv->GetInternalActionStart(), TABBACKDROP_SCROLLTARGETVALUE);
	pTouchLayer->SetScrollingBehaviour(id, false, false);
	pTouchLayer->SetTouchTargetNeedsContinuousUpdates(id, true);
}

void ElementCollection::EnsureElementFullyDisplayed(IOverlayElement* pElement) {
	float scrollDelta = CalcScrollDeltaToElementFullyDisplayed(pElement);
	ScrollByDelta(scrollDelta);
}

float ElementCollection::CalcScrollDeltaToElementFullyDisplayed(IOverlayElement* pElement) {
	if (!pElement->RenderedYet()) {
		return 0;
	}
	float yOrigin = pElement->GetLastRenderedYOrigin();
	float fHeight = pElement->GetHeight();
	float fDisplayTabHeight = m_pContainer->GetDisplayableHeight();

	bool bScrolled = false;
	if (yOrigin < m_yLastRenderedOrigin + TAB_MARGIN) {
		return m_yLastRenderedOrigin + TAB_MARGIN - yOrigin;
	}
	else if (yOrigin + fHeight > m_yLastRenderedOrigin + fDisplayTabHeight) {
		return -(yOrigin + fHeight - (m_yLastRenderedOrigin + TAB_MARGIN + fDisplayTabHeight));
	}
	return 0.0f;
}

int ElementCollection::FindFirstDisplayedElementIndex() {
	int nDisplayTabHeight = static_cast<int>(m_pContainer->GetDisplayableHeight());

	for (int index = 0; index < m_vcElements.size(); ++index) {
		auto e = m_vcElements[index];
		float y = e->GetLastRenderedYOrigin();
		if (y < m_yLastRenderedOrigin + nDisplayTabHeight) {
			return index;
		}
	}
	return -1;
}

void ElementCollection::ConsiderDisplayingScrollbars() {
	if (m_bDisplayScrollbar || m_pContainer->GetContainerAspectIsVertical()==false) {
		return;
	}
	IOverlayIntl* pOverlay = m_pContainer->GetOverlay();
	m_fContentHeight = CalculateTotalHeight();
	if (m_fContentHeight > m_pContainer->GetDisplayableHeight()) {
		if (m_pScrollbar == nullptr) {
			
			int nElementId = pOverlay->NextElementId();
			m_pScrollbar = new ScrollbarElement(pOverlay, this, nElementId, m_pContainer->GetDisplayableHeight(), static_cast<int>(m_fContentHeight), !m_pContainer->GetContainerIsTabNotView());
		}
		else {
			m_pScrollbar->SetTotalContentHeight(static_cast<int>(m_fContentHeight));
		}
		m_bDisplayScrollbar = true;

		const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();
		float fMarginInsideControl = pEnv->GetControlHorizMargin(m_pContainer->GetContainerIsTabNotView());

		float width = m_pContainer->GetDisplayableWidth() - 2 * fMarginInsideControl;
		for (auto e : m_vcElements) {
			// Width of control may already be smaller than this width - because the control does not fill the container (tab or view) horizontally
			// Still need to re-initialise in case it involves centring the control different now that there is a scrollbar to the right
			if (m_pContainer->GetCentreELementsHorizontally()) {
				// This is for tabs where a control is centred
				e->InitialiseForWidth(width);
			}
			else {
				// This is for views where text fields may be left or right aligned
				float fWidthToUse = width;
				if (e->GetPreferredControlWidth() < fWidthToUse) {
					fWidthToUse = e->GetPreferredControlWidth();
				}
				e->InitialiseForWidth(fWidthToUse);
			}
		}
	}
}

void ElementCollection::UpdateScrollableContentHeight() {
	m_fContentHeight = CalculateTotalHeight();
	if (m_pScrollbar != nullptr) {
		m_pScrollbar->SetTotalContentHeight(static_cast<int>(m_fContentHeight));
	}
}

float ElementCollection::CalculateTotalHeight() const {
	float cumulativeHeight = 0.0f;

	bool bTabNotView = m_pContainer->GetContainerIsTabNotView();

	const IOverlayEnvIntl* pEnv = m_pContainer->GetEnvironment();
	float fInterElementSpacing = pEnv->GetVerticalInterElementSpacing(bTabNotView);

	for (auto e : m_vcElements) {
		cumulativeHeight += e->GetHeight();
		cumulativeHeight += fInterElementSpacing;
	}
	cumulativeHeight -= fInterElementSpacing;
	return cumulativeHeight;
}

bool ElementCollection::DataAltered() const { 
	for (auto e : m_vcElements) {
		if (e->Altered()) {
			return true;
		}
	}
	return false;
}

void ElementCollection::RevertData() { 
	for (auto e : m_vcElements) {
		e->Revert();
	}
}

bool ElementCollection::SaveData() { 
	bool bCanSave = true;
	for (auto e : m_vcElements) {
		if (e->Altered()) {
			if (!e->CanSave()) {
				bCanSave = false;
				// Check all, in case validation messages appear
			}
		}
	}
	if (!bCanSave) {
		return false;
	}
	for (auto e : m_vcElements) {
		if (e->Altered()) {
			e->Save();
		}
	}
	return true;
}

bool ElementCollection::CanClose() const {
	bool bCanClose = true;
	for (auto e : m_vcElements) {
		if (e->Altered()) {
			if (!e->CanSave()) {
				bCanClose = false;
			}
		}
	}
	return bCanClose;
}

void ElementCollection::EnableElement(int nElementId, bool bEnabled) {
	auto find = m_mpIdToElement.find(nElementId);
	if (find != m_mpIdToElement.end()) {
		find->second->Disable(!bEnabled);
	}
}

void ElementCollection::ShowDebugBackgroundOnControlType(OverlayControlType eType, bool bShow) {
	for (IOverlayElement* e : m_vcElements) {
		if (e->GetControlType() == eType) {
			e->ShowDebugBackground(bShow);
		}
	}
}
