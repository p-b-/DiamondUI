#pragma once

class ITouchTarget {
public:
	virtual ~ITouchTarget() {}
	virtual bool Invoke(unsigned int unAction, int nValue, float fDeltaTime) = 0;
	virtual bool TouchUpdate(unsigned int unAction, int nValue, bool bPressed, bool bOverTarget, int x, int y) = 0;
};