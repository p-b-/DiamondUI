#pragma once

class ITouchLayer;
class ITextInputTarget;

// Interface to input, internal to engine itself
class IEngineInputIntl {
public:
	virtual bool Initialise() = 0;
	virtual void DeinitialiseAndDelete() = 0;
	virtual bool RegisterInternalTouchAction(unsigned int unAction, DigitalActionType eActionType) = 0;
	virtual void ProcessInput(float fDeltaTime) = 0;
	virtual void ShowBindingPanel() = 0;
	virtual void AddTouchLayer(ITouchLayer* pTouchLayer) = 0;
	virtual void RemoveTouchLayer(ITouchLayer* pTouchLayer) = 0;
	virtual void NewWindowLayer() = 0;
	virtual void AlterTopWindowTouchLayerCount(bool bIncNotDec) = 0;
	virtual void RemoveTopWindowLayer(bool bLayersAlreadyRemoved) = 0;
	virtual void ActivateTopWindowLayer(bool bActivateNotDeactivate) = 0;
	virtual bool ActivateKeyboardInput(CharSet eCharSet, ITextInputTarget* pTextInputTarget, const IntRect& rcInputArea) = 0;
	virtual void KeyboardDismissed() = 0;
};