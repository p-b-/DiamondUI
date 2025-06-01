#include "TextRenderer.h"
#include "TextLine.h"
#include "Window.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#include <glad/glad.h>

TextRenderer::TextRenderer() {
	m_aunNextHandle = 1;
	m_ftLib = nullptr;
    m_unVAO = 0;
    m_unVBO = 0;
}

TextRenderer::~TextRenderer() {
    if (m_ftLib != nullptr) {
        FT_Done_FreeType(m_ftLib);
        m_ftLib = nullptr;
    }
    glDeleteVertexArrays(1, &m_unVAO);
    glDeleteBuffers(1, &m_unVBO);
}

bool TextRenderer::Initialise(Window* pWnd) {
	if (FT_Init_FreeType(&m_ftLib) != 0) {
		return false;
	}
    m_shader.InitialiseShaders("text_vert.glsl", "text_frag.glsl");
    UpdateWindow(pWnd);
    CreateBuffersForRendering();
    return true;
}

void TextRenderer::UpdateWindow(Window* pWnd) {
    int nWidth = pWnd->GetWidth();
    int nHeight = pWnd->GetHeight();
    WindowSizeChanged(nWidth, nHeight);
}

void TextRenderer::FinishedLoading() {
    if (m_ftLib != nullptr) {
        FT_Done_FreeType(m_ftLib);
        m_ftLib = nullptr;
    }
}

