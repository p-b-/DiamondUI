#pragma once
#include "OverlayDefs.h"
#include "ElementBase.h"
#include "IOverlayIntl.h"
#include "ITextRenderer.h"
#include <string>
#include <vector>
#include <map>

class ButtonElement;
class IOverlayEnv;

class ButtonBarElement :
    public ElementBase
{
public:
    ButtonBarElement(IOverlayIntl* pOverlay, int nElementId, OverlayLabelAlignment eLabelAlignment);
    virtual ~ButtonBarElement();

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual void Highlight(bool bHighlight);
    virtual float GetScrollIncrement();
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue);
    virtual OverlayControlType GetControlType() const { return eOCTButtonBar; }
    virtual ActionResult Action(unsigned int unAction);
    virtual bool Altered() const { return false; }
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save() {}
    virtual void Revert() {}
    virtual void ResetDisplayState();

    // ITouchTarget declarations
    virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
    virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

    ButtonElement* AddButton(int nElementId, TabActionButtonType eButtonType, std::string sLabel);
    static bool InvokeTargetAction(unsigned int unTargetHandle, int nAction, unsigned int unInvokeValue);
private:
    bool MoveHighlightLeft();
    bool MoveHighlightRight();
    bool InvokeAction(int nAction, unsigned int unInvokeValue);
    static unsigned int RegisterButtonBar(ButtonBarElement* pBar);

private:
    int m_nHighlightedElementIndex;
    unsigned int m_unInvokeHandle;
    std::vector<IOverlayElement* > m_vcElements;
    static std::map<unsigned int, ButtonBarElement* > s_mpInvokeHandleToButtonBar;
};

