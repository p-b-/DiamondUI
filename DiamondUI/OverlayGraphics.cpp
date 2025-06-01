#include "OverlayGraphics.h"
#include "Textures.h"
#include "OverlayEnv.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define SELECTOR_BUTTON_WIDTH 50
#define SELECTOR_BUTTON_HEIGHT 30

fnRenderButton OverlayGraphics::s_pRenderButtonFunc = nullptr;
fnRenderSelector OverlayGraphics::s_pRenderSelectorFunc = nullptr;

float OverlayGraphics::s_fTextDescender = 0.0f;

unsigned int OverlayGraphics::s_unVAOQuad = 0;
unsigned int OverlayGraphics::s_unVBOQuad = 0;
unsigned int OverlayGraphics::s_unVAOHighlightOutline = 0;
unsigned int OverlayGraphics::s_unVBOHighlightOutline = 0;
unsigned int OverlayGraphics::m_unVAOGlyph = 0;
unsigned int OverlayGraphics::m_unVBOGlyph = 0;
unsigned int OverlayGraphics::s_unVAOButtonOutline = 0;
unsigned int OverlayGraphics::s_unVBOButtonOutline = 0;

unsigned int OverlayGraphics::s_unVAOLeftButton = 0;
unsigned int OverlayGraphics::s_unVBOLeftButton = 0;
unsigned int OverlayGraphics::s_unVerticesCountInArrow;

int OverlayGraphics::s_nScreenWidth;
int OverlayGraphics::s_nScreenHeight;
Shader OverlayGraphics::s_shader;
Shader OverlayGraphics::s_shaderGlyph;
ITextRenderer* OverlayGraphics::s_pTextRenderer;
std::stack<OverlayGraphics::ScissorState> OverlayGraphics::s_stScissorState;
Textures* OverlayGraphics::s_pTextureCtrl;
unsigned int OverlayGraphics::s_unGlyphAtlasId;
unsigned int OverlayGraphics::s_unGlyphTextureId;

std::list<unsigned int> OverlayGraphics::s_lsVAO;
std::list<unsigned int> OverlayGraphics::s_lsVBO;

void OverlayGraphics::InitialiseOverlayGraphics(int nScreenWidth, int nScreenHeight) {
    // If this is being re-called, delete previous buffers
    Deinitialise();
    float rectangleVertices[] = {
        0,1,
        0,0,
        1,1,
        1,0
    };
    InitialiseVectorBuffer(rectangleVertices, 4, &s_unVAOQuad, &s_unVBOQuad);
    float highlightVertices[] = {
        0.0001f, 1,
        0.0001f,0.0001f,
        1,0.0001f,
        1,1
    };
    InitialiseVectorBuffer(highlightVertices, 4, &s_unVAOHighlightOutline, &s_unVBOHighlightOutline);
    
    float glyphVertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    };
    InitialiseVectorWithTexturesBuffer(glyphVertices, 4, &m_unVAOGlyph, &m_unVBOGlyph);

    float buttonOutlineVertices[] = {
    0,1,
    0,0.25f,
    0.25f,0,
    1,0,
    1,0.75f,
    0.75f,1
    };

    InitialiseVectorBuffer(buttonOutlineVertices, 6, &s_unVAOButtonOutline, &s_unVBOButtonOutline);

    float leftArrowVertices[] = {
        0.0f,0.49f,
        0.0f,0.51f,
        0.2f,0.4f,
        0.2f,0.6f,
        0.4f,0.3f,
        0.4f,0.7f,
        0.42f,0.3f,
        0.42f,0.7f,
        0.6f,0.2f,
        0.6f,0.8f,
        0.62f,0.2f,
        0.62f,0.8f,
        0.8f,0.1f,
        0.8f,0.9f,
        0.82f, 0.1f,
        0.82f,0.9f,
        0.84f, 0.1f,
        0.84f,0.9f,
        1.0f,0.0f,
        1.0f,1.0f
    };
    s_unVerticesCountInArrow = sizeof(leftArrowVertices) / (sizeof(float)*2);
    InitialiseVectorBuffer(leftArrowVertices, s_unVerticesCountInArrow, &s_unVAOLeftButton, &s_unVBOLeftButton);
    
    s_shader.InitialiseShaders("overlay_vert.glsl", "overlay_frag.glsl");
    s_shaderGlyph.InitialiseShaders("glyph_vert.glsl", "glyph_frag.glsl");

    UpdateOrthProjection(nScreenWidth, nScreenHeight);
    s_shader.Use();
    s_shader.SetVec4("overlayColour", glm::vec4(0.0f, 0.0f, 1.0f, 0.2f));
}

