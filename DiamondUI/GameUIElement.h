#pragma once

#include "OverlayDefs.h"
class ITextLine;

class GameUIElement
{
public:
	static void RenderButton(float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, ITextLine* pTLText);
	static void RenderSelector(float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, ITextLine* pTLValue);
};

