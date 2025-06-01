#include "DiamondGame.h"
#include "DiamondScene.h"
#include "GameOverlay.h"
#include "IOverlayTab.h"
#include "IOverlay.h"
#include "IOverlayView.h"
#include "IOverlayEnv.h"
#include "GameFonts.h"
#include "GameInput.h"
#include "IOverlayEntity.h"
#include <sstream>
#include <iomanip>
#include "TextRenderer.h"
#include "IWindow.h"

DiamondGame* DiamondGame::s_pGame = nullptr;

DiamondGame::DiamondGame() : 
	m_bDisplayUI(false), m_pGameInput(nullptr), m_pOverlay(nullptr), m_pPitchLabel(nullptr),
	m_pPositionLabel(nullptr), m_pScene(nullptr), m_pTextRenderer(nullptr),
	m_pTextureCtrl(nullptr), m_pUI(nullptr), m_pYawLabel(nullptr), m_unGlyphAtlasId(0),m_pWindow(nullptr),
	m_pFullScreenOverlay(nullptr) {

    s_pGame = this;
    std::fill(std::begin(m_nNumber), std::end(m_nNumber), 0);
}

DiamondGame::~DiamondGame() {
	if (m_pOverlay != nullptr) {
		delete m_pOverlay;
		m_pOverlay = nullptr;
	}
	if (m_pScene != nullptr) {
		delete m_pScene;
		m_pScene = nullptr;
	}
	if (m_pGameInput) {
		delete m_pGameInput;
		m_pGameInput = nullptr;
	}
}

// IGameInput implementation
//
void DiamondGame::Initialise(ITextRenderer* pTextRenderer, Textures* pTextureCtrl, IWindow* pWnd) {
	m_pTextRenderer = pTextRenderer;
	m_pScene = new DiamondScene(this);
	m_pScene->Initialise(pTextureCtrl, pWnd);
	m_pTextureCtrl = pTextureCtrl;
	m_pWindow = pWnd;

	m_pOverlay = nullptr;

	m_pTextRenderer->RegisterFont(eGF_Aria, "arial.ttf");
	m_pTextRenderer->RegisterFont(eGF_OcraExt, "OCRAEXT.ttf");

	m_pGameInput = new GameInput(this, m_pScene);
}

void DiamondGame::InitialiseUI() {
	FontFaceHandle font = m_pTextRenderer->CreateFont(eGF_Aria, 24);

	auto rgbText = std::make_tuple(1.0f, 1.0f, 1.0f);
	CreateOverlay(rgbText, font);
	CreateUI(m_pWindow, rgbText, font);
}

void DiamondGame::WindowSizeChanged(int nWidth, int nHeight) {
	m_pScene->WindowSizeChanged(nWidth, nHeight);
	if (m_pUI != nullptr) {
		m_pUI->WindowSizeChanged(nWidth, nHeight);
	}
	if (m_pFullScreenOverlay != nullptr) {
		m_pFullScreenOverlay->WindowSizeChanged(nWidth, nHeight);
	}
	if (m_pTextRenderer != nullptr) {
		m_pTextRenderer->WindowSizeChanged(nWidth, nHeight);
	}
}

void DiamondGame::Deinitialise() {
	m_pPositionLabel = nullptr;
	m_pPitchLabel = nullptr;
	m_pYawLabel = nullptr;
	m_pOverlay->Deinitialise();
	delete m_pOverlay;
	m_pOverlay = nullptr;

	m_pScene->Deinitialise();
	delete m_pScene;
	m_pScene = nullptr;

	delete m_pGameInput;
	m_pGameInput = nullptr;
}

void DiamondGame::ReleaseEngineResources() {
	m_pTextRenderer = nullptr;
}