void TextRenderer::CreateBuffersForRendering() {
    glGenVertexArrays(1, &m_unVAO);
    glGenBuffers(1, &m_unVBO);
    glBindVertexArray(m_unVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_unVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TextRenderer::WindowSizeChanged(int nWidth, int nHeight) {
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(nWidth), 0.0f, static_cast<float>(nHeight));
    m_shader.Use();
    m_shader.SetMat4("projection", projection);
}

void TextRenderer::RegisterFont(int nFontFileHandle, std::string sFontFile) {
    m_mpFontFileHandleToFilename[nFontFileHandle] = sFontFile;
}

std::string TextRenderer::GetFilenameFromFileHandle(int nFontFileHandle) {
    auto i = m_mpFontFileHandleToFilename.find(nFontFileHandle);
    if (i == m_mpFontFileHandleToFilename.end()) {
        return "";
    }
    return i->second;
}


FontFaceHandle TextRenderer::RegisterFontFaceHandle(int nFontFileHandle, unsigned int unHeight, const FontFace& fontFace) {
    unsigned int unHandle = m_aunNextHandle++;
    m_mpFontFaces[unHandle] = fontFace;
    m_mpFontFacesCreated[nFontFileHandle * 65536 + unHeight] = unHandle;

    return FontFaceHandle(true, unHandle);
}

FontFaceHandle TextRenderer::GetFontFaceHandleIfCreated(int nFontFileHandle, unsigned int unHeight) {
    int lookUp = nFontFileHandle * 65536 + unHeight;
    auto i = m_mpFontFacesCreated.find(lookUp);
    if (i == m_mpFontFacesCreated.end()) {
        return {};
    }
    unsigned int unHandle = i->second;
    return FontFaceHandle(true, unHandle);
}

TextRenderer::FontFace* TextRenderer::GetFontFace(const FontFaceHandle& hFont) {
    unsigned int unH = hFont.GetIntHandle();
    auto i = m_mpFontFaces.find(unH);
    if (i == m_mpFontFaces.end()) {
        return nullptr;
    }
    return &i->second;
}

FontFaceHandle TextRenderer::CreateFont(int nFontHandle, unsigned int unHeight) {
    FontFaceHandle fontFaceHandle = GetFontFaceHandleIfCreated(nFontHandle, unHeight);
    if (fontFaceHandle.IsValid()) {
        return fontFaceHandle;
    }

    std::string sFontName = GetFilenameFromFileHandle(nFontHandle);
    FT_Face ftFace;
    // fonts\OCRAEXT.TTF
    std::ostringstream ssFont;
    ssFont << "fonts\\" << sFontName;

    if (FT_New_Face(m_ftLib, ssFont.str().c_str(), 0, &ftFace)) {
        // Invalid font handle
        return FontFaceHandle();
    }

    FT_Set_Pixel_Sizes(ftFace, 0, unHeight);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    FontFace fontFace;
    char cStart = 32;
    char cEnd = 127;
    fontFace.m_tpAtlasSize = DetermineFontAtlasTextureSize(ftFace, cStart, cEnd);
    fontFace.m_unAtlasTextureId = CreateAtlasTexture(&fontFace);
    FillFontAtlasTexture(ftFace, cStart, cEnd, &fontFace);

    FT_Done_Face(ftFace);
    return RegisterFontFaceHandle(nFontHandle, unHeight, fontFace);
}

std::tuple<int, int> TextRenderer::DetermineFontAtlasTextureSize(FT_Face ftFace, char cStart, char cEnd) {
    unsigned int w = 0;
    unsigned int h = 0;

    for (int i = cStart; i <= cEnd; i++) {
        if (FT_Load_Char(ftFace, i, FT_LOAD_RENDER)) {
            std::cerr << "Error: Loading character "<< i <<" failed" <<std::endl;
            continue;
        }

        w += ftFace->glyph->bitmap.width;
        h = std::max(h, ftFace->glyph->bitmap.rows);
    }
    return std::make_tuple(w, h);
}

GLuint TextRenderer::CreateAtlasTexture(FontFace* pFontFace) {
    GLuint unTexture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &unTexture);
    glBindTexture(GL_TEXTURE_2D, unTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int sz = std::get<0>(pFontFace->m_tpAtlasSize) * std::get<1>(pFontFace->m_tpAtlasSize);
    unsigned char* pixels = new unsigned char[sz];
    memset(pixels, 0, sz);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, std::get<0>(pFontFace->m_tpAtlasSize), std::get<1>(pFontFace->m_tpAtlasSize), 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    delete[] pixels;
    pixels = nullptr;
    
    return unTexture;
}

void TextRenderer::FillFontAtlasTexture(FT_Face ftFace, char cStart, char cEnd, FontFace* pFontFace) {
    GLuint unTexture = pFontFace->m_unAtlasTextureId;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, unTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    int x = 0;
    FT_GlyphSlot gs = ftFace->glyph;

    for (int i = cStart; i <= cEnd; i++) {
        if (FT_Load_Char(ftFace, i, FT_LOAD_RENDER)) {
            continue;
        }
       glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, gs->bitmap.width, gs->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, gs->bitmap.buffer);

        TextRenderer::CharInfo c;

        c.m_fxAdvance = (float)(gs->advance.x >> 6);
        c.m_fyAdvance = (float)(gs->advance.y >> 6);

        c.m_fBitmapWidth = (float)(gs->bitmap.width);
        c.m_fBitmapHeight = (float)(gs->bitmap.rows);

        c.m_fBitmapLeft = (float)gs->bitmap_left;
        c.m_fBitmapTop = (float)gs->bitmap_top;

        c.m_fxTexture = (float)x / std::get<0>(pFontFace->m_tpAtlasSize);
        pFontFace->m_mpCharInfo[i] = c;

        x += gs->bitmap.width;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

std::tuple<int, int, int, int> TextRenderer::MeasureText(const FontFaceHandle& hFontFace, std::string sText, float fScale) {
    if (hFontFace.IsValid() == false) {
        std::cerr << "Attempt to measure text with an invalid font handle, text: " << sText << std::endl;
        return {};
    }
    FontFace* pFontFace = GetFontFace(hFontFace);
    if (pFontFace == nullptr) {
        std::cerr << "Attempt to measure text with an invalid font, text: " << sText << std::endl;
        return {};
    }

    int nHeight = 0;
    int nyLowest = 0;
    float fxCursor = 0.0f;
    float fyCursor = 0.0f;

    for (char c : sText) {
        CharInfo ch = pFontFace->m_mpCharInfo[c];
        float x2 = fxCursor + ch.m_fBitmapLeft * fScale;
        float y2 = fyCursor + ch.m_fBitmapTop * fScale;
        float w = ch.m_fBitmapWidth * fScale;
        float h = ch.m_fBitmapHeight * fScale;

        /* Advance the cursor to the start of the next character */
        fxCursor += ch.m_fxAdvance * fScale;
        fyCursor += ch.m_fyAdvance * fScale;
        if (!w || !h) {
            continue;
        }
        if (y2 - h < nyLowest) {
            nyLowest = (int)(y2 - h);
        }
        if ((int)y2 > nHeight) {
            nHeight = (int)(y2);
        }
    }
    return std::make_tuple(0, nyLowest, (int)fxCursor, nHeight);
}

float TextRenderer::GetFontHeight(const FontFaceHandle& hFontFace) {
    if (hFontFace.IsValid() == false) {
        std::cerr << "Attempt to measure height of font face with an invalid font handle" << std::endl;
        return 0.0f;
    }
    FontFace* pFontFace = GetFontFace(hFontFace);
    if (pFontFace == nullptr) {
        std::cerr << "Attempt to measure height of font face with an invalid font" << std::endl;
        return 0.0f;
    }
	auto sz = MeasureText(hFontFace, "XxgyHW", 1.0f);
	return static_cast<float>(std::get<3>(sz) - std::get<1>(sz));
}

float TextRenderer::GetFontDescenderHeight(const FontFaceHandle& hFontFace) {
    if (hFontFace.IsValid() == false) {
        std::cerr << "Attempt to measure descender height of font face with an invalid font handle" << std::endl;
        return 0.0f;
    }
    FontFace* pFontFace = GetFontFace(hFontFace);
    if (pFontFace == nullptr) {
        std::cerr << "Attempt to measure descender height of font face with an invalid font" << std::endl;
        return 0.0f;
    }
    auto sz = MeasureText(hFontFace, "XxgyHW", 1.0f);
    return static_cast<float>( - std::get<1>(sz));
}


std::vector<ITextLine*> TextRenderer::CreateMultipleTextLines(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) {
    if (hFontFace.IsValid() == false) {
        std::cerr << "Attempt to create multiple text lines with an invalid font handle, text: " << sText << std::endl;
        return {};
    }
    FontFace* pFontFace = GetFontFace(hFontFace);
    if (pFontFace == nullptr) {
        std::cerr << "Attempt to create multiple text lines with an invalid font, text: " << sText << std::endl;
        return {};
    }

	std::vector<ITextLine*> vcTextLines;

    float fFontHeight = GetFontHeight(hFontFace);
    float fFontDescenderheight = GetFontDescenderHeight(hFontFace);

    // Note, this only works for ASCII/UTF-8
	// TODO Expand to use utf-16 or utf-32 strings
    int index = 0;
    int currentTextStartOffset = 0;
    int lastWhitespaceStart = -1;
    int lastWhitespaceEnd = -1;
    std::string currentText = "";
    bool bLastCharWasWhitespace = false;
    bool bForceBreakAfterWhitespace = false;
    float fLastMeasuredWidth = 0.0f;
    float fPrevMeasuredWidth = 0.0f;
    for(int index = 0;index< sText.length();++index) {
        char c = sText[index];

        if (c == ' ' || c == '\t' || c=='\n' || c=='\r') {
            if (c == '\n' || c == '\r') {
                bForceBreakAfterWhitespace = true;
            }
            // Could split here
            if (bLastCharWasWhitespace) {
                lastWhitespaceEnd = index;
				// ignore multiple whitespace
                continue;
            }
            lastWhitespaceStart = index;
            lastWhitespaceEnd = index;
            bLastCharWasWhitespace = true;
        }
        else {
            bLastCharWasWhitespace = false;
        }

        if (bForceBreakAfterWhitespace && !bLastCharWasWhitespace) {
            --index;
        }

        if (!bForceBreakAfterWhitespace) {
            currentText += c;
        }
		auto sz = MeasureText(hFontFace, currentText, fScale);
        fLastMeasuredWidth = static_cast<float>(std::get<2>(sz));
        // TODO Test
        // this was std::get<2>(sz);
		if (fLastMeasuredWidth > fWidth || (bForceBreakAfterWhitespace && !bLastCharWasWhitespace)) {
            float fWidthToUse = fLastMeasuredWidth;
            if (fLastMeasuredWidth > fWidth) {
                fWidthToUse = fPrevMeasuredWidth;
            }
			if (lastWhitespaceStart == -1) {
				// Split here

				auto pTL = dynamic_cast<TextLine*>(CreateTextLine(hFontFace, currentText, fWidth, fScale));
                pTL->SetWidth(fWidthToUse);
                pTL->SetHeight(fFontHeight, fFontDescenderheight);

				vcTextLines.push_back(pTL);
                currentTextStartOffset += static_cast<int>(currentText.length());

				currentText = "";
			}
			else {
				// Split at last whitespace
                std::string textForLine = currentText.substr(0, lastWhitespaceStart - currentTextStartOffset);
                auto pTL = dynamic_cast<TextLine*>(CreateTextLine(hFontFace, textForLine, fWidth, fScale));
                pTL->SetWidth(fWidthToUse);
                pTL->SetHeight(fFontHeight, fFontDescenderheight);
				vcTextLines.push_back(pTL);
                if (bForceBreakAfterWhitespace) {
                    currentText = "";
                }
                else {
                    currentText = currentText.substr(lastWhitespaceStart - currentTextStartOffset + 1);
                }
                currentTextStartOffset = lastWhitespaceEnd+1;
                lastWhitespaceStart = -1;
                lastWhitespaceEnd = -1;
			}
            bForceBreakAfterWhitespace = false;
            fPrevMeasuredWidth = 0.0f;
            fLastMeasuredWidth = 0.0f;
		}
        fPrevMeasuredWidth = fLastMeasuredWidth;
    }
    if (currentText.length() > 0) {
        // TODO Ensure this is fully tested, changing fWidth to fLastMeasuredWidth
        auto pTL = dynamic_cast<TextLine*>(CreateTextLine(hFontFace, currentText, fWidth, fScale));
        pTL->SetWidth(fLastMeasuredWidth);
        pTL->SetHeight(fFontHeight, fFontDescenderheight);

        vcTextLines.push_back(pTL);
    }
    return vcTextLines;
}

ITextLine* TextRenderer::CreateTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) {
    FontFace* pFontFace;

    if (!RenderSetup(hFontFace, std::make_tuple(0.0f,0.0f,0.0f), pFontFace)) {
        return {};
    }

    size_t nPointsLength = 6 * sText.length();
    point* pCoords = new point[nPointsLength];

    int nextVertex = 0;
    float fxEnd = fWidth;

    float fxCursor = 0;
    float fyCursor = 0;
    for (char c : sText) {
        CharInfo ch = pFontFace->m_mpCharInfo[c];
        float xCharOrigin = fxCursor + ch.m_fBitmapLeft * fScale;
        float yCharOrigin = fyCursor + ch.m_fBitmapTop * fScale;
        float w = ch.m_fBitmapWidth * fScale;
        float h = ch.m_fBitmapHeight * fScale;
        if (xCharOrigin + w >= fxEnd) {
            break;
        }

        fxCursor += ch.m_fxAdvance * fScale;
        fyCursor += ch.m_fyAdvance * fScale;

        if (!w || !h) {
            continue;
        }
        CreateQuadForChar(pCoords, nPointsLength, nextVertex, ch, pFontFace, xCharOrigin, yCharOrigin, w, h);
    }

    TextLine* pTextLine = RenderVerticesToTextLine(pFontFace, hFontFace, pCoords, nPointsLength, nextVertex);
    pTextLine->SetWidth(fxCursor);
    pTextLine->SetHeight(GetFontHeight(hFontFace), GetFontDescenderHeight(hFontFace));
    delete [] pCoords;
    return pTextLine;
}

ITextLine* TextRenderer::CreateEditableTextLine(const FontFaceHandle& hFontFace, std::string sText, float fWidth, float fScale) {
    FontFace* pFontFace;

    if (!RenderSetup(hFontFace, std::make_tuple(0.0f, 0.0f, 0.0f), pFontFace)) {
        return {};
    }

    size_t nPointsLength = 6 * sText.length();
    point* pCoords = new point[nPointsLength];

    int nextVertex = 0;
    float fxEnd = fWidth;

    float fxCursor = 0;
    float fyCursor = 0;
    std::vector<float> vcCharOffsets;
    vcCharOffsets.push_back(0.0f);
    for (char c : sText) {
        CharInfo ch = pFontFace->m_mpCharInfo[c];
        float xCharOrigin = fxCursor + ch.m_fBitmapLeft * fScale;
        float yCharOrigin = fyCursor + ch.m_fBitmapTop * fScale;
        float w = ch.m_fBitmapWidth * fScale;
        float h = ch.m_fBitmapHeight * fScale;
        //if (xCharOrigin + w >= fxEnd) {
        //    break;
        //}

        fxCursor += ch.m_fxAdvance * fScale;
        fyCursor += ch.m_fyAdvance * fScale;

        vcCharOffsets.push_back(fxCursor);

        if (!w || !h) {
            continue;
        }
        CreateQuadForChar(pCoords, nPointsLength, nextVertex, ch, pFontFace, xCharOrigin, yCharOrigin, w, h);
    }

    TextLine* pTextLine = RenderVerticesToTextLine(pFontFace, hFontFace, pCoords, nPointsLength, nextVertex);
    pTextLine->SetCharOffsets(vcCharOffsets);
    pTextLine->SetWidth(fxCursor);
    pTextLine->SetHeight(GetFontHeight(hFontFace), GetFontDescenderHeight(hFontFace));
    delete[] pCoords;
    return pTextLine;
}

void TextRenderer::RenderTextLine(const ITextLine* pITextLine, float fX, float fY, std::tuple<float, float, float> rgbText) {
	const TextLine* pTextLine = dynamic_cast<const TextLine*>(pITextLine);
    if (!pTextLine->IsValid()) {
        return;
    }
    if (!pTextLine->GetFontFaceHandle().IsValid()) {
        return;
    }
    unsigned int unVAO = pTextLine->GetVAO();
    unsigned int unVBO = pTextLine->GetVBO();
    unsigned int unVertexCount = pTextLine->GetVertexCount();

    FontFace* pFontFace = GetFontFace(pTextLine->GetFontFaceHandle());
    if (pFontFace == nullptr) {
        return;
    }

    glDisable(GL_DEPTH_TEST);
    m_shader.Use();
    m_shader.SetVec3("textColour", rgbText);

    glm::mat4 transform(1);
    transform = glm::translate(transform, glm::vec3(fX, fY, 0));
    m_shader.SetMat4("transform", transform);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pFontFace->m_unAtlasTextureId);
    glBindVertexArray(unVAO);

    glBindBuffer(GL_ARRAY_BUFFER, unVBO);
    glDrawArrays(GL_TRIANGLES, 0, unVertexCount);
    glEnable(GL_DEPTH_TEST);
}


