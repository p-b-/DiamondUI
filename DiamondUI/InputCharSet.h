#pragma once
#include "InputDefs.h"

#include <map>

enum EProcessChar {
	ePS_NotProcessed,
	ePS_Process,
	ePS_Ignore
};

class InputCharSet
{
public:
	InputCharSet(CharSet eCharSet);
	void AddChar(char cAdd);
	void AddCharRange(char cRangeSt, char cRangeEndInclusive);
	void KeyIsProcessed(TextInputKey eKey, EProcessChar ePS);

	EProcessChar GetKeyIsProcessed(TextInputKey eKey);
	EProcessChar GetIsCharProcessed(char ch);

private:
	CharSet m_eCharSet;

	std::map<char, bool> m_mpConsumeChars;
	std::map<TextInputKey, EProcessChar> m_mpKeyIsProcessed;
};