void DiamondGame::RenderOverlay(float fDeltaTime) {
	auto p = m_pScene->GetCameraPosition();
	auto a = m_pScene->GetCameraPitchAndYaw();
	std::ostringstream ssLabel;
	if (m_pPositionLabel != nullptr) {
		ssLabel << std::setprecision(1) << std::fixed << "Position: x:" << p.x << " y:" << p.y << " z:" << p.z;
		m_pPositionLabel->SetText(ssLabel.str());
		ssLabel.str("");
		ssLabel.clear();
	}

	if (m_pPitchLabel != nullptr) {
		ssLabel << std::setprecision(1) << std::fixed << "Pitch: " << a.x;
		m_pPitchLabel->SetText(ssLabel.str());
		ssLabel.str("");
		ssLabel.clear();
	}
	if (m_pYawLabel != nullptr) {
		ssLabel << std::setprecision(1) << std::fixed << "Yaw: " << a.y;
		m_pYawLabel->SetText(ssLabel.str());
	}

	m_pOverlay->Render(fDeltaTime);

	m_pFullScreenOverlay->Render(fDeltaTime);

	if (m_bDisplayUI && m_pUI != nullptr) {
		m_pUI->Render(fDeltaTime);
	}
}

IScene* DiamondGame::GetScene() {
	return m_pScene;
}

IGameInput* DiamondGame::GetGameInput() {
	return m_pGameInput;
}
//
// IGameInput implementation

void DiamondGame::SetGlyphTextureAtlasId(unsigned int unGlyphAtlasId) {
	m_unGlyphAtlasId = unGlyphAtlasId;
	m_pUI->GetOverlayEnvironment()->SetGlyphTextureAtlasId(unGlyphAtlasId);
}


void DiamondGame::ControllerConnected(bool bConnected) {
	if (bConnected) {
		m_pUI->GetOverlayEnvironment()->SetGlyphTextureAtlasId(m_unGlyphAtlasId);
	}
	else {
		m_pUI->GetOverlayEnvironment()->SetGlyphTextureAtlasId(0);
	}
}

bool DiamondGame::CanCloseUI() {
	if (m_bDisplayUI) {
		return m_pUI->CanClose();
	}
	return false;
}

void DiamondGame::GlyphFilepathForAction(unsigned int unAction, unsigned int unActionSet, const char* pzFilename) {
}

void DiamondGame::DisplayUI(bool bDisplay) {
	m_pUI->ChangeDisplayState(bDisplay);
	m_bDisplayUI = bDisplay;
}

void DiamondGame::ProcessGameDigitalAction(SceneAction eAction, float fDeltaTime) {
}

ActionResult DiamondGame::ProcessMenuDigitalAction(EDigitalInput eAction, float fDeltaTime) {
	if (!m_bDisplayUI || m_pUI == nullptr) {
		return false;
	}
	switch (eAction) {
	case eDigitalInput_Menu_UnpauseGame:
		m_pUI->SaveData();
		DisplayUI(false);
		return true;
	default:
		return m_pUI->Action(eAction);
	}
}

void DiamondGame::CreateOverlay(std::tuple<float, float, float> rgbText, FontFaceHandle fontFace) {
	m_pOverlay = new GameOverlay();
	auto rgbOverlay = std::make_tuple(0.0f, 0.0f, 0.2f, 0.8f);
	m_pOverlay->Initialise(fontFace, rgbText, rgbOverlay);
	m_pPositionLabel = m_pOverlay->AddTextLabel("Position:", 10, 10, 200, 19 + 2 * 5, eOERTFromTopLeft);
	m_pPitchLabel = m_pOverlay->AddTextLabel("Pitch:", eOERTBelow, m_pPositionLabel);
	m_pYawLabel = m_pOverlay->AddTextLabel("Yaw:", eOERTBelow, m_pPitchLabel);
}

