#include "GameOverlay.h"
#include "OverlayTextLabel.h"

void GameOverlay::Initialise(FontFaceHandle hText, rgb rgbText, rgba rgbaOverlay) {
	m_hText = hText;

	m_rgbText = rgbText;
	m_rgbaOverlay = rgbaOverlay;
}

void GameOverlay::Deinitialise() {
	for (auto d : m_vcOverlayEntities) {
		delete d;
	}
	m_vcOverlayEntities.clear();
}

void GameOverlay::Render(float fDeltaTime) {
	for (auto e : m_vcOverlayEntities) {
		e->Render();
	}
}

IOverlayEntity* GameOverlay::AddTextLabel(std::string sText, std::string sMaxText, int nX, int nY, OverlayEntityRelativeTo eRelativeTo) {
	if (sMaxText.length() == 0) {
		sMaxText = sText;
	}
	OverlayTextLabel* pEntity = new OverlayTextLabel(sText, 10, m_hText, m_rgbText, m_rgbaOverlay);

	pEntity->SetBoundingRect(nX, nY, eRelativeTo);
	m_vcOverlayEntities.push_back(pEntity);

	return pEntity;
}

IOverlayEntity* GameOverlay::AddTextLabel(std::string sText, int nX, int nY, int nWidth, int nHeight, OverlayEntityRelativeTo eRelativeTo) {
	OverlayTextLabel* pEntity = new OverlayTextLabel(sText, nWidth, nHeight, m_hText, m_rgbText, m_rgbaOverlay);

	pEntity->SetBoundingRect(nX, nY, eRelativeTo);
	m_vcOverlayEntities.push_back(pEntity);

	return pEntity;
}

IOverlayEntity* GameOverlay::AddTextLabel(std::string sText, OverlayEntityRelativeTo eRelativeTo, IOverlayEntity* pRelativeTo) {
	OverlayEntity* pOtherEntity = dynamic_cast<OverlayEntity*>(pRelativeTo);
	int nWidth = pOtherEntity->GetWidth();
	int nHeight = pOtherEntity->GetHeight();

	OverlayTextLabel* pEntity = new OverlayTextLabel(sText, nWidth, nHeight, m_hText, m_rgbText, m_rgbaOverlay);

	pEntity->SetBoundingRect(eRelativeTo, pOtherEntity);

	m_vcOverlayEntities.push_back(pEntity);

	return pEntity;

}
