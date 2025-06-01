#pragma once
#include "IWindow.h"
#include "EngineInput.h"
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <steam/steam_api.h>

class Shader;
class IOverlay;

class Window :
	public IWindow
{
public:
	Window();
	~Window();
	bool Initialise(int width, int height);
	void Deinitialise();
	void SetMouseCallback(GLFWcursorposfun func);
	void SetMouseButtonCallback(GLFWmousebuttonfun func);
	void SetScrollCallback(GLFWscrollfun func);
	void SetKeyCallback(GLFWkeyfun func);
	void SetCharCallback(GLFWcharfun func);
	void SetFocusCallback(GLFWwindowfocusfun func);

	void CaptureMouse();
	void HidePointer();
	void SetClearColour(float red, float green, float blue, float alpha);

	void StartRenderLoop(bool bHasShadows);
	void EndRenderLoop();
	void EndShadowRendering();

	bool IsClosing();
	int GetKey(int key);
	void ShouldClose(bool shouldClose);
	void SwapBuffers();
	static void FramebufferSizeCallback(GLFWwindow* pGLWnd, int nWidth, int nHeight);
	static Window* FindAssociatedWindow(GLFWwindow* pGLWnd);

	bool IsFocused();
	void FocusToWindow(bool focus);

	bool ScreenSizeChanged();
	void ResetScreenSizeChangedFlag();

	// IWindow declarations
	virtual IOverlay* CreateUIOverlay(Textures* pTextureCtrl, IOverlayEnv* pEnv = nullptr);
	virtual int GetWidth() const { return m_nWidth; }
	virtual int GetHeight() const { return m_nHeight; }
	virtual double GetCurrentTime() const;
	virtual void GetMousePosition(int* pxPosition, int* pyPosition) const;
	virtual TextInputKey TranslateKeyToEnum(int nKey) const;
	virtual InputAction TranslateActionToEnum(int nAction) const;
	virtual void TranslateModifiersToFlags(int nMods, bool* pbShift, bool* pbCtrl, bool *pbAlt) const;
	virtual bool ShowKeyboard(int nXOrigin, int nYOrigin, int nWidth, int nHeight) const;
	virtual bool HideKeyboard() const;

private:
	// Steam callback declarations
	STEAM_CALLBACK(Window, KeyboardDismissed, FloatingGamepadTextInputDismissed_t);

public:
	float GetDeltaTime() const { return m_fDeltaTime; }
	void SetInput(EngineInput* pEngineInput) { m_pEngineInput = pEngineInput; }
	EngineInput* GetEngineInput() { return m_pEngineInput; }
	void ShowDepthMap(bool bShow, unsigned int unDepthMapTexture, float fNearPlane, float fFarPlane);
	
protected:
	void RemoveWnd();
	void RenderDepthMapToScreen();
	void InitialiseDepthMapDisplay();
	void ChangeWindowSize(int nWidth, int nHeight);

private:
	int m_nWidth;
	int m_nHeight;
	bool m_bClosing;
	bool m_bScreenSizeChanged;

	float m_fClearRed;
	float m_fClearGreen;
	float m_fClearBlue;
	float m_fClearAlpha;

	GLFWwindow* m_pWindow;
	static std::vector<Window* > m_svcWnds;

	float m_fDeltaTime = 0.0f;	// time between current frame and last frame
	float m_fLastFrame = 0.0f;

	EngineInput* m_pEngineInput;

	bool m_bShowDepthMap;
	unsigned int m_unDepthMapVAO;
	unsigned int m_unDepthMapVBO;
	unsigned int m_unDepthMapTexture;
	float m_fNearPlane;
	float m_fFarPlane;
	Shader* m_pShaderDepthMap;
};

