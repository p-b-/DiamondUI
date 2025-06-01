#pragma once

//namespace Condensate {
	enum DigitalActionType {
		eDAT_None,
		eDAT_EventOnPress,
		eDAT_EventOnRelease,
		eDAT_EventContinuous,
		eDAT_EventAutoRepeat,
		eDAT_EventAutoRepeatTwoLevels,
	};

	enum CharSet {
		eCS_None,
		eCS_Text,
		eCS_Numeric
	};

	enum InputAction {
		eIA_Unknown,
		eIA_Press,
		eIA_Release,
		eIA_AutoRepeat
	};

	enum TextInputKey {
		eTIK_Unknown,
		eTIK_LeftArrow,
		eTIK_RightArrow,
		eTIK_UpArrow,
		eTIK_DownArrow,
		eTIK_Delete,
		eTIK_Backspace,
		eTIK_Home,
		eTIK_End,
		eTIK_Return,
		eTIK_Escape,
		eTIK_Tab
	};

	struct IntRect {
		IntRect(int nXOrigin, int nYOrigin, int nWidth, int nHeight) :
			m_nXOrigin(nXOrigin), m_nYOrigin(nYOrigin), m_nWidth(nWidth), m_nHeight(nHeight) {
		}
		int m_nXOrigin;
		int m_nYOrigin;
		int m_nWidth;
		int m_nHeight;
	};

	class ITouchTarget;
	struct Touch {
		Touch(bool bTouchValid) :
			m_bTouchInsideArea(false), m_unAction(0), m_pTarget(nullptr), m_nInvocationValue(0), m_bContinuousUpdate(false),
			m_rcTouch(0,0,0,0) {
			m_bTouchValid = bTouchValid;
		}
		bool m_bTouchValid;
		bool m_bTouchInsideArea;
		unsigned int m_unAction;
		ITouchTarget* m_pTarget;
		int m_nInvocationValue;
		bool m_bContinuousUpdate;

		IntRect m_rcTouch;
	};
//}