void TextRenderer::RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fWidth, float fScale, std::tuple<float, float, float> rgbText) {
    FontFace* pFontFace;
    if (!RenderSetup(hFontFace, rgbText, pFontFace)) {
        return;
    }

    size_t nPointsLength = 6 * sText.length();
    point* pCoords = new point[nPointsLength];

    int nextVertex = 0;
    float fxEnd = fX + fWidth;

    float fxCursor = fX;
    float fycursor = fY;
    for (char c : sText) {
        CharInfo ch = pFontFace->m_mpCharInfo[c];
        float xCharOrigin = fxCursor + ch.m_fBitmapLeft * fScale;
        float yCharOrigin = fycursor + ch.m_fBitmapTop * fScale;
        float w = ch.m_fBitmapWidth * fScale;
        float h = ch.m_fBitmapHeight * fScale;
        if (xCharOrigin + w >= fxEnd) {
            break;
        }

        fxCursor += ch.m_fxAdvance * fScale;
        fycursor += ch.m_fyAdvance * fScale;

        if (!w || !h) {
            continue;
        }
        CreateQuadForChar(pCoords, nPointsLength, nextVertex, ch, pFontFace, xCharOrigin, yCharOrigin, w, h);
    }

    RenderVertices(pFontFace, pCoords, nPointsLength, nextVertex);
    delete[]pCoords;
}

