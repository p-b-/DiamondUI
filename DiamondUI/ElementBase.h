#pragma once
#include "OverlayDefs.h"
#include "IOverlayElement.h"
#include "OverlayGraphics.h"
#include "ITouchTarget.h"
#include "ITextRenderer.h"
#include <string>

class OverlayTab;
class IOverlayIntl;
class IOverlayEnvIntl;

class ElementBase :
    public IOverlayElement, public OverlayGraphics, public ITouchTarget
{
public:
	ElementBase(IOverlayIntl* pOverlay, int nElementId, std::string sLabelText, OverlayLabelAlignment eLabelAlignment);
	ElementBase(IOverlayIntl* pOverlay, int nElementId, OverlayLabelAlignment eLabelAlignment);
	virtual ~ElementBase();


	// IOverlayElement Declarations
	virtual void Deinitialise();
	virtual void FreeSpace();
	virtual int GetId() const;
	virtual float GetPreferredControlWidth() const;
	virtual void SetPreferredControlWidth(float fWidth);
	virtual float GetLastRenderedYOrigin() const;
	virtual void Render(float xOrigin, float yOrigin, float fWidth, float fHeight);
	virtual bool RenderedYet() const { return m_bRenderedAtLeastOnce; }
	virtual bool Disabled() { return m_bDisabled; }
	virtual void Disable(bool bDisable) { m_bDisabled = bDisable; }
	virtual bool CanHighlight() { return !m_bDisabled; }
	virtual void ShowDebugBackground(bool bShow);

	// ITouchTarget Declarations
	virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
	virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) { return false; }

protected:
	float GetHeightIncludingLabel(float fHeightOfControl) const;
	void InitialiseContainerForWidth(float fWidth);

private:
	void InitialiseFontMetrics();
	void InitialiseLeftLabelForWidth(float fWidth, FontFaceHandle hTextFont);
	void InitialiseAboveLabelForWidth(float fWidth, FontFaceHandle hTextFont);
	void InitialiseWidthForNoLabel(float fWidth, FontFaceHandle hTextFont);
	void CreateGraphicalTextLinesForLabel(FontFaceHandle hTextFont, std::string sLabel, float fLabelWidth, int nMaxLineCount);

protected:
	bool m_bHasLabel;
	bool m_bDisabled;
	bool m_bHighlighted;
	bool m_bShowDebugBackground;

	IOverlayIntl* m_pOverlayWithin;
	const IOverlayEnvIntl* m_pOverlayEnv;
	int m_nElementId;

	OverlayLabelAlignment m_eLabelAlignment;
	std::string m_sLabelText;
	std::vector<ITextLine*> m_vcLabelTextLines;

	float m_yLastRenderedOrigin;
	bool m_bRenderedAtLeastOnce;

	float m_fFontHeight;
	float m_fLineSpacing;
	float m_fTextDescender;

	float m_fCtrlWidth;
	float m_xCtrlOffset;
};

