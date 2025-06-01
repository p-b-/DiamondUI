#pragma once
#include "OverlayGraphics.h"
#include "IOverlayEntity.h"
#include <tuple>

enum OverlayEntityRelativeTo {
	eOERTFromTopLeft,
	eOERTFromTopRight,
	eOERTFromBottomRight,
	eOERTFromBottomLeft,
	eOERTAbove,
	eOERTBelow,
	eOERTLeftOf,
	eOERTRightOf
};

class OverlayEntity : public OverlayGraphics, public IOverlayEntity {
public:
	virtual ~OverlayEntity() {}
	virtual void Render();

	void SetBoundingRect(int nX, int nY, OverlayEntityRelativeTo eRelativeTo, int nWidth = 0, int nHeight = 0);
	void SetBoundingRect(OverlayEntityRelativeTo eRelativeTo, OverlayEntity* pOtherEntity);
	
	static void SetTextRenderer(ITextRenderer* pTextRenderer) { s_pTextRenderer = pTextRenderer; }

	int GetWidth() const { return m_nWidth; }
	int GetHeight() const { return m_nHeight; }

protected:
	
	int m_nX;
	int m_nY;
	int m_nWidth;
	int m_nHeight;
	std::tuple<float,float,float,float> m_rgbaOverlay;
};