void DiamondGame::CreateUI(IWindow* pWnd, std::tuple<float, float, float> rgbText, FontFaceHandle fontFace) {
	auto rgbOverlay = std::make_tuple(0.0f, 0.0f, 0.2f, 0.8f);
	m_pUI = pWnd->CreateUIOverlay(m_pTextureCtrl);
	IOverlayEnv* pEnv = m_pUI->GetOverlayEnvironment();
	pEnv->SetPreferredControlWidth(250);
	pEnv->SetTabTitleFont(fontFace);
	pEnv->SetTextFont(fontFace);
	pEnv->SetOverlayBackgroundColour(rgbOverlay);
	pEnv->SetOutsideOverlayAction(eDigitalInput_Menu_UnpauseGame);

	bool bTestSolo = true;
	if (bTestSolo) {
		m_pUI->Initialise(eOLAAbove, "Save settings?", 1280 * 3 / 4, 800 * 3 / 4);
	}
	else {
		m_pUI->Initialise(eOLALeft, "Save settings?", 1280 * 3 / 4, 800 * 3 / 4);
	}
	pEnv->SetMoveActions(eDigitalInput_Menu_Left, eDigitalInput_Menu_Right, eDigitalInput_Menu_Up, eDigitalInput_Menu_Down);
	pEnv->SetTabActions(eDigitalInput_Menu_NextTab, eDigitalInput_Menu_PrevTab, eDigitalInput_Menu_ChooseTab);
	pEnv->SetOkCloseActions(eDigitalInput_Menu_Ok, eDigitalInput_Menu_UnpauseGame);
	pEnv->SetHighlightElementAction(eDigitalInput_Menu_Highlight);
	if (bTestSolo) {
		CreateSampleUI_SingleTab();
	}
	else {
		CreateSampleUI();
	}

	m_pFullScreenOverlay = pWnd->CreateUIOverlay(m_pTextureCtrl, pEnv);
	m_pFullScreenOverlay->InitialiseFullScreen(eOLANone, false, m_pWindow->GetWidth(), m_pWindow->GetHeight());
	IOverlayView* pViewVert = m_pFullScreenOverlay->AddView(50, 100, 200, 500, true);
	IOverlayView* pViewHoriz = m_pFullScreenOverlay->AddView(400, 700, 500, 100, false);
	m_pViewDialog = m_pFullScreenOverlay->AddView(1280 - 200, 0, 200, 800, true);

	pViewVert->AddTextLabel("This is a test of the full screen overlay", 200);
	pViewVert->AddTextLabel("Position: 1.00,2.10,3.5", 200 - 2 * 5);

	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);
	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);
	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);
	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);
	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);
	m_pViewDialog->AddTextField("Hi I am a little dog looking for a biscuit", 150);
	m_pViewDialog->AddTextField("I am puss, the guardian of the biscuits", 150);

	pViewHoriz->AddTextLabel("Position: 1.00,2.10,3.5", 200 - 2 * 5);
	pViewHoriz->AddTextLabel("Angle: 135,45", 200 - 2 * 5);
	//m_pUI->ShowDebugBackgroundOnControl(eOCTNumericSelector, true);
	//m_pUI->ShowDebugBackgroundOnControl(eOCTTextField, true);

	m_pUI->InitialiseTouchAreas();
	m_pFullScreenOverlay->InitialiseTouchAreas();
	m_pFullScreenOverlay->ChangeDisplayState(true);
}

