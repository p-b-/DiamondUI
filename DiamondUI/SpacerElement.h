#pragma once
#include "OverlayDefs.h"
#include "IOverlayElement.h"
#include "OverlayGraphics.h"
#include "IOverlayIntl.h"

class IOverlayEnvIntl;

class SpacerElement :
    public IOverlayElement, public OverlayGraphics
{
public:
    SpacerElement(IOverlayIntl* pOverlay, unsigned int unHeight, unsigned int unDividerType = 0);
    virtual ~SpacerElement();

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual int GetId() const { return -1; }
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual float GetPreferredControlWidth() const;
    virtual void SetPreferredControlWidth(float fWidth) {}

    virtual float GetLastRenderedYOrigin() const { return m_yLastRenderedOrigin; }
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual bool RenderedYet() const { return m_bRenderedAtLeastOnce; }
    virtual bool Disabled() { return true; }
    virtual void Disable(bool bDisable) {}
    virtual bool CanHighlight() { return false; }
    virtual void Highlight(bool bHighlight) {}
    virtual float GetScrollIncrement();
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue) {}
    virtual OverlayControlType GetControlType() const { return eOCTSpacer; }
    virtual void ShowDebugBackground(bool bShow);
    virtual ActionResult Action(unsigned int unAction) { return false; }
    virtual bool Altered() const { return false; }
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save() {}
    virtual void Revert() {}
    virtual void ResetDisplayState() {}

private:
    IOverlayIntl* m_pOverlayWithin;
    const IOverlayEnvIntl* m_pOverlayEnv;
    float m_fSpacerHeight;
    unsigned int m_unDividerType;

    float m_yLastRenderedOrigin;

    bool m_bRenderedAtLeastOnce;

    bool m_bShowDebugBackground;

    bool m_bDisabled;
    bool m_bHighlighted;
};

