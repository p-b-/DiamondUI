#include "InputCharSet.h"

InputCharSet::InputCharSet(CharSet eCharSet) {
	m_eCharSet = eCharSet;
}

void InputCharSet::AddChar(char cAdd) {
	m_mpConsumeChars[cAdd] = true;
}

void InputCharSet::AddCharRange(char cRangeSt, char cRangeEndInclusive) {
	for (char c = cRangeSt; c <= cRangeEndInclusive; ++c) {
		m_mpConsumeChars[c] = true;
	}
}

void InputCharSet::KeyIsProcessed(TextInputKey eKey, EProcessChar ePS) {
	m_mpKeyIsProcessed[eKey] = ePS;
}

EProcessChar InputCharSet::GetKeyIsProcessed(TextInputKey eKey) {
	auto find = m_mpKeyIsProcessed.find(eKey);
	if (find == m_mpKeyIsProcessed.end()) {
		return ePS_NotProcessed;
	}
	return find->second;
}

EProcessChar InputCharSet::GetIsCharProcessed(char ch) {
	auto find = m_mpConsumeChars.find(ch);
	if (find == m_mpConsumeChars.end()) {
		return ePS_NotProcessed;
	}
	if (find->second) {
		return ePS_Process;
	}
	return ePS_Ignore;
}