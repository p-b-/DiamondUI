#pragma once
#include "OverlayGraphics.h"
#include "IOverlayIntl.h"
#include "IOverlayEnvIntl.h"
#include "IOverlay.h"
#include "ITouchTarget.h"
#include "Shader.h"
#include "OverlayDefs.h"

#include <atomic>
#include <set>
#include <stack>
#include <string>
#include <vector>

class ITouchLayer;
class IEngineInputIntl;
class OverlayTab;
class OverlayView;

class Overlay : public OverlayGraphics, public IOverlayIntl, public IOverlay, public ITouchTarget
{
public:
	Overlay(IOverlayEnvIntl* pEnv);
	virtual ~Overlay();
	Overlay(const Overlay& toCopy) = delete;
	Overlay(Overlay&& toMove) = delete;

	// IOverlayIntl declarations
	virtual const IOverlayEnvIntl* GetInternalOverlayEnvironment() const { return m_pEnvironment; }
	virtual unsigned int AddTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, bool bNoScroll, int nInvokeValue);
	virtual unsigned int AddVerticallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue);
	virtual unsigned int AddHorizontallyScrollingTouchArea(ITouchLayer* pTL, float xOrigin, float yOrigin, float fWidth, float fHeight, ITouchTarget* pTarget, unsigned int unAction, int nInvokeValue);
	virtual void SetTouchAreaToNeedContinuousUpdates(ITouchLayer* pTL, unsigned int unId, bool bContinuousUpdates);
	virtual bool ApplyTabPressed();
	virtual bool ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTarget, const IntRect& rcTargetArea);
	virtual int NextElementId();
	
	// IOverlay declarations
	virtual void Render(float fDeltaTime);
	virtual void Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float fWidth, float fHeight);
	virtual void Initialise(OverlayLabelAlignment eAlignment, std::string sUnsavedDataText, float xOrigin, float yOrigin, float fWidth, float fHeight);
	virtual void InitialiseFullScreen(OverlayLabelAlignment eAlignment, bool bRenderBackdrop, float fWidth, float fHeight);
	virtual void WindowSizeChanged(int nWidth, int nHeight);
	virtual void InitialiseTouchAreas();
	virtual bool CanClose() const;
	virtual bool SaveData();
	virtual void ChangeDisplayState(bool bDisplayingNotHiding);
	virtual IOverlayEnv* GetOverlayEnvironment() const { return m_pEnvironment->GetPublicInterface(); }
	
	virtual ActionResult Action(unsigned int unAction);
	virtual IOverlayTab* AddTab(std::string sTabTitle);
	virtual IOverlayView* AddView(float fXOrigin, float fYOrigin, float fWidth, float fHeight, bool bVerticalNotHoriz);
	virtual void ShowDebugBackgroundOnControl(OverlayControlType eControlType, bool bShow);
	virtual void ShowDebugBackgroundOnControlInTab(unsigned int unTab, OverlayControlType eControlType, bool bShow);
	virtual void DisplayAlert(std::string sText, std::string sButtonText, std::string sCancelText);

	virtual void SetButtonRenderFunc(fnRenderButton pFunc);
	virtual void SetSelectorRenderFunc(fnRenderSelector pFunc);

	// ITouchTarget declarations
	virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime);
	virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y);


	// Static method declarations
	static void SetEngineInput(IEngineInputIntl* pEngineInput);

private:
	void CentreInScreen(float fWidth = 0.0f, float fHeight = 0.0f);
	void AnimateTabs(float fDeltaTime);
	void RenderTabs();
	void RenderBackdrop();

	void AnimateViews(float fDeltaTime);
	void RenderViews();
	void InitialiseTouchAreasForTabs();
	void InitialiseTouchAreasForViews();

	void ResetTabDisplayState();

	bool DisplayTabHeader() const { return m_vcTabs.size() > 1; }
	bool TabDataAltered();
	void RevertTabData();
	bool SaveTabData();
	int GetTabCount() {
		// Extremely unlikely to overflow
		return static_cast<int>(m_vcTabs.size());
	}
	bool CheckTabExists(unsigned int unTabIndex);
	OverlayTab* GetTabWithIndex(unsigned int unTabIndex);
	const OverlayTab* GetTabWithIndex(unsigned int unTabIndex) const;
	void HighlightTabTitleAsPressed(int nTabIndex, bool bPressed);
	void DisplayTab(int nTabIndex);
	bool DisplayNextTab();
	bool DisplayPreviousTab();
	void InitialiseInternalActions();

private:
	IOverlayEnvIntl* m_pEnvironment;

	std::vector<OverlayTab*> m_vcTabs;
	unsigned int m_unTabDisplayed;
	std::string m_sUnsavedDataText;
	bool m_bRenderBackdrop;
	bool m_bKeepFullScreen;
	float m_xOrigin;
	float m_yOrigin;
	float m_fWidth;
	float m_fHeight;
	bool m_bKeepCentred;
	float m_xTabOffset;
	bool m_bTabsNotViews;
	ITouchLayer* m_pTouchLayer;
	ITouchLayer* m_pTabBarTouchLayer;

	OverlayLabelAlignment m_eLabelAligment;
	Overlay* m_pAlertOverlay;

	std::vector<OverlayView* > m_vcViews;

	static std::atomic<int> s_nNextElementId;
	static IEngineInputIntl* s_pEngineInput;
};