void OverlayGraphics::InitialiseVectorBuffer(float* pVertices, int nCount, unsigned int* punVAO, unsigned int* punVBO) {
    glGenVertexArrays(1, punVAO);
    glGenBuffers(1, punVBO);
    glBindVertexArray(*punVAO);
    glBindBuffer(GL_ARRAY_BUFFER, *punVBO);
    glBufferData(GL_ARRAY_BUFFER, nCount * sizeof(float) * 2, pVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    s_lsVAO.push_back(*punVAO);
    s_lsVBO.push_back(*punVBO);
}

void OverlayGraphics::InitialiseVectorWithTexturesBuffer(float* pVertices, int nCount, unsigned int* punVAO, unsigned int* punVBO) {
    glGenVertexArrays(1, punVAO);
    glGenBuffers(1, punVBO);
    glBindVertexArray(*punVAO);
    glBindBuffer(GL_ARRAY_BUFFER, *punVBO);
    glBufferData(GL_ARRAY_BUFFER, nCount * sizeof(float) * 4, pVertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    s_lsVAO.push_back(*punVAO);
    s_lsVBO.push_back(*punVBO);
}

void OverlayGraphics::Deinitialise() {
    for (unsigned int vao : s_lsVAO) {
        glDeleteVertexArrays(1, &vao);
    }
    for (unsigned int vbo : s_lsVAO) {
        glDeleteBuffers(1, &vbo);
    }
}

void OverlayGraphics::SetTextureCtrl(Textures* pTextureCtrl) {
    s_pTextureCtrl = pTextureCtrl;
}

void OverlayGraphics::SetGlyphAtlasId(unsigned int unGlypthAtlasId) {
    s_unGlyphAtlasId = unGlypthAtlasId;
    if (unGlypthAtlasId == 0) {
        s_unGlyphAtlasId = 0;
    }
    else {
        s_unGlyphTextureId = s_pTextureCtrl->GetTextureIdForTextureAtlas(s_unGlyphAtlasId);
    }
}

void OverlayGraphics::UpdateOrthProjection(int nScreenWidth, int nScreenHeight) {
    s_nScreenWidth = nScreenWidth;
    s_nScreenHeight = nScreenHeight;

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(nScreenWidth), 0.0f, static_cast<float>(nScreenHeight));
    s_shader.Use();
    s_shader.SetMat4("projection", projection);
    s_shaderGlyph.Use();
    s_shaderGlyph.SetMat4("projection", projection);
}

void OverlayGraphics::SetButtonRenderFunc(fnRenderButton pFunc) {
    s_pRenderButtonFunc = pFunc;
}

void OverlayGraphics::SetSelectorRenderFunc(fnRenderSelector pFunc) {
    s_pRenderSelectorFunc = pFunc;
}

void OverlayGraphics::RenderDebugBackground(int nElementId, float fX, float fY, float fWidth, float fHeight, rgba rgbaOverlay) {
    RenderTranslucentOverlay(fX, fY, fWidth, fHeight, rgbaOverlay);
}

void OverlayGraphics::RenderTextEditField(int nElementId, float fX, float fY, float fWidth, float fHeight, rgba col) {
    RenderButtonOutline(fX, fY, fWidth, fHeight, col);
}

void OverlayGraphics::RenderCaret(int nElementId, float fX, float fY, float fHeight, rgba rgbaCaret) {
    RenderTranslucentOverlay(fX, fY, 2.0f, fHeight, rgbaCaret);
}

void OverlayGraphics::RenderTranslucentOverlay(int nX, int nY, int nWidth, int nHeight, rgba rgbaOverlay) {
    RenderTranslucentOverlay(static_cast<float>(nX), static_cast<float>(nY), static_cast<float>(nWidth), static_cast<float>(nHeight), rgbaOverlay);
}

void OverlayGraphics::RenderTranslucentOverlay(float fX, float fY, float fWidth, float fHeight, rgba rgbaOverlay) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);
    glm::mat4 transform = glm::mat4(1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);


    transform = glm::translate(transform, glm::vec3(fX, fY, 0));
    transform = glm::scale(transform, glm::vec3(fWidth - 1, fHeight - 1, 0));
    s_shader.SetMat4("transform", transform);
    s_shader.SetVec4("overlayColour", rgbaOverlay);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void OverlayGraphics::RenderOutline(int nX, int nY, int nWidth, int nHeight, rgba col) {
    s_shader.Use();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(s_unVAOHighlightOutline);
    glm::mat4 transform = glm::mat4(1.0f);

    transform = glm::translate(transform, glm::vec3(nX, nY, 0));
    transform = glm::scale(transform, glm::vec3((float)nWidth - 1, (float)nHeight - 1, 0));
    s_shader.SetMat4("transform", transform);
    s_shader.SetVec4("overlayColour", col);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void OverlayGraphics::RenderSelector(const IOverlayEnv* pEnv, int nElementId, float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, ITextLine* pTLValue) {
    if (s_pRenderSelectorFunc != nullptr) {
        s_pRenderSelectorFunc(pEnv, nElementId, fX, fY, fWidth, fHeight, elementState, pTLValue);
        return;
    }
    rgb textRGB = pEnv->GetTextRGB();
    rgba rgbaVector = pEnv->GetVectorColour(elementState.m_bHighlighted);

    float fTextDescenderHeight = GetTextDescender(pEnv);
    float yUnusedVerticalPaddingInArrow = 7;
    float yArrowPoint = fY - yUnusedVerticalPaddingInArrow + fTextDescenderHeight;
    RenderLeftArrow(fX, yArrowPoint, SELECTOR_BUTTON_WIDTH, SELECTOR_BUTTON_HEIGHT, rgbaVector);
    RenderRightArrow(fX + fWidth - SELECTOR_BUTTON_WIDTH, yArrowPoint, SELECTOR_BUTTON_WIDTH, SELECTOR_BUTTON_HEIGHT, rgbaVector);

    pTLValue->GetWidth();
    float fXLabel = fX + (fWidth - pTLValue->GetWidth()) / 2;
    float fYLabel = fY + (fHeight - (pTLValue->GetHeight() - fTextDescenderHeight)) / 2;

    s_pTextRenderer->RenderTextLine(pTLValue, fXLabel, fYLabel, textRGB);
}

void OverlayGraphics::RenderButton(const IOverlayEnv* pEnv, int nElementId, float fX, float fY, float fWidth, float fHeight, const OverlayElementState& elementState, OverlayAnimation eAnim, float fAnimationSeqPt, ITextLine* pTLText) {
    if (s_pRenderButtonFunc != nullptr) {
		s_pRenderButtonFunc(pEnv, nElementId, fX, fY, fWidth, fHeight, elementState, pTLText);
        return;
    }
    bool bHighlight = elementState.m_bHighlighted;
    if (elementState.m_bActivated && elementState.m_bEnabled) {
        bHighlight = false;
    }  
    rgba rgbaOutline = pEnv->GetVectorColour(bHighlight && elementState.m_bEnabled);

    if (elementState.m_bActivated && elementState.m_bEnabled) {
        // TODO Make this colour configurable
        rgbaOutline = MakeRGBA(0.8f, 0.2f, 0.2f, 0.8f);
    }
    rgb rgbText = pEnv->GetTextRGB();
    if (!elementState.m_bEnabled) {
        rgbText = pEnv->GetDisabledTextRGB();
    }
    // TODO Make this colour configurable
    rgba rgbaTextBackground = MakeRGBA(0.1f, 0.1f, 0.5f, 0.8f);
    if (!elementState.m_bEnabled) {
        // TODO Make this colour configurable
        rgbaTextBackground = MakeRGBA(0.0f, 0.0f, 0.0f, 0.0f);
    }

    RenderButtonOutline(fX, fY, fWidth, fHeight, rgbaOutline);
    float fXLabel = fX + (fWidth- pTLText->GetWidth())/2;
    float fDescenderHeight = pTLText->GetDescenderHeight();
    float fHeightWithoutDescender = pTLText->GetHeight() - fDescenderHeight;
    float fYLabel = fY + (fHeight - fHeightWithoutDescender) / 2;
    RenderTranslucentOverlay(fXLabel - fDescenderHeight, fYLabel - fDescenderHeight, pTLText->GetWidth() + 2 * fDescenderHeight, fHeightWithoutDescender + 2 * fDescenderHeight, rgbaTextBackground);
    s_pTextRenderer->RenderTextLine(pTLText, fXLabel, fYLabel, rgbText);
}

void OverlayGraphics::RenderButtonOutline(float fX, float fY, float fWidth, float fHeight, rgba col) {
    s_shader.Use();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindVertexArray(s_unVAOButtonOutline);

    float fCorner = 10.0f;
    float buttonOutlineVertices[] = {
        fX,fY + fHeight,
        fX,fY + fCorner,
        fX + fCorner,fY,
        fX + fWidth,fY,
        fX + fWidth,fY + fHeight - fCorner,
        fX + fWidth - fCorner,fY + fHeight
    };
    glBindBuffer(GL_ARRAY_BUFFER, s_unVBOButtonOutline);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, &buttonOutlineVertices, GL_DYNAMIC_DRAW);

    glm::mat4 transform = glm::mat4(1.0f);
    s_shader.SetMat4("transform", transform);
    s_shader.SetVec4("overlayColour", col);
    glDrawArrays(GL_LINE_LOOP, 0, 6);
}

