#pragma once
#include "ElementBase.h"
#include "IOverlayIntl.h"
#include "ITextRenderer.h"
#include "OverlayDefs.h"
#include <string>

class OverlayTab;
class IOverlayEnv;

class NumericSelectorElement :
    public ElementBase
{
public:
    NumericSelectorElement(IOverlayIntl* pOverlay, int nElementId, std::string sLabelText, OverlayLabelAlignment eLabelAlignment, int* pnValue, int nMin, int nMaxInclusive, int nStep);
    virtual ~NumericSelectorElement();

    // IOverlayElement Declarations
    virtual void Deinitialise();
    virtual void FreeSpace();
    virtual void InitialiseForWidth(float fWidth);
    virtual float GetHeight() const;
    virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
    virtual void Highlight(bool bHighlight);
    virtual float GetScrollIncrement();
    virtual void InitialiseTouchAreas(TouchLayer* pTouchLayer, float xOrigin, float yOrigin, ITouchTarget* pTarget, int nInvokeValue);
    virtual OverlayControlType GetControlType() const { return eOCTNumericSelector; }
    virtual ActionResult Action(unsigned int unAction);
    virtual bool Altered() const;
    virtual bool CanSave() const;
    virtual void Save();
    virtual void Revert();
    virtual void ResetDisplayState() {}

    // ITouchTarget declarations
    virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);

protected:
    void GetValueLabelOffset(float* pfXOffset, float* pfYOffset);
    void GetBackgroundOffsetAndSize(float* pfXOffset, float* pfYOffset, float* pfWidth, float* pfHeight);
    bool ActionRight();
    bool ActionLeft();
 private:
    void UpdateValueText();

protected:
    int* m_pnValue;
    int m_nValue;
    
    int m_nMin;
    int m_nMaxInclusive;
    int m_nStep;
  
    ITextLine* m_pTLValue;
    float m_fValueWidth;
};