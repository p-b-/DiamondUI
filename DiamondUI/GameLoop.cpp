#include "GameLoop.h"
#include "Shader.h"
#include "Camera.h"
#include "EngineInput.h"
#include "GameInput.h"
#include "ShadowedScene.h"
#include "TextRenderer.h"
#include "OverlayEntity.h"
#include "Overlay.h"
#include "OverlayEnv.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int gnFrameCount = 0;

GameLoop::GameLoop() {
    m_bGamePaused = false;
    m_pWnd = nullptr;
    m_pEngineInput = nullptr;
}

GameLoop::~GameLoop() {
    if (m_pEngineInput != nullptr) {
        m_pEngineInput->DeinitialiseAndDelete();
        m_pEngineInput = nullptr;
    }
}

void GameLoop::RunLoop(Window& wnd, Textures* pTextureCtrl, IGame* pGame) {
    m_pWnd = &wnd;
    OverlayGraphics::InitialiseOverlayGraphics(m_pWnd->GetWidth(), m_pWnd->GetHeight());
    int nInternalActionStartIndex = EngineInput::GetInternalActionStartIndex();
    OverlayEnv::SetInternalActionStartIndex(nInternalActionStartIndex);

    TextRenderer textRenderer;
    if (textRenderer.Initialise(&wnd)) {
        OverlayEntity::SetTextRenderer(&textRenderer);
    }
    else {
        std::cerr << "Failed to initialise text renderer" << std::endl;
    }
    pGame->Initialise(&textRenderer, pTextureCtrl, &wnd);
    //textRenderer.FinishedLoading();
    IScene* pScene = pGame->GetScene();
    pScene->SetGameLoop(this);
    bool bUsingShadows = pScene->GetUsingShadows();

    if (SteamUtils()->IsSteamRunningOnSteamDeck()) {
        m_pWnd->HidePointer();
    }

    m_pEngineInput = new EngineInput(&wnd, pGame->GetGameInput());
    if (!m_pEngineInput->Initialise()) {
        wnd.ShouldClose(true);
    }
    else {
        // To display the control binding panel on start-up, uncomment this.
        /*if (SteamUtils()->IsSteamRunningOnSteamDeck()) {
            pIP->ShowBindingPanel();
        } */
    }
    Overlay::SetEngineInput(m_pEngineInput);

    pGame->InitialiseUI();

    while (!wnd.IsClosing()) {
        if (wnd.ScreenSizeChanged()) {
            wnd.ResetScreenSizeChangedFlag();
            int width = wnd.GetWidth();
            int height = wnd.GetHeight();
            pGame->WindowSizeChanged(width, height);
        }
        gnFrameCount++;
        if (gnFrameCount == 100) {

        }
        wnd.StartRenderLoop(bUsingShadows);
        SteamAPI_RunCallbacks();
        float fDeltaTime = wnd.GetDeltaTime();
        if (m_pEngineInput != nullptr) {
            m_pEngineInput->ProcessInput(fDeltaTime);
        }
        pScene->AnimateScene(fDeltaTime);

        if (bUsingShadows) {
            pScene->DrawScene(true);
            m_pWnd->EndShadowRendering();
        }
        pScene->DrawScene(false);

        wnd.EndRenderLoop();
        pGame->RenderOverlay(fDeltaTime);
        
        wnd.SwapBuffers();
    }
    pGame->ReleaseEngineResources();
    pGame->Deinitialise();
    Entity::ClearUnreferencedEntities();

    if (m_pEngineInput != nullptr) {
        m_pEngineInput->DeinitialiseAndDelete();
        m_pEngineInput = nullptr;
    }
    OverlayGraphics::Deinitialise();
}

void GameLoop::PauseGame(bool bPause) {
    m_bGamePaused = bPause;
}

void GameLoop::QuitGame() {
    m_pWnd->ShouldClose(true);
}

bool GameLoop::IsPaused() {
    return m_bGamePaused;
}

void GameLoop::DisplayDepthMap(bool bShow, unsigned int unDepthMapTexture, float fNearPlane, float fFarPlane) {
    m_pWnd->ShowDepthMap(bShow, unDepthMapTexture, fNearPlane, fFarPlane);
}

void GameLoop::DisplayBindingPanel() {
    m_pEngineInput->ShowBindingPanel();
}