void TextRenderer::RenderText(const FontFaceHandle& hFontFace, std::string sText, float fX, float fY, float fScale, std::tuple<float, float, float> rgbText) {
    FontFace* pFontFace;
    if (!RenderSetup(hFontFace, rgbText, pFontFace)) {
        return;
    }

    size_t nPointsLength = 6 * sText.length();
    point* pCoords = new point[nPointsLength];

    int nextVertex = 0;

    float fxCursor = fX;
    float fycursor = fY;
    for (char c : sText) {
        CharInfo ch = pFontFace->m_mpCharInfo[c];
        float xCharOrigin = fxCursor + ch.m_fBitmapLeft * fScale;
        float yCharOrigin = fycursor + ch.m_fBitmapTop * fScale;
        float w = ch.m_fBitmapWidth * fScale;
        float h = ch.m_fBitmapHeight * fScale;

        fxCursor += ch.m_fxAdvance * fScale;
        fycursor += ch.m_fyAdvance * fScale;

        if (!w || !h) {
            continue;
        }
        CreateQuadForChar(pCoords, nPointsLength, nextVertex, ch,pFontFace, xCharOrigin, yCharOrigin, w, h);
    }

    RenderVertices(pFontFace, pCoords, nPointsLength, nextVertex);
    delete[]pCoords;
}

