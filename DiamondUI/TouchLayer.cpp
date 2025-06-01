#include "TouchLayer.h"
#include "ITouchTarget.h"
#include "InputDefs.h"
#include "IEngineInputIntl.h"

std::atomic<unsigned int> TouchLayer::s_unNextId = 1;

IEngineInputIntl* TouchLayer::s_pEngineInput;

TouchLayer::TouchLayer(int xOrigin, int yOrigin, int nWidth, int nHeight, ITouchTarget* pTarget, unsigned int unOutsideAreaAction) :
	m_layerRect(xOrigin, yOrigin, nWidth, nHeight) {
	m_pTarget = pTarget;
	m_unOutsideAreaAction = unOutsideAreaAction;
	m_bActive = false;
	m_yScrollDelta = 0.0f;
	m_xScrollDelta = 0.0f;
	m_fXOffset = 0.0f;
	m_fYOffset = 0.0f;
}

TouchLayer::TouchLayer(float xOrigin, float m_nYOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unOutsideAreaAction) :
	TouchLayer(static_cast<int>(xOrigin), static_cast<int>(m_nYOrigin),
		static_cast<int>(fWidth), static_cast<int>(fHeight),
		pTarget, unOutsideAreaAction) {
}

TouchLayer::~TouchLayer() {
	for (auto pTA : m_vcTouchAreas) {
		delete pTA;
	}
	m_vcTouchAreas.clear();
}

// ITouchLayer implmentation
//
void TouchLayer::SetLayerActive(bool bActive) {
	m_bActive = bActive;
} 

void TouchLayer::AddToInputEngine() {
	s_pEngineInput->AddTouchLayer(this);
}

void TouchLayer::RemoveFromInputEngine() {
	s_pEngineInput->RemoveTouchLayer(this);
}

bool TouchLayer::GetLayerActive() const {
	return m_bActive;
}

void TouchLayer::SetYScrollDelta(float yScrollDelta) { 
	m_yScrollDelta = yScrollDelta;
}

void TouchLayer::SetXScrollDelta(float xScrollDelta) {
	m_xScrollDelta = xScrollDelta;
}

unsigned int TouchLayer::AddTouchArea(float xOrigin, float m_nYOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nTouchValue) {
	unsigned int id = s_unNextId++;
	TouchArea* pTA = new TouchArea { true, true, IntRect{static_cast<int>(xOrigin),static_cast<int>(m_nYOrigin), static_cast<int>(fWidth),static_cast<int>(fHeight) }, unAction, pTarget, nTouchValue, false, false, false};
	m_vcTouchAreas.push_back(pTA);
	m_mpTouchAreas[id] = pTA;
	return id;
}

void TouchLayer::SetScrollingBehaviour(int unId, bool bScrollsVertically, bool bScrollsHorizontally) {
	auto* pTA = GetTouchArea(unId);
	if (pTA != nullptr) {
		pTA->m_bScrollVert = bScrollsVertically;;
		pTA->m_bScrollHoriz = bScrollsHorizontally;
	}
}

void TouchLayer::SetTouchTargetNeedsContinuousUpdates(int unId, bool bContinuousUpdates) {
	auto* pTA = GetTouchArea(unId);
	if (pTA != nullptr) {
		pTA->m_bContinuousUpdates = bContinuousUpdates;
	}
}

void TouchLayer::SetTouchAreaActive(unsigned int unId, bool bActive) {
	auto pTA = GetTouchArea(unId);
	if (pTA != nullptr) {
		pTA->m_bIsActive = bActive;
	}
}

Touch TouchLayer::GetTouchTarget(int x, int y) {
	if (m_bActive == false) {
		return Touch(false);
	}
	if (PointOutsideRect(x, y, m_layerRect)) {
		if (m_unOutsideAreaAction != 0) {
			Touch t(true);
			t.m_bTouchInsideArea = false;
			t.m_unAction = m_unOutsideAreaAction;
			t.m_pTarget = m_pTarget;
			t.m_nInvocationValue = 0;
			return t;
		}
		return Touch(false);
	}
	int index = 0;
	for (auto pTA : m_vcTouchAreas) {
		bool bInsideArea = false;
		if (!pTA->m_bScrollVert && !pTA->m_bScrollHoriz) {
			bInsideArea = !PointOutsideRect(x, y, pTA->m_rect);
		} 
		else {
			bInsideArea = !PointOutsideScrolledRect(x, y, pTA->m_rect, pTA->m_bScrollVert, pTA->m_bScrollHoriz);
		}

		if (bInsideArea) {
			if (pTA->m_pTarget != nullptr) {
				Touch t(true);
				t.m_unAction = pTA->m_unAction;
				t.m_pTarget = pTA->m_pTarget;
				t.m_nInvocationValue = pTA->m_nTouchValue;
				t.m_bContinuousUpdate = pTA->m_bContinuousUpdates;
				t.m_rcTouch = pTA->m_rect;
				return t;
			}
			return Touch(false);
		}
		++index;
	}
	return Touch(false);
}

void TouchLayer::MoveLayer(float fXOffset, float fYOffset) {
	// Do not alter the m_layerRect - points being compared are offset by these:
	m_fXOffset += fXOffset;
	m_fYOffset += fYOffset;
}
//
// ITouchLayer implmentation

TouchLayer::TouchArea* TouchLayer::GetTouchArea(unsigned int unId) {
	auto find = m_mpTouchAreas.find(unId);
	if (find == m_mpTouchAreas.end()) {
		return nullptr;
	}
	return find->second;
}

bool TouchLayer::PointOutsideRect(int x, int y, const IntRect& rect) {
	x -= static_cast<int>(m_fXOffset);
	y -= static_cast<int>(m_fYOffset);
	
	return x<rect.m_nXOrigin || y<rect.m_nYOrigin ||
		x>rect.m_nXOrigin + rect.m_nWidth ||
		y>rect.m_nYOrigin + rect.m_nHeight;
}

bool TouchLayer::PointOutsideScrolledRect(int x, int y, const IntRect& rect, bool bCanScrollVert, bool bCanScrollHoriz) {
	x -= static_cast<int>(m_fXOffset);
	y -= static_cast<int>(m_fYOffset);
	if (bCanScrollVert && !bCanScrollHoriz) {
		return x<rect.m_nXOrigin || y<rect.m_nYOrigin + m_yScrollDelta ||
			x>rect.m_nXOrigin + rect.m_nWidth ||
			y>rect.m_nYOrigin + rect.m_nHeight + m_yScrollDelta;
	}
	else if (bCanScrollHoriz && !bCanScrollVert) {
		return x<rect.m_nXOrigin + m_xScrollDelta || y<rect.m_nYOrigin ||
			x>rect.m_nXOrigin + rect.m_nWidth + m_xScrollDelta ||
			y>rect.m_nYOrigin + rect.m_nHeight;
	}
	else if (bCanScrollVert && bCanScrollHoriz) {
		return x<rect.m_nXOrigin + m_xScrollDelta || y<rect.m_nYOrigin + m_yScrollDelta ||
			x>rect.m_nXOrigin + rect.m_nWidth + m_xScrollDelta ||
			y>rect.m_nYOrigin + rect.m_nHeight + m_yScrollDelta;
	}
	else {
		return PointOutsideRect(x, y, rect);
	}
}

void TouchLayer::SetEngineInput(IEngineInputIntl* pEngineInput) {
	TouchLayer::s_pEngineInput = pEngineInput;
}