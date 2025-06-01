#include <iostream>

#include "Window.h"
#include "Shader.h"
#include "OverlayEnv.h"
#include "Overlay.h"

std::vector<Window* > Window::m_svcWnds;

void errorCallback(int error, const char* description)
{
    std::cout << "ERR: " << description << std::endl;
    fprintf(stderr, "Error: %s\n", description);
}

Window::Window() {
    m_nWidth= 0;
    m_nHeight= 0;
    m_bClosing= 0;

    m_fClearRed= 0;
    m_fClearGreen= 0;
    m_fClearBlue= 0;
    m_fClearAlpha= 0;

    m_pWindow = nullptr;
    m_pEngineInput = nullptr;

    m_pShaderDepthMap = nullptr;
    m_unDepthMapTexture = 0;
    m_fNearPlane = 1.0f;
    m_fFarPlane = 7.5f;

    m_bScreenSizeChanged = false;

    m_bShowDepthMap = false;
    m_unDepthMapVAO = m_unDepthMapVBO = 0;
}

Window::~Window() {
    delete m_pShaderDepthMap;
    m_pShaderDepthMap = nullptr;
}

bool Window::Initialise(int width, int height) {
    m_nWidth = width;
    m_nHeight = height;
    m_bClosing = false;

    // glfw: initialize and configure
// ------------------------------
    if (!glfwInit()) {
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    m_pWindow = glfwCreateWindow(m_nWidth, m_nHeight, "Diamond UI", NULL, NULL);
    if (m_pWindow == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetErrorCallback(errorCallback);
    glfwMakeContextCurrent(m_pWindow);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferSizeCallback);

    if (!gladLoadGL()) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    Window::m_svcWnds.push_back(this);
    return true;
}

bool Window::IsFocused() {
    int focused = glfwGetWindowAttrib(m_pWindow, GLFW_FOCUSED);
    return focused == 1;
}

void Window::FocusToWindow(bool focus) {
    if (focus) {
        glfwFocusWindow(m_pWindow);
    }
    else {
    }
}

bool Window::ScreenSizeChanged() {
    return m_bScreenSizeChanged;
}

void Window::ResetScreenSizeChangedFlag() {
    m_bScreenSizeChanged = false;
}

void Window::Deinitialise() {
    glfwTerminate();
}

// IWindow implementation
//
IOverlay* Window::CreateUIOverlay(Textures* pTextureCtrl, IOverlayEnv* pEnv /*= nullptr*/) {
    IOverlayEnvIntl* pEnvIntl = nullptr;
    if (pEnv == nullptr) {
        pEnvIntl = new OverlayEnv();
        pEnvIntl->Initialise(pTextureCtrl);
    }
    else {
        pEnvIntl = dynamic_cast<IOverlayEnvIntl*>(pEnv);
    }
    Overlay* pOverlay = new Overlay(pEnvIntl);
    return pOverlay;
}

double Window::GetCurrentTime() const {
    return glfwGetTime();
}

void Window::GetMousePosition(int* pxPosition, int* pyPosition) const {
    double dX;
    double dY;
    glfwGetCursorPos(m_pWindow, &dX, &dY);
    *pxPosition = static_cast<int>(floor(dX));
    *pyPosition = m_nHeight - static_cast<int>(floor(dY)) - 1;
}

TextInputKey Window::TranslateKeyToEnum(int nKey) const {
    switch (nKey) {
    case GLFW_KEY_LEFT: return eTIK_LeftArrow;
    case GLFW_KEY_RIGHT: return eTIK_RightArrow;
    case GLFW_KEY_UP: return eTIK_UpArrow;
    case GLFW_KEY_DOWN: return eTIK_DownArrow;
    case GLFW_KEY_ENTER: return eTIK_Return;
    case GLFW_KEY_BACKSPACE: return eTIK_Backspace;
    case GLFW_KEY_DELETE: return eTIK_Delete;
    case GLFW_KEY_TAB: return eTIK_Tab;
    case GLFW_KEY_ESCAPE: return eTIK_Escape;
    case GLFW_KEY_HOME: return eTIK_Home;
    case GLFW_KEY_END: return eTIK_End;
    default:
        return eTIK_Unknown;
    }
}

InputAction Window::TranslateActionToEnum(int nAction) const {
    switch (nAction) {
    case GLFW_PRESS: return eIA_Press;
    case GLFW_RELEASE: return eIA_Release;
    case GLFW_REPEAT: return eIA_AutoRepeat;
    default:
        return eIA_Unknown;
    }
}

void Window::TranslateModifiersToFlags(int nMods, bool* pbShift, bool* pbCtrl, bool* pbAlt) const {
    *pbShift = false;
    *pbCtrl = false;
    *pbAlt = false;
    if (nMods & GLFW_MOD_SHIFT) {
        *pbShift = true;
    }
    if (nMods & GLFW_MOD_CONTROL) {
        *pbCtrl = true;
    }
    if (nMods & GLFW_MOD_ALT) {
        *pbAlt = true;
    }
}

bool Window::ShowKeyboard(int nXOrigin, int nYOrigin, int nWidth, int nHeight) const {
    // SteamDeck floating Keyboard is displayed at top or bottom of screen
    // Giving ShowFloatingGamepadTextInput() the rectangle where the text is being edited, does not work. Whether it is flipped so (0,0) is at
    //  top-left, or (0,0) is at bottom-left, it does not seem to work
    //

    // Keyboard dimensios are as follows:
    // --------- (0,799) top of screen
    // 
    // 
    // --------- (0,438) bottom of keyboard (when displayed at top) 
    // --------- (0,362) top of keyboard (when displayed at bottom)
    // 
    // 
    // --------- (0,0)   bottom of screen

    // Can force keyboard to be at bottom with (0,0,1200,2020) and to be at top with (0,300,1200,20)

    int yKeyboard = 0;
    if (nYOrigin < 362) {
        yKeyboard = 300;
    }
    std::cout << "Calling ShowFloatingGamepadTextInput" << std::endl;

    return SteamUtils()->ShowFloatingGamepadTextInput(k_EFloatingGamepadTextInputModeModeSingleLine,
       00, yKeyboard, 1200, 20);
}


bool Window::HideKeyboard() const {
    return SteamUtils()->DismissFloatingGamepadTextInput();
}
//
// IWindow implementation

// Steam callback implementation
//
void Window::KeyboardDismissed(FloatingGamepadTextInputDismissed_t* pParam) {
    std::cout << "Keyboard dismiss callback called" << std::endl;
    m_pEngineInput->KeyboardDismissed();
}

//
// Steam callback implementation


void Window::SetMouseCallback(GLFWcursorposfun func) {
    glfwSetCursorPosCallback(m_pWindow, func);
}

void Window::SetMouseButtonCallback(GLFWmousebuttonfun func) {
    glfwSetMouseButtonCallback(m_pWindow, func);
}

void Window::SetScrollCallback(GLFWscrollfun func) {
    glfwSetScrollCallback(m_pWindow, func);
}

void Window::SetKeyCallback(GLFWkeyfun func) {
    glfwSetKeyCallback(m_pWindow, func);
}

void Window::SetCharCallback(GLFWcharfun func) {
    glfwSetCharCallback(m_pWindow, func);
}

void Window::SetFocusCallback(GLFWwindowfocusfun func) {
    glfwSetWindowFocusCallback(m_pWindow, func);
}

void Window::CaptureMouse() {
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}
void Window::HidePointer() {
    glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
}

void Window::SetClearColour(float red, float green, float blue, float alpha) {
    m_fClearRed = red;
    m_fClearGreen = green;
    m_fClearBlue = blue;
    m_fClearAlpha = alpha;
}

void Window::StartRenderLoop(bool bHasShadows) {
    float currentFrame = static_cast<float>(glfwGetTime());
    m_fDeltaTime = currentFrame - m_fLastFrame;
    m_fLastFrame = currentFrame;
    
    if (!bHasShadows) {
        glClearColor(m_fClearRed, m_fClearGreen, m_fClearBlue, m_fClearAlpha);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void Window::EndShadowRendering() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, m_nWidth, m_nHeight);
    glClearColor(m_fClearRed, m_fClearGreen, m_fClearBlue, m_fClearAlpha);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Window::EndRenderLoop() {
    if (m_bShowDepthMap) {
        RenderDepthMapToScreen();
    }
    glfwPollEvents();
    glDisable(GL_BLEND);
}

void Window::RemoveWnd() {
    // TODO Should lock this at al access points, however, it is unlikely to have multiple windows right now
    for (std::vector<Window* >::iterator iter = m_svcWnds.begin(); iter != m_svcWnds.end(); iter++) {
        if ((*iter) == this) {
            m_svcWnds.erase(iter);
            break;
        }
    }
}

bool Window::IsClosing() {
    return glfwWindowShouldClose(m_pWindow) || m_bClosing;
}

int Window::GetKey(int key) {
    return glfwGetKey(m_pWindow, key);
}

void Window::ShouldClose(bool shouldClose) {
    glfwSetWindowShouldClose(m_pWindow, shouldClose);
    m_bClosing = true;
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_pWindow);
}

void Window::FramebufferSizeCallback(GLFWwindow* pGLWnd, int nWidth, int nHeight) {
    std::cout << "Framebuffer size changed to " << nWidth << "x" << nHeight << std::endl;
    Window* pWnd = FindAssociatedWindow(pGLWnd);
    if (pWnd != nullptr) {
        pWnd->ChangeWindowSize(nWidth, nHeight);
    }
}

Window* Window::FindAssociatedWindow(GLFWwindow* pGLWnd) {
    for (Window* pWnd : m_svcWnds) {
        if (pWnd->m_pWindow == pGLWnd) {
            return pWnd;
        }
    }
    return nullptr;
}

void Window::ShowDepthMap(bool bShow, unsigned int unDepthMapTexture, float fNearPlane, float fFarPlane) {
    m_bShowDepthMap = bShow;
    m_unDepthMapTexture = unDepthMapTexture;
    m_fNearPlane = fNearPlane;
    m_fFarPlane = fFarPlane;
}

void Window::RenderDepthMapToScreen() {
    if (m_unDepthMapVAO == 0) {
        InitialiseDepthMapDisplay();
    }

    m_pShaderDepthMap->Use();
    m_pShaderDepthMap->SetFloat("near_plane", m_fNearPlane);
    m_pShaderDepthMap->SetFloat("far_plane", m_fFarPlane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_unDepthMapTexture);

    glBindVertexArray(m_unDepthMapVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Window::InitialiseDepthMapDisplay() {
    float rectangleVertices[] = {
        -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    };

    glGenVertexArrays(1, &m_unDepthMapVAO);
    glGenBuffers(1, &m_unDepthMapVBO);
    glBindVertexArray(m_unDepthMapVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_unDepthMapVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    m_pShaderDepthMap = new Shader();
    m_pShaderDepthMap->InitialiseShaders("depth_display_vert.glsl", "depth_display_frag.glsl");
    m_pShaderDepthMap->Use();
    m_pShaderDepthMap->SetInt("depthMap", 0);
}

void Window::ChangeWindowSize(int nWidth, int nHeight) {
    glViewport(0, 0, nWidth, nHeight);

    m_nWidth = nWidth;
    m_nHeight = nHeight;
    m_bScreenSizeChanged = true;
}
