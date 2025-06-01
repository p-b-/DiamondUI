#pragma once
#include "ITextRenderer.h"
#include "Shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H  

#include <string>
#include <map>
#include <atomic>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Window;
class TextLine;

class TextRenderer : public ITextRenderer
{
	struct CharInfo {
		float m_fxAdvance;
		float m_fyAdvance;

		float m_fBitmapWidth;
		float m_fBitmapHeight;

		float m_fBitmapLeft;
		float m_fBitmapTop;

		float m_fxTexture;
	};
	struct FontFace {
		std::map<char, CharInfo> m_mpCharInfo;
		std::tuple<int,int> m_tpAtlasSize;
		GLuint m_unAtlasTextureId;
	};
	struct point {
		GLfloat x;
		GLfloat y;
		GLfloat s;
		GLfloat t;
	};
public:
	TextRenderer();
	virtual ~TextRenderer();

	bool Initialise(Window* pWnd);
	void UpdateWindow(Window* pWnd);
	void FinishedLoading();

	virtual void WindowSizeChanged(int nWidth, int nHeight);
	virtual void RegisterFont(int nFontFileHandle, std::string sFontFile);
	virtual FontFaceHandle CreateFont(int nFontFileHandle, unsigned int unHeight);
	virtual std::vector<ITextLine*> CreateMultipleTextLines(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale);
	virtual ITextLine* CreateTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale);
	virtual ITextLine* CreateEditableTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale);
	virtual void RenderTextLine(const ITextLine* pITextLine, float fX, float fY, std::tuple<float, float, float> rgbText);
	virtual void RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fScale, std::tuple<float,float,float> rgbText);
	virtual void RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fWidth, float fScale, std::tuple<float, float, float> rgbText);
	virtual std::tuple<int, int, int, int> MeasureText(const FontFaceHandle& hFontFace, std::string sText, float fScale);
	virtual float GetFontHeight(const FontFaceHandle& hFontFace);
	virtual float GetFontDescenderHeight(const FontFaceHandle& hFontFace);
	
private:
	TextRenderer::FontFace* GetFontFace(const FontFaceHandle& hFont);
	std::string GetFilenameFromFileHandle(int nFontFileHandle);
	FontFaceHandle RegisterFontFaceHandle(int nFontFileHandle, unsigned int unHeight, const FontFace& fontFace);
	FontFaceHandle GetFontFaceHandleIfCreated(int nFontFileHandle, unsigned int unHeight);

	bool RenderSetup(const FontFaceHandle& hFontFace, std::tuple<float, float, float> rgbText, FontFace*& prefFontFace);
	inline void CreateQuadForChar(point* pCoords, size_t nPointsLength, int& nNextVertex, const CharInfo& ch, FontFace* pFontFace, float x, float y, float w, float h);
	void RenderVertices(FontFace* pFontFace, point* pCoords, size_t nPointsLength, int nVertexCount);
	TextLine* RenderVerticesToTextLine(FontFace* pFontFace, const FontFaceHandle& hFontFace, point* pCoords, size_t nPointsLength, int nVertexCount);
	void CreateBuffersForRendering();
	std::tuple<int, int> DetermineFontAtlasTextureSize(FT_Face ftFace, char cStart, char cEnd);
	GLuint CreateAtlasTexture(FontFace* pFont);
	void FillFontAtlasTexture(FT_Face ftFace, char cStart, char cEnd, FontFace* pFontFace);

private:
	std::map<unsigned int, TextRenderer::FontFace> m_mpFontFaces;
	FT_Library m_ftLib;
	std::atomic<unsigned int> m_aunNextHandle;
	std::map<int, std::string> m_mpFontFileHandleToFilename;
	std::map<int, unsigned int> m_mpFontFacesCreated; // Maps (font file handle * 65536 + height) => font handle integer

	Shader m_shader;
	unsigned int m_unVAO;
	unsigned int m_unVBO;
};