void OverlayGraphics::RenderDivider(unsigned int unDivider, float fX, float fY, float fWidth, float fHeight) {

}

bool OverlayGraphics::RenderGlypth(float fXOrigin, float fYOrigin, float fDrawWidth, float fDrawHeight, unsigned int unAction) {
    if (s_unGlyphAtlasId == 0) {
        return false;
    }
    float fX;
    float fY;
    float fWidth;
    float fHeight;
    s_pTextureCtrl->GetTexelForTextureInAtlas(s_unGlyphAtlasId, unAction, &fX, &fY, &fWidth, &fHeight);

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(fXOrigin, fYOrigin, 0));
    transform = glm::scale(transform, glm::vec3(fDrawWidth - 1, fDrawHeight- 1, 0));
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    s_shaderGlyph.Use();
    s_shaderGlyph.SetMat4("transform", transform);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s_unGlyphTextureId);

    glBindVertexArray(m_unVAOGlyph);

    float glyphVertices[] = {
     0.0f,  1.0f,   fX, fY + fHeight,
     0.0f,  0.0f,   fX, fY,
     1.0f,  1.0f,   fX + fWidth, fY + fHeight,
     1.0f,  0.0f,   fX + fWidth, fY
    };
    glBindBuffer(GL_ARRAY_BUFFER, m_unVBOGlyph);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, &glyphVertices, GL_DYNAMIC_DRAW);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    return true;
}