void DiamondGame::CreateSampleUI_SingleTab() {
	IOverlayTab* pTab1 = m_pUI->AddTab("Demo tab 1");
	for (int n = 0; n <= 11; ++n) {
		m_nNumber[n] = n;
	}
	m_nNumber[2] = m_nNumber[3] = 101;
	//pTab1->AddButton("Button1!", eDigitalInput_Menu_DisplayAlert);
	//pTab1->AddButton("Buttony 2 g!", eDigitalInput_Menu_DisplayAlert);
	//pTab1->AddSpacer(100);
	m_sName = "Paul Bentley The Grand Overseer";
	m_sNickname = "Paulster";
	pTab1->AddTextEdit("Name", m_sName, 50);
	pTab1->AddTextEdit("Nickname", m_sNickname, 10);
	//pTab1->AddTextEdit("N1", m_sNickname, 10);
	//pTab1->AddTextEdit("N2", m_sNickname, 10);
	//pTab1->AddTextEdit("N3", m_sNickname, 10);
	//pTab1->AddTextEdit("N4", m_sNickname, 10);
	//pTab1->AddTextEdit("N5", m_sNickname, 10);
	//pTab1->AddTextEdit("N6", m_sNickname, 10);
	//pTab1->AddTextEdit("N7", m_sNickname, 10);
	pTab1->AddSpacer(50);
	m_nNumber[0] = 5;
	pTab1->AddNumericSelector("Number A", m_nNumber + 0, 0, 100, 5);
	for (int n = 3; n < 9; ++n) {
		std::ostringstream ssLabel;
		ssLabel << "Button " << n;
		int nElementId = pTab1->AddButton(ssLabel.str(), eDigitalInput_Menu_DisplayAlert);
		if (n == 3) {
			pTab1->EnableElement(nElementId, false);
		}
	}
	pTab1->AddButton(eTABTClose, "CLOSE", true);
	int nButtonBarId = pTab1->AddButtonBar();
	pTab1->AddButtonToButtonBar(nButtonBarId, eTABTApply, "APPLY", false);
	pTab1->AddButtonToButtonBar(nButtonBarId, eTABTClose, "CLOSE", true);
}

