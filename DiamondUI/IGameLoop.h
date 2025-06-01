#pragma once

class IGameLoop {
public:
	virtual ~IGameLoop() {}
	virtual void PauseGame(bool bPause) = 0;
	virtual void QuitGame() = 0;

	virtual bool IsPaused() = 0;
	virtual void DisplayDepthMap(bool bShow, unsigned int unDepthMapTexture, float fNearPlane, float fFarPlane) = 0;
	virtual void DisplayBindingPanel() = 0;
};