bool TextRenderer::RenderSetup(const FontFaceHandle& hFontFace, std::tuple<float, float, float> rgbText, FontFace*& prefFontFace) {
    if (hFontFace.IsValid() == false) {
        std::cerr << "Attempt to render text with an invalid font handle" << std::endl;
        return false;
    }
    prefFontFace = GetFontFace(hFontFace);
    if (prefFontFace == nullptr) {
        std::cerr << "Attempt to render text with an invalid font face" << std::endl;
        return false;
    }
    glDisable(GL_DEPTH_TEST);
    m_shader.Use();
    m_shader.SetVec3("textColour", rgbText);
    m_shader.SetMat4("transform", glm::mat4(1));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void TextRenderer::RenderVertices(FontFace* pFontFace, point* pCoords, size_t nPointsLength, int nVertexCount) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pFontFace->m_unAtlasTextureId);
    glBindVertexArray(m_unVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_unVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * nPointsLength, pCoords, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, nVertexCount);
    glEnable(GL_DEPTH_TEST);
}

TextLine* TextRenderer::RenderVerticesToTextLine(FontFace* pFontFace, const FontFaceHandle& hFontFace, point* pCoords, size_t nPointsLength, int nVertexCount) {
    unsigned int unVAO = 0;
    unsigned int unVBO = 0;
    glGenVertexArrays(1, &unVAO);
    glBindVertexArray(unVAO);

    glGenBuffers(1, &unVBO);
    glBindBuffer(GL_ARRAY_BUFFER, unVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * nPointsLength, pCoords, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    return new TextLine(unVAO, unVBO, nVertexCount, hFontFace);
}

void TextRenderer::CreateQuadForChar(point* pCoords, size_t nPointsLength, int& nNextVertex, const CharInfo& ch, FontFace* pFontFace, float x, float y, float w, float h) {
    if (nNextVertex > nPointsLength - 6) {
        return;
    }
    float txLeft = ch.m_fxTexture;
    float txRight = txLeft + ch.m_fBitmapWidth / std::get<0>(pFontFace->m_tpAtlasSize);
    float tyBottom = 0.0f;
    float tyTop = tyBottom + ch.m_fBitmapHeight / std::get<1>(pFontFace->m_tpAtlasSize);

    pCoords[nNextVertex++] = { x,     y    , txLeft, tyBottom };
    pCoords[nNextVertex++] = { x,     y - h, txLeft, tyTop };
    pCoords[nNextVertex++] = { x + w, y    , txRight, tyBottom };
    pCoords[nNextVertex++] = { x + w, y    , txRight, tyBottom };
    pCoords[nNextVertex++] = { x,     y - h, txLeft,  tyTop };
    pCoords[nNextVertex++] = { x + w, y - h, txRight, tyTop };
}