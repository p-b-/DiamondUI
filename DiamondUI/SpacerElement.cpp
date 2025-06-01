#include "SpacerElement.h"
#include "IOverlayEnvIntl.h"

SpacerElement::SpacerElement(IOverlayIntl* pOverlay, unsigned int unHeight, unsigned int unDividerType /*= 0*/) {
	m_pOverlayWithin = pOverlay;
	m_fSpacerHeight = static_cast<float>(unHeight);
	m_unDividerType = unDividerType;
	m_bDisabled = false;
	m_bHighlighted = false;
	m_bRenderedAtLeastOnce = false;
	m_bShowDebugBackground = false;
	m_yLastRenderedOrigin = 0;
	m_pOverlayEnv = m_pOverlayWithin->GetInternalOverlayEnvironment();
}

SpacerElement::~SpacerElement() {

}

// IOverlayElement implementation
//
void SpacerElement::Deinitialise() { 
}

void SpacerElement::FreeSpace() { 
}

void SpacerElement::InitialiseForWidth(float fWidth) { 
}

float SpacerElement::GetHeight() const { 
	return m_fSpacerHeight;
}

float SpacerElement::GetPreferredControlWidth() const {
	return static_cast<float>(m_pOverlayEnv->GetPreferredControlWidth());
}

void SpacerElement::Render(float xOrigin, float yOrigin, float fWidth, float fHeight) { 
	m_yLastRenderedOrigin = yOrigin;
	if (m_bShowDebugBackground) {
		const IOverlayEnv* pPublicEnv = m_pOverlayEnv->GetConstPublicInterface();
		RenderTranslucentOverlay(xOrigin, yOrigin, fWidth, fHeight, pPublicEnv->GetDebugBackgroundRGBA(true));
	}
	if (m_unDividerType != 0) {
		RenderDivider(m_unDividerType, xOrigin, yOrigin, fWidth, fHeight);
	}
}

float SpacerElement::GetScrollIncrement() {
	return m_fSpacerHeight;
}

void SpacerElement::ShowDebugBackground(bool bShow) { 
}
//
// IOverlayElement implementation
