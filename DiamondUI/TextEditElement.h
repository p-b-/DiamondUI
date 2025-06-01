#pragma once
#include "ElementBase.h"
#include "InputDefs.h"
#include "ITextInputTarget.h"
#include "ITextRenderer.h"
#include <string>

class IOverlayEnv;

class TextEditElement :
    public ElementBase, public ITextInputTarget
{
public:
    TextEditElement(IOverlayIntl* pOverlay, int nElementId, std::string sLabel, std::string& sText, OverlayLabelAlignment eLabelAlignment, int nMaxLength = -1);
    virtual ~TextEditElement();

    void HideText(bool bHideText, char cPasswordChar);
   
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
    virtual OverlayControlType GetControlType() const { return eOCTTextField; }
    virtual bool Altered() const;
    // No validation errors or anything stopping it saving - just nothing to save, but Altered() returns false
    virtual bool CanSave() const { return true; }
    virtual void Save();
    virtual void Revert();
    virtual void ResetDisplayState();

    // ITouchTarget declarations
    virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
    virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);

    // ITextInputTarget declarations
    virtual void CharInput(char ch);
    virtual void KeyPressed(TextInputKey eKey, bool bShiftPressed, bool bCtrlPressed, bool bAltPressed);
    virtual void KeyboardDismissed();

private:
    void InitialiseControlDimensions();
    void UpdateText();
    void CalculateCursorXOffset();
    void JumpRightToWhitespace();
    void JumpLeftToWhitespace();
    void DisplayKeyboardSoon(bool bQuickly);

private:
    float m_fPreferredWidth;
    ITextLine* m_pTLText;
    std::string& m_sOriginalText;
    std::string m_sText;
    int m_nMaxLength;

    bool m_bHideText;
    char m_cPasswordChar;

    int m_nCursorPos;
    float m_fCursorXOffset;
    bool m_bDrawTemporaryCaret;

    float m_xTextDrawOffset;
    bool m_bCentred;

    bool m_bStartKeyboardInputOnRender;
    int m_nFramesBeforeKeyboardDisplay;
    bool m_bKeyboardDisplayed;

    float m_fTextMargin;
    float m_fXBackground;
    float m_fYBackground;
    float m_fBackgroundWidth;
    float m_fBackgroundHeight;

    float m_fXTextOffset;
    float m_fYTextOffset;
    float m_fTextWidth;
    float m_fTextHeight;
};
