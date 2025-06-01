#pragma once
#include "OverlayDefs.h"
#include "IOverlayElement.h"
#include "OverlayGraphics.h"
#include "IOverlayIntl.h"
#include "ITouchTarget.h"

class OverlayTab;
class IOverlayEnvIntl;
class ScrollTarget;

class ScrollbarElement :
    public IOverlayElement, public OverlayGraphics, public ITouchTarget
{
public:
    ScrollbarElement(IOverlayIntl* pOverlay, ScrollTarget* pTab, int nElementId, float fHeight, int nTotalContentHeight, bool bForViewNotTab);
    virtual ~ScrollbarElement();

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual int GetId() const { return m_nElementId; }
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual float GetPreferredControlWidth() const;
    virtual void SetPreferredControlWidth(float fWidth) {}
    virtual float GetLastRenderedYOrigin() const { return m_yLastRenderedOrigin; }
    virtual bool RenderedYet() const { return m_bRenderedAtLeastOnce;  }
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual bool Disabled() { return false; }
    virtual void Disable(bool bDisable) {}
    virtual bool CanHighlight() { return false; }
    virtual void Highlight(bool bHighlight) {}
    virtual float GetScrollIncrement() { return 0.0f; }
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue);
    virtual OverlayControlType GetControlType() const { return eCTScrollbar; }
    virtual void ShowDebugBackground(bool bShow);
    virtual ActionResult Action(unsigned int unAction) { return false; }
    virtual bool Altered() const { return false; }
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save() {}
    virtual void Revert() {}
    virtual void ResetDisplayState();

    // ITouchTarget declarations
    virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
    virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

    static float GetWidth() { return 40.0f; }
    // TODO Move this to the environment
    static unsigned int GetInvokeValue() { return 0x10000; }

    void SetTotalContentHeight(int nTotalContentHeight);
    void SetHeight(float fHeight);
    void SetYOffset(float nYOffset);
    void AlterYOffset(int nDelta);
    int GetScrollUpAction() { return m_nScrollUpAction; }
    int GetScrollDownAction() { return m_nScrollDownAction; }

private:
    void CalcBarOffset();
    void CalcScrollOffsetFromBarOffset();
    bool CoordsOnBar(int x, int y);

protected:
    IOverlayIntl* m_pOverlayWithin;
    const IOverlayEnvIntl* m_pOverlayEnv;
    ScrollTarget* m_pScrollTarget;
    int m_nElementId;
    float m_fHeight;
    int m_nTotalContentHeight;
    int m_nDisplayableContentHeight;
    float m_fYOffset;
    bool m_bForViewNotTab;

    bool m_bShowDebugBackground;

    float m_yLastRenderedOrigin;
    bool m_bRenderedAtLeastOnce;

    float m_xBarOffset;
    double m_dYBarOffset;
    float m_fBarWidth;
    double m_dBarHeight;

    int m_nScrollUpAction;
    int m_nScrollDownAction;

    bool m_bScrolling;
    float m_yScrollStart;
};

