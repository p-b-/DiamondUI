#pragma once

enum OverlayLabelAlignment {
	eOLANone,
	eOLALeft,
	eOLAAbove
};

enum OverlayControlType {
	eOCTSpacer,
	eOCTLabel,
	eOCTButton,
	eOCTButtonBar,
	eOCTNumericSelector,
	eOCTEnumSelector,
	eOCTTextField,
	eOCTCheckbox,
	eCTScrollbar
};

enum TabActionButtonType {
	eTABTNoneTabAction,
	eTABTOkay,
	eTABTCancel,
	eTABTApply,
	eTABTClose
};

enum OverlayAnimation {
	eOA_None,
	eOA_Press,
	eOA_UnpressedToPressed,
	eOA_PressedToUnpressed
};

struct OverlayElementState {
	bool m_bHighlighted;
	bool m_bEnabled;
	bool m_bActivated;
};

struct ActionResult {
	ActionResult() {
		m_bSuccess = false;
		m_nRetryWithTargetAction = -1;
	}
	ActionResult(bool bSuccessful) {
		m_bSuccess = bSuccessful;
		m_nRetryWithTargetAction = -1;
	}
	ActionResult(int nRetryWithAction) {
		m_bSuccess = true;
		m_nRetryWithTargetAction = nRetryWithAction;
	}

	// TODO Determine when haptic feedback is provided and ensure this object supports the relevant data
	
	// false if an issue occurred process an action, that may need haptic feedback. true if succeeded (even if requesting the action be retried with a different target action)
	bool m_bSuccess;

	// -1 if no addition action should be tried. 0+ if action should be retried
	int m_nRetryWithTargetAction;
};

class ITextLine;
typedef bool(*fnInvokeTargetAction)(unsigned int unTargetHandle, int nAction, unsigned int unInvokeValue);

class IOverlayEnv;
typedef void(*fnRenderButton)(const IOverlayEnv*, int, float, float, float, float, const OverlayElementState&, ITextLine*);
typedef void(*fnRenderSelector)(const IOverlayEnv*, int, float, float, float, float, const OverlayElementState&, ITextLine*);
