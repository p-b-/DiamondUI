#pragma once
class IEngineInput;
class Camera;
class ITouchLayer;
class IGameInput
{
public:
	virtual ~IGameInput() {}
	virtual void SetEngineInput(IEngineInput* pInput) = 0;
	virtual bool RegisterInputs(IEngineInput* pInput) = 0;
	virtual void ProcessInput(IEngineInput* pInput, float fDeltaTime) = 0;
	virtual void ProcessMouseMovement(float fXPos, float fYPos) = 0;
	virtual void ProcessMouseButton(int button, int action, int mods) = 0;
	virtual void ProcessMouseScroll(float fScroll) = 0;
	virtual void ProcessKeyEvent(int nKey, int nScancode, int nAction, int nMods) = 0;
	virtual void ControllerConnected(unsigned int unActiveControllerId) = 0;
	virtual void ControllerReconnected(unsigned int hActiveControllerId) = 0;
	virtual void ControllerDisconnected() = 0;
};