void DiamondGame::CreateSampleUI() {
	IOverlayTab* pTab1 = m_pUI->AddTab("Demo tab 1");
	for (int n = 0; n <= 11; ++n) {
		m_nNumber[n] = n;
	}
	m_nNumber[2] = m_nNumber[3] = 101;
	pTab1->AddButton("Button1!", eDigitalInput_Menu_DisplayAlert);
	pTab1->AddButton("Buttony 2 g!", eDigitalInput_Menu_DisplayAlert);
	pTab1->AddButton("Pu", eDigitalInput_Menu_DisplayAlert);
	m_sName = "Paul";
	m_sNickname = "Paulster";
	pTab1->AddTextEdit("Name", m_sName, 50);
	pTab1->AddTextEdit("Nickname", m_sNickname, 50);
	pTab1->AddNumericSelector("Number A", m_nNumber + 0, 0, 100, 1);
	pTab1->AddNumericSelector("Number B double line", m_nNumber + 1, 0, 9, 1);
	pTab1->AddNumericSelector("Number C double glue!", m_nNumber + 2, 0, 150, 1);
	pTab1->AddNumericSelector("Number D", m_nNumber + 3, 0, 150, 1);
	pTab1->AddNumericSelector("Number E", m_nNumber + 4, 0, 9, 1);
	pTab1->AddNumericSelector("Number F", m_nNumber + 5, 0, 9, 1);
	pTab1->AddNumericSelector("Number g", m_nNumber + 6, 0, 9, 1);
	pTab1->AddNumericSelector("Number H", m_nNumber + 7, 0, 9, 1);
	pTab1->AddText("Line 1\nLine 2\nLine 3 \nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\n");
	pTab1->AddText("Line 1\nLine 2\nLine 3 \nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\n");
	pTab1->AddNumericSelector("Number I", m_nNumber + 8, 0, 20, 1);
	int nElementId = pTab1->AddNumericSelector("Number J", m_nNumber + 9, 0, 20, 1);
	pTab1->EnableElement(nElementId, false);
	pTab1->AddNumericSelector("Number K", m_nNumber + 10, 0, 20, 1);
	pTab1->AddNumericSelector("Number L", m_nNumber + 11, 0, 20, 1);
	pTab1->AddText("This is a test of the ui overlay system. Hopefully the word wrapping works as expected.");
	//pTab1->AddText("This is a second lot of text.");
	IOverlayTab* pTab2 = m_pUI->AddTab("Demo tab 2 12345679abcdef");
	pTab2->AddText("Line 1\nLine 2\nLine 3 \nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20\nLine 21\nLine 22\nLine 23\n");
	pTab2->AddText("Line 1\nLine 2\nLine 3 \nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20\nLine 21\nLine 22\nLine 23\n");
	//	pTab2->AddText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut sit amet tristique mi. Sed nisl diam, pulvinar non vulputate sed, condimentum vel massa. Pellentesque vel erat ac libero dapibus semper. Proin mattis nisl eu efficitur efficitur. Proin ultrices tempor odio, sed malesuada sem feugiat quis. Morbi sit amet dolor massa. Nam vitae arcu nunc. Phasellus ac ante ipsum. Sed iaculis tincidunt nibh et blandit. Cras scelerisque in nunc ut accumsan. Phasellus ut iaculis mi.\n\
	//Maecenas pellentesque eros ac venenatis fringilla.Vestibulum interdum ligula velit, a pellentesque dui auctor nec.Nullam scelerisque ipsum odio, ac congue magna tincidunt ac.Sed maximus quam sed molestie sodales.Integer massa odio, pretium vel sapien in, ullamcorper facilisis neque.Etiam id maximus nisi.Aliquam tincidunt lacinia ipsum et malesuada.Phasellus ullamcorper ornare luctus.Praesent scelerisque velit volutpat porttitor vestibulum.Curabitur interdum elit vel vulputate elementum.Suspendisse potenti.Pellentesque finibus, velit vel feugiat tincidunt, eros quam rutrum ante, id fermentum dui nisl quis purus.Nunc sed nunc est.Mauris convallis maximus lectus, quis rhoncus ex vestibulum sit amet.\n\
	//In at felis aliquet, tincidunt elit ac, laoreet elit.Donec sed velit porttitor, dignissim odio eget, feugiat enim.Nulla porta, augue sed vehicula eleifend, felis nisi sollicitudin sapien, non imperdiet neque est ac magna.Vestibulum suscipit ante quis efficitur pretium.In augue libero, scelerisque vel ligula sed, volutpat mattis mauris.Mauris vitae neque augue.In hac habitasse platea dictumst.Fusce iaculis accumsan sapien, non efficitur elit fringilla at.Nulla sit amet laoreet neque, ut pharetra eros.Maecenas at ante ullamcorper, commodo nulla vitae, feugiat ligula.Maecenas aliquam enim eget magna fringilla, eget vehicula lectus blandit.In nulla sem, placerat at pretium a, tempor sed arcu.\n\
	//Sed laoreet aliquam nulla gravida aliquet.Aliquam posuere lacinia mi quis fermentum.Aliquam commodo dui dui, quis suscipit nisi euismod sed.Duis luctus, nisl ut pharetra scelerisque, nibh leo porta tellus, sed tincidunt nunc magna ac eros.Ut a euismod neque.Proin id odio vel enim venenatis faucibus.Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos.Mauris quis rhoncus lectus.Mauris hendrerit nec turpis eget dictum.Aenean volutpat erat vel eros congue, efficitur sodales nibh aliquam.Etiam egestas lectus ut massa sodales, in convallis ante sollicitudin.Sed vestibulum convallis mauris ut placerat.Curabitur ex enim, tempor ac cursus dapibus, dignissim eget arcu.Maecenas luctus ornare lacinia.Vestibulum accumsan tempus dolor eget tristique.Nunc tortor ex, semper quis suscipit ac, lobortis id diam.\n\
	//Mauris nibh elit, placerat euismod dolor quis, pretium porta lorem.Curabitur interdum imperdiet iaculis.In in massa id mauris euismod condimentum.Phasellus fringilla lobortis odio, a iaculis elit dapibus sed.Donec nibh massa, facilisis vel gravida quis, congue non urna.Vestibulum lobortis elit vel nisl fringilla dignissim.Duis sit amet risus tellus.Sed bibendum finibus efficitur.Sed ante nisl, finibus id tellus sed, scelerisque cursus erat.Praesent efficitur pretium velit.Integer lectus sem, dignissim at dui et, imperdiet convallis metus.");
	IOverlayTab* pTab3 = m_pUI->AddTab("Long long long tab 3");
	pTab3->AddText("Lorem ipsum dolor sit amet, consectetur adipiscing elit. Ut sit amet tristique mi. Sed nisl diam, pulvinar non vulputate sed, condimentum vel massa. Pellentesque vel erat ac libero dapibus semper. Proin mattis nisl eu efficitur efficitur. Proin ultrices tempor odio, sed malesuada sem feugiat quis. Morbi sit amet dolor massa. Nam vitae arcu nunc. Phasellus ac ante ipsum. Sed iaculis tincidunt nibh et blandit. Cras scelerisque in nunc ut accumsan. Phasellus ut iaculis mi.");
	pTab3->AddText("Maecenas pellentesque eros ac venenatis fringilla.Vestibulum interdum ligula velit, a pellentesque dui auctor nec.Nullam scelerisque ipsum odio, ac congue magna tincidunt ac.Sed maximus quam sed molestie sodales.Integer massa odio, pretium vel sapien in, ullamcorper facilisis neque.Etiam id maximus nisi.Aliquam tincidunt lacinia ipsum et malesuada.Phasellus ullamcorper ornare luctus.Praesent scelerisque velit volutpat porttitor vestibulum.Curabitur interdum elit vel vulputate elementum.Suspendisse potenti.Pellentesque finibus, velit vel feugiat tincidunt, eros quam rutrum ante, id fermentum dui nisl quis purus.Nunc sed nunc est.Mauris convallis maximus lectus, quis rhoncus ex vestibulum sit amet.");
	pTab3->AddText("Sed laoreet aliquam nulla gravida aliquet.Aliquam posuere lacinia mi quis fermentum.Aliquam commodo dui dui, quis suscipit nisi euismod sed.Duis luctus, nisl ut pharetra scelerisque, nibh leo porta tellus, sed tincidunt nunc magna ac eros.Ut a euismod neque.Proin id odio vel enim venenatis faucibus.Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos.Mauris quis rhoncus lectus.Mauris hendrerit nec turpis eget dictum.Aenean volutpat erat vel eros congue, efficitur sodales nibh aliquam.Etiam egestas lectus ut massa sodales, in convallis ante sollicitudin.Sed vestibulum convallis mauris ut placerat.Curabitur ex enim, tempor ac cursus dapibus, dignissim eget arcu.Maecenas luctus ornare lacinia.Vestibulum accumsan tempus dolor eget tristique.Nunc tortor ex, semper quis suscipit ac, lobortis id diam.");
	IOverlayTab* pTab4 = m_pUI->AddTab("Long long long tab 4");
	pTab4->AddText("Line 1\nLine 2\nLine 3 \nLine 4\nLine 5\nLine 6\nLine 7\nLine 8\nLine 9\nLine 10\nLine 11\nLine 12\nLine 13\nLine 14\nLine 15\nLine 16\nLine 17\nLine 18\nLine 19\nLine 20\nLine21\n");
	IOverlayTab* pTab5 = m_pUI->AddTab("Demo tab 5");
	IOverlayTab* pTab6 = m_pUI->AddTab("Demo tab 6");
}

bool DiamondGame::InvokeAction( int nAction, unsigned int unInvokeValue) {
	switch (nAction) {
	case eDigitalInput_Menu_DisplayAlert:
		//m_pUI->DisplayAlert("Alert!")
		// display alert.
		return true;
	}
	return false;
}

bool DiamondGame::InvokeTargetAction(unsigned int unTargetHandle, int nAction, unsigned int unInvokeValue) {
	return s_pGame->InvokeAction(nAction, unInvokeValue);
}