void OverlayGraphics::RenderLeftArrow(float nX, float nY, float nWidth, float nHeight, rgba rgbaArrow) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float xStep = nWidth / 8;
    float yCentre = nHeight / 2;

    float x = nX;
    float y = nY + yCentre;
    float nArrowletteWidth = 3;
    float dY = 2;
    rgbaArrow = ChangeAlpha(rgbaArrow, -0.5f);
    
    for (int n = 0; n < 8; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(x, y - dY, 0));
        transform = glm::scale(transform, glm::vec3((float)nArrowletteWidth, (float)dY*2, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaArrow);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        x += xStep;
        if (n < 3) {
            dY += 2;
        }
        if (n < 4) {
            rgbaArrow = ChangeAlpha(rgbaArrow, 0.125f);
        }
        if (n == 4 || n == 5) {
            ++nArrowletteWidth;
        }
    }
}

void OverlayGraphics::RenderRightArrow(float nX, float nY, float nWidth, float nHeight, rgba rgbaArrow) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float fXStep = nWidth / 8;
    float fYCentre = nHeight / 2;

    float fY = nY + fYCentre;
    float fArrowletteWidth = 3;
    float fX = nX + nWidth - fArrowletteWidth;
    float fDY = 2;
    rgbaArrow = ChangeAlpha(rgbaArrow, -0.5f);

    for (int n = 0; n < 8; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(fX, fY - fDY, 0));
        transform = glm::scale(transform, glm::vec3(fArrowletteWidth, fDY * 2, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaArrow);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        fX -= fXStep;
        if (n < 3) {
            fDY += 2;
        }
        if (n < 4) {
            rgbaArrow = ChangeAlpha(rgbaArrow, 0.125f);
        }
        if (n == 4 || n == 5) {
            fArrowletteWidth+=1.0;
        }
    }
}

