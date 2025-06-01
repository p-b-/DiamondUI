#pragma once
#include "OverlayDefs.h"
#include "ElementBase.h"
#include "IOverlayIntl.h"
#include "ITextRenderer.h"
#include <string>

class IOverlayEnv;

class ButtonElement :
    public ElementBase
{
public:
    ButtonElement(IOverlayIntl* pOverlay, int nElementId, std::string sText, OverlayLabelAlignment eLabelAlignment);
    virtual ~ButtonElement();

    void SetTargetAction(int nTargetAction);
    void SetActionType(TabActionButtonType eButtonType);
    TabActionButtonType GetActionType() const { return m_eButtonType; }

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual float GetPreferredControlWidth() const { return m_fPreferredWidth; }
    virtual void SetPreferredControlWidth(float fWidth) { m_fPreferredWidth = fWidth; }
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual void Highlight(bool bHighlight);
    virtual ActionResult Action(unsigned int unAction);
    virtual float GetScrollIncrement();
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue);
    virtual OverlayControlType GetControlType() const { return eOCTButton; }
    virtual bool Altered() const { return false; }
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save() {}
    virtual void Revert() {}
    virtual void ResetDisplayState() {}

    // ITouchTarget declarations
    virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
    virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

protected:
    //void RenderFloatingButton(int xOrigin, int yOrigin, int nWidth, int nHeight);
    //void RenderScrollingButton(int xOrigin, int yOrigin, int nWidth, int nHeight);

    void GetLabelOffset(float* pfXOffset, float* pfYOffset);
    void GetBackgroundOffsetAndSize(float* pfXOffset, float* pfYOffset, float* pfWidth, float* pfHeight);
    void ActionOkay();

protected:
    void UpdateButtonText();

private:
    std::string m_sButtonLabel;

    int m_nButtonTextWidth;
    bool m_bRenderPressed;
    float m_fPreferredWidth;

    ITextLine* m_pTLLabel;
    int m_nTargetAction;
    TabActionButtonType m_eButtonType;
};

