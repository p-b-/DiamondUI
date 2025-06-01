#pragma once

class ITextInputTarget {
public:
	virtual void CharInput(char ch) = 0;
	virtual void KeyPressed(TextInputKey eKey, bool bShiftPressed, bool bCtrlPressed, bool bAltPressed) = 0;
	virtual void KeyboardDismissed() = 0;
};