#pragma once

#include "InputDefs.h"

class IOverlay;
class Textures;
class IOverlayEnv;

class IWindow {
public:
	virtual ~IWindow() {}
	virtual IOverlay* CreateUIOverlay(Textures* pTextureCtrl, IOverlayEnv* pEnv = nullptr) = 0;
	virtual int GetWidth() const = 0;
	virtual int GetHeight() const = 0;
	virtual double GetCurrentTime() const = 0;
	virtual void GetMousePosition(int* pxPosition, int* pyPosition) const = 0;
	virtual TextInputKey TranslateKeyToEnum(int nKey) const = 0;
	virtual InputAction TranslateActionToEnum(int nAction) const = 0;
	virtual void TranslateModifiersToFlags(int nMods, bool* pbShift, bool* pbCtrl, bool* pbAlt) const = 0;
	virtual bool ShowKeyboard(int nXOrigin, int nYOrigin, int nWidth, int nHeight) const = 0;
	virtual bool HideKeyboard() const = 0;
};