#pragma once

class IWindow;
class Textures;
class IGameLoop;

class IScene {
public:
	virtual ~IScene() {}
	virtual bool Initialise(Textures* pTextureCtrl, IWindow* pWnd) = 0;
	virtual bool GetUsingShadows() = 0;
	virtual void SetGameLoop(IGameLoop* pGameLoop) = 0;
	virtual void Deinitialise() = 0;
	virtual void WindowSizeChanged(int nWidth, int nHeight) = 0;
	virtual void AnimateScene(float fDeltaTime) = 0;
	virtual void DrawScene(bool bForShadows) = 0;
};