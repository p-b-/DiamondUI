#pragma once

#include "SceneActions.h"

class IDiamondScene {
public:
	virtual ~IDiamondScene() {}

	virtual void ProcessDigitalAction(SceneAction eAction, float fDeltaTime) = 0;
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue, float fDeltaTime) = 0;
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fDeltaTime) = 0;
	virtual void ProcessAnalogAction(SceneAction eAction, float fAnalogValue1, float fAnalogValue2, float fAnalogValue3, float fDeltaTime) = 0;

	virtual void ControllerConnected() = 0;
};