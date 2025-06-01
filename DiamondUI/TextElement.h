#pragma once
#include "OverlayDefs.h"
#include "IOverlayElement.h"
#include "OverlayGraphics.h"
#include "IOverlayIntl.h"
#include "ITextRenderer.h"

#include <string>

class IOverlayEnvIntl;

class TextElement :
    public IOverlayElement, public OverlayGraphics
{
public:
    TextElement(IOverlayIntl* pOverlay, int nElementId, std::string sText, bool bSingleLine = false);
    virtual ~TextElement();

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual int GetId() const { return m_nElementId; }
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual float GetPreferredControlWidth() const;
    virtual void SetPreferredControlWidth(float fWidth) { m_fPreferredWidth = fWidth; }
    virtual float GetLastRenderedYOrigin() const { return m_yLastRenderedOrigin; }
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual bool RenderedYet() const { return m_bRenderedAtLeastOnce;  }
    virtual bool Disabled() { return m_bDisabled; }
    virtual void Disable(bool bDisable);
    virtual bool CanHighlight() { return false; }
    virtual void Highlight(bool bHighlight);
    virtual float GetScrollIncrement();
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {}
    virtual OverlayControlType GetControlType() const { return eOCTLabel; }
    virtual void ShowDebugBackground(bool bShow);
    virtual ActionResult Action(unsigned int unAction) { return false; }
    virtual bool Altered() const { return false; }
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save() {}
    virtual void Revert() {}
    virtual void ResetDisplayState() {}

protected:
    IOverlayIntl* m_pOverlayWithin;
    const IOverlayEnvIntl* m_pOverlayEnv;
    int m_nElementId;
    std::string m_sText;
    bool m_bSingleLine;
    std::vector<ITextLine*> m_vcTextLines;
    float m_fLineSpacing;
    float m_fFontHeight;
    float m_fTextDescender;

    float m_yLastRenderedOrigin;
    bool m_bRenderedAtLeastOnce;
    float m_fPreferredWidth;

    bool m_bShowDebugBackground;

    bool m_bDisabled;
    bool m_bHighlighted;
};
