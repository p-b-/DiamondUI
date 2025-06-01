#pragma once

#include "ITextRenderer.h"
#include "OverlayEntity.h"
#include "ColourDefs.h"
#include <vector>
#include <string>
#include <tuple>

class GameOverlay
{
public:
	void Initialise(FontFaceHandle hText, rgb colText, rgba rgbaOverlay);
	void Deinitialise();

	void Render(float fDeltaTime);

	IOverlayEntity* AddTextLabel(std::string sText, std::string sMaxText, int nX, int nY, OverlayEntityRelativeTo eRelativeTo);
	IOverlayEntity* AddTextLabel(std::string sText, int nX, int nY, int nWidth, int nHeight, OverlayEntityRelativeTo eRelativeTo);
	IOverlayEntity* AddTextLabel(std::string sText, OverlayEntityRelativeTo eRelativeTo, IOverlayEntity* pRelativeTo);

private:
	
private:
	std::vector<OverlayEntity* > m_vcOverlayEntities;
	FontFaceHandle m_hText;
	rgb m_rgbText;
	rgba m_rgbaOverlay;
};

