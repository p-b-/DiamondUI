#pragma once
#include <string>
#include <tuple>
#include <vector>

// ITextLine represents an interface that the TextRenderer implementation of ITextRenderer both creates when word-wrapping, and consumes when rendering
// The overlay system needs will call ITextRenderer to create these when text changes, but it does not need to know the internals of how the graphics are represented.
class ITextLine {
public:
	virtual ~ITextLine() {}

	virtual float GetWidth() const = 0;
	virtual float GetHeight() const = 0;
	virtual float GetDescenderHeight() const = 0;
	virtual float GetCharOffset(int nChar) = 0;
};

class FontFaceHandle {
public:
	FontFaceHandle(bool bValid = false, unsigned int unHandle = 0) :
		m_bIsValid(bValid), m_unHandle(unHandle) {
	}
	bool IsValid() const { return m_bIsValid; }
	unsigned int GetIntHandle() const { return m_unHandle; }
private:
	bool m_bIsValid;
	unsigned int m_unHandle;
};

// The implementation of this allows the overlay system to create text
// TODO Split this into ITextRenderer and ITextRendererIntl
//      The overlay system does not need to know how to render text.
class ITextRenderer {
public:
	virtual ~ITextRenderer() {}
	virtual void WindowSizeChanged(int nWidth, int nHeight) = 0;
	virtual void RegisterFont(int nFontFileHandle, std::string sFontFile) = 0;
	virtual FontFaceHandle CreateFont(int nFontFileHandle, unsigned int unHeight) = 0;
	virtual std::vector<ITextLine*> CreateMultipleTextLines(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) = 0;
	virtual ITextLine* CreateTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) = 0;
	virtual ITextLine* CreateEditableTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) = 0;
	virtual void RenderTextLine(const ITextLine* pITextLine, float fX, float fY, std::tuple<float, float, float> rgbText) = 0;
	virtual void RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fScale, std::tuple<float, float, float> rgbText) = 0;
	virtual void RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fWidth, float fScale, std::tuple<float, float, float> rgbText) = 0;
	virtual std::tuple<int, int, int, int> MeasureText(const FontFaceHandle& hFontFace, std::string sText, float fScale) = 0;

	virtual float GetFontHeight(const FontFaceHandle& hFontFace) = 0;
	virtual float GetFontDescenderHeight(const FontFaceHandle& hFontFace) = 0;
};