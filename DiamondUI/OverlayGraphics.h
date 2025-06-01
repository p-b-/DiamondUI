#pragma once
#include "ColourDefs.h"
#include "Shader.h"
#include "ITextRenderer.h"
#include "OverlayDefs.h"

#include <stack>
#include <list>

class Textures;
class IOverlayEnv;

class OverlayGraphics
{
	struct ScissorState {
		GLboolean m_bEnabled;
		int m_nX;
		int m_nY;
		int m_nWidth;
		int m_nHeight;
	};
public:
	static void InitialiseOverlayGraphics(int nScreenWidth, int nScreenHeight);
	static void UpdateOrthProjection(int nWidth, int nHeight);
	static void Deinitialise();
	static void SetTextureCtrl(Textures* pTextureCtrl);
	static void SetGlyphAtlasId(unsigned int unGlypthAtlasId);

public:
	static void SetButtonRenderFunc(fnRenderButton pFunc);
	static void SetSelectorRenderFunc(fnRenderSelector pFunc);

public:
	static void RenderDebugBackground(int nElementId, float fX, float fY, float fWidth, float fHeight, rgba rgbaOverlay);

	static void RenderTextEditField(int nElementId, float fX, float fY, float fWidth, float fHeight, rgba col);
	static void RenderCaret(int nElementId, float fX, float fY, float fHeight, rgba rgbaCaret);

	static void RenderLeftArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow);
	static void RenderRightArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow);
	static void RenderUpArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow);
	static void RenderDownArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow);
	static void DrawButtonBackground(float fX, float fY, float fWidth, float fHeight, rgba rgbaButton);
	static void DrawSelectorBackground(float fX, float fY, float fWidth, float fHeight, rgba rgbaButton);

	static void RenderSelector(const IOverlayEnv* pEnv, int nElementId, float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, ITextLine* pTLValue);
	static void RenderButton(const IOverlayEnv* pEnv, int nElementId, float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, OverlayAnimation eAnim, float fAnimationSeqPt, ITextLine* pTLText);

protected:
	static void InitialiseVectorBuffer(float* pVertices, int nCount, unsigned int* punVAO, unsigned int* punVBO);
	static void InitialiseVectorWithTexturesBuffer(float* pVertices, int nCount, unsigned int* punVAO, unsigned int* punVBO);

	static void RenderTranslucentOverlay(int nX, int nY, int nWidth, int nHeight, rgba rgbaOverlay);
	static void RenderTranslucentOverlay(float fX, float fY, float fWidth, float fHeight, rgba rgbaOverlay);
	static bool RenderGlypth(float fX, float fY, float fWidth, float fHeight, unsigned int unAction);
	static void RenderOutline(int nX, int nY, int nWidth, int nHeight, rgba rgba);

	static void RenderButtonOutline(float fX, float fY, float fWidth, float fHeight, rgba col);
	static void RenderDivider(unsigned int unDivider, float fX, float fY, float fWidth, float fHeight);

	static float GetTextDescender(const IOverlayEnv* pEnv);

	static void SetClippingArea(int nX, int nY, int nWidth, int nHeight);
	static void SetClippingArea(float fX, float fY, float fWidth, float fHeight);
	static void RemoveClippingArea();

protected:
	static ITextRenderer* s_pTextRenderer;

	static fnRenderButton s_pRenderButtonFunc;
	static fnRenderSelector s_pRenderSelectorFunc;

	static float s_fTextDescender;

	static unsigned int s_unVAOQuad;
	static unsigned int s_unVBOQuad;
	static unsigned int s_unVAOHighlightOutline;
	static unsigned int s_unVBOHighlightOutline;
	static unsigned int m_unVAOGlyph;
	static unsigned int m_unVBOGlyph;
	static unsigned int s_unVAOButtonOutline;
	static unsigned int s_unVBOButtonOutline;

	static unsigned int s_unVAOLeftButton;
	static unsigned int s_unVBOLeftButton;
	static unsigned int s_unVerticesCountInArrow;

	static int s_nScreenWidth;
	static int s_nScreenHeight;
	static Shader s_shader;
	static Shader s_shaderGlyph;
	static Textures* s_pTextureCtrl;
	static unsigned int s_unGlyphAtlasId;
	static unsigned int s_unGlyphTextureId;

	static std::list<unsigned int> s_lsVAO;
	static std::list<unsigned int> s_lsVBO;

	static std::stack<ScissorState> s_stScissorState;
};
