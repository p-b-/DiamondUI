#pragma once
#include "IGameInput.h"

class ITextRenderer;

#include "IScene.h"

class IGame {
public:
	virtual ~IGame() {}
	virtual void Initialise(ITextRenderer* pTextRenderer, Textures* pTextureCtrl, IWindow* pWnd) = 0;
	virtual void InitialiseUI() = 0;
	virtual void Deinitialise() = 0;
	virtual void WindowSizeChanged(int nWidth, int nHeight) = 0;
	virtual void ReleaseEngineResources() = 0;
	virtual void RenderOverlay(float fDeltaTime) = 0;
	virtual IScene* GetScene() = 0;
	virtual IGameInput* GetGameInput() = 0;
};