void OverlayGraphics::RenderUpArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float fYStep = fHeight / 8;
    float fXCentre = fWidth/ 2;
    float fArrowletteHeight = 3;
    fY = fY + fHeight- fArrowletteHeight;
    fX = fX + fXCentre;
    float fDX = 2;
    rgbaArrow = ChangeAlpha(rgbaArrow, -0.5f);

    for (int n = 0; n < 8; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(fX - fDX - 1, fY, 0));
        transform = glm::scale(transform, glm::vec3(fDX * 2, fArrowletteHeight, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaArrow);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        fY -= fYStep;
        if (n < 3) {
            fDX += 2;
        }
        if (n < 4) {
            rgbaArrow = ChangeAlpha(rgbaArrow, 0.125f);
        }
        if (n == 4 || n == 5) {
            fArrowletteHeight += 1.0;
        }
    }
}

void OverlayGraphics::RenderDownArrow(float fX, float fY, float fWidth, float fHeight, rgba rgbaArrow) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float fYStep = fHeight / 8;
    float fXCentre = fWidth / 2;
    float fArrowletteHeight = 3;
    fY = fY;
    fX = fX + fXCentre;
    float fDX = 2;
    rgbaArrow = ChangeAlpha(rgbaArrow, -0.5f);

    for (int n = 0; n < 8; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(fX - fDX - 1, fY, 0));
        transform = glm::scale(transform, glm::vec3(fDX * 2, fArrowletteHeight, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaArrow);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        fY += fYStep;
        if (n < 3) {
            fDX += 2;
        }
        if (n < 4) {
            rgbaArrow = ChangeAlpha(rgbaArrow, 0.125f);
        }
        if (n == 4 || n == 5) {
            fArrowletteHeight += 1.0;
        }
    }
}

void OverlayGraphics::DrawButtonBackground(float fX, float fY, float fWidth, float fHeight, rgba rgbaButton) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float fXStep = 5;

    float fStripeWidth = 2;
    rgbaButton = ChangeAlpha(rgbaButton, -0.6f);

    int nStepCount = static_cast<int>(fWidth / fXStep);

    int nStage1 = 4;
    int nStage2 = 9;
    int nStage3 = nStepCount - 9;
    int nStage4 = nStage3 + 5;


    for (int n = 0; n < nStepCount; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(fX, fY, 0));
        transform = glm::scale(transform, glm::vec3(fStripeWidth, (float)fHeight, 0));
        //transform = glm::translate(transform, glm::vec3(fX, fYCentre-fDY, 0));
        //transform = glm::scale(transform, glm::vec3(fStripeWidth, 2*fDY, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaButton);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        fX += fXStep;
        if (n < nStage1) {
            rgbaButton = ChangeAlpha(rgbaButton, 0.125f);
        }
        else if (n < nStage2) {
            rgbaButton = ChangeAlpha(rgbaButton, -0.125f);
        }
        else if (n > nStage4) {
            rgbaButton = ChangeAlpha(rgbaButton, -0.125f);
        }
        else if (n > nStage3) {
            rgbaButton = ChangeAlpha(rgbaButton, 0.125f);
        }
        if (n == nStage1 || n == nStage1-1) {
            fStripeWidth += 1.0f;
        }
        else if (n == nStage2) {
            fStripeWidth -= 1.0f;
        }
        else if (n == nStage3) {
            fStripeWidth += 1.0f;
        } 
        else if (n == nStage4 || n == nStage4 + 1) {
            fStripeWidth -= 1.0f;
        }
    }
}

void OverlayGraphics::DrawSelectorBackground(float fX, float fY, float fWidth, float fHeight, rgba rgbaButton) {
    s_shader.Use();
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray(s_unVAOQuad);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    float fXStep = 5;

    float fStripeWidth = 2;
    rgbaButton = ChangeAlpha(rgbaButton, -0.6f);

    int nStepCount = static_cast<int>(fWidth / fXStep);

    int nStepCentre = nStepCount / 2;

    int nArrowStage = 4;
    int nPostArrowStage = 5;

    //int nGrowForLabelStage = 10;
    int nGrowForLabelStage = nStepCentre-nArrowStage-nPostArrowStage - 10;
    int nGrownForLabelStage = 4;
    int nStage1 = nArrowStage;
    int nStage2 = nStage1 + nPostArrowStage;
    int nStage3 = nStage2 + nGrowForLabelStage;
    int nStage4 = nStage3 + nGrownForLabelStage;
    int nStage5 = nStepCount - nArrowStage - nPostArrowStage - nGrowForLabelStage - nGrownForLabelStage - 1;
    int nStage6 = nStepCount - nArrowStage - nPostArrowStage - nGrowForLabelStage - 1;
    int nStage7 = nStepCount - nArrowStage - nPostArrowStage - 1;
    int nStage8 = nStepCount - nArrowStage-1;

    float fYCentre = fY + fHeight / 2;
    float fDY = 2;

    for (int n = nStage3; n < nStage7; ++n) {
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, glm::vec3(fX, fYCentre - fDY, 0));
        transform = glm::scale(transform, glm::vec3(fStripeWidth, 2 * fDY, 0));
        s_shader.SetMat4("transform", transform);
        s_shader.SetVec4("overlayColour", rgbaButton);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        fX += fXStep;
        if (n < nStage1) {
            fDY += 2;
        }
        /*else if (n < nStage4 && n >= nStage3) {
            fDY += 2;
        }
        else if (n >= nStage5 && n < nStage6) {
            fDY -= 2;
        }*/
        else if (n >= nStage8) {
            fDY -= 2;
        }
        if (n < nStage1) {
            rgbaButton = ChangeAlpha(rgbaButton, 0.125f);
        }
        else if (n < nStage2) {
            rgbaButton = ChangeAlpha(rgbaButton, -0.125f);
        }
        else if (n > nStage8) {
            rgbaButton = ChangeAlpha(rgbaButton, -0.125f);
        }
        else if (n > nStage7) {
            rgbaButton = ChangeAlpha(rgbaButton, 0.125f);
        }
        if (n == nStage1 || n == nStage1 - 1) {
            fStripeWidth += 1.0f;
        }
        else if (n == nStage2) {
            fStripeWidth -= 1.0f;
        }
        else if (n == nStage7) {
            fStripeWidth += 1.0f;
        }
        else if (n == nStage8 || n == nStage8 + 1) {
            fStripeWidth -= 1.0f;
        }
    }
}

float OverlayGraphics::GetTextDescender(const IOverlayEnv* pEnv) {
	if (s_fTextDescender == 0.0f) {
		FontFaceHandle hTextFont = pEnv->GetTextFontFace();
		s_fTextDescender = s_pTextRenderer->GetFontDescenderHeight(hTextFont);
	}
	return s_fTextDescender;
}

void OverlayGraphics::SetClippingArea(float fX, float fY, float fWidth, float fHeight) {
    SetClippingArea(static_cast<int>(fX), static_cast<int>(fY), static_cast<int>(fWidth), static_cast<int>(fHeight));
}

void OverlayGraphics::SetClippingArea(int nX, int nY, int nWidth, int nHeight) {
    int nxCurrent = 0;
    int nyCurrent = 0;
    int widthCurrent = s_nScreenWidth;
    int heightCurrent = s_nScreenHeight;
    if (s_stScissorState.size() > 0) {
        auto sc = s_stScissorState.top();
		nxCurrent = sc.m_nX;
        nyCurrent = sc.m_nY;
		widthCurrent = sc.m_nWidth;
        heightCurrent = sc.m_nHeight;
    }

	if (nX < nxCurrent) {
        int diff = nxCurrent - nX;
		nX = nxCurrent;
		nWidth -= diff;
	}
    if (nX + nWidth > nxCurrent + widthCurrent) {
		nWidth = nxCurrent + widthCurrent - nX;
    }

    if (nY < nyCurrent) {
        int diff = nyCurrent - nY;
        nY = nyCurrent;
		nHeight -= diff;
    }
    if (nY + nHeight > nyCurrent + heightCurrent) {
		nHeight = nyCurrent + heightCurrent - nY;
    }
    s_stScissorState.push({ true,nX,nY,nWidth,nHeight});
	glScissor(nX, nY, nWidth, nHeight);
    glEnable(GL_SCISSOR_TEST);
}

void OverlayGraphics::RemoveClippingArea() {
    s_stScissorState.pop();
    if (s_stScissorState.size() == 0) {
		glDisable(GL_SCISSOR_TEST);
	}
	else {
		auto sc = s_stScissorState.top();
		glScissor(sc.m_nX, sc.m_nY, sc.m_nWidth, sc.m_nHeight); 
    }
}