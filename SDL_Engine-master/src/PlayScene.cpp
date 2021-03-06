#include "PlayScene.h"
#include "Game.h"
#include "EventManager.h"

// required for IMGUI
#include "imgui.h"
#include "imgui_sdl.h"
#include "Renderer.h"

#include <string.h>

PlayScene::PlayScene()
{
	PlayScene::start();
}

PlayScene::~PlayScene()
= default;

void PlayScene::draw()
{
	TextureManager::Instance()->draw("background", 400, 300, 0, 255, true);
	if(EventManager::Instance().isIMGUIActive())
	{
		GUI_Function();
	}

	drawDisplayList();
	SDL_SetRenderDrawColor(Renderer::Instance()->getRenderer(), 255, 255, 255, 255);
}

void PlayScene::update()
{
	updateDisplayList();
}

void PlayScene::clean()
{
	removeAllChildren();
}

void PlayScene::handleEvents()
{
	EventManager::Instance().update();

	// handle player movement with GameController
	if (SDL_NumJoysticks() > 0)
	{
		if (EventManager::Instance().getGameController(0) != nullptr)
		{
			const auto deadZone = 10000;
			if (EventManager::Instance().getGameController(0)->LEFT_STICK_X > deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
				m_playerFacingRight = true;
			}
			else if (EventManager::Instance().getGameController(0)->LEFT_STICK_X < -deadZone)
			{
				m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
				m_playerFacingRight = false;
			}
			else
			{
				if (m_playerFacingRight)
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
				}
				else
				{
					m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
				}
			}
		}
	}


	// handle player movement if no Game Controllers found
	if (SDL_NumJoysticks() < 1)
	{
		if (EventManager::Instance().isKeyDown(SDL_SCANCODE_A))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_LEFT);
			m_playerFacingRight = false;
		}
		else if (EventManager::Instance().isKeyDown(SDL_SCANCODE_D))
		{
			m_pPlayer->setAnimationState(PLAYER_RUN_RIGHT);
			m_playerFacingRight = true;
		}
		else
		{
			if (m_playerFacingRight)
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_RIGHT);
			}
			else
			{
				m_pPlayer->setAnimationState(PLAYER_IDLE_LEFT);
			}
		}
	}
	

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_ESCAPE))
	{
		TheGame::Instance()->quit();
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_1))
	{
		TheGame::Instance()->changeSceneState(START_SCENE);
	}

	if (EventManager::Instance().isKeyDown(SDL_SCANCODE_2))
	{
		TheGame::Instance()->changeSceneState(END_SCENE);
	}
}

void PlayScene::start()
{
	// Set GUI Title
	m_guiTitle = "Play Scene";

	// Player Sprite
	m_pPlayer = new Player();
	addChild(m_pPlayer);
	m_playerFacingRight = true;

	//Ball sprite
	m_pBall = new Target();
	addChild(m_pBall);

	TextureManager::Instance()->load("../Assets/textures/Background.png", "background");

	// Back Button
	m_pBackButton = new Button("../Assets/textures/backButton.png", "backButton", BACK_BUTTON);
	m_pBackButton->getTransform()->position = glm::vec2(300.0f, 400.0f);
	m_pBackButton->addEventListener(CLICK, [&]()-> void
	{
		m_pBackButton->setActive(false);
		TheGame::Instance()->changeSceneState(START_SCENE);
	});

	m_pBackButton->addEventListener(MOUSE_OVER, [&]()->void
	{
		m_pBackButton->setAlpha(128);
	});

	m_pBackButton->addEventListener(MOUSE_OUT, [&]()->void
	{
		m_pBackButton->setAlpha(255);
	});
	addChild(m_pBackButton);

	// Next Button
	m_pNextButton = new Button("../Assets/textures/nextButton.png", "nextButton", NEXT_BUTTON);
	m_pNextButton->getTransform()->position = glm::vec2(500.0f, 400.0f);
	m_pNextButton->addEventListener(CLICK, [&]()-> void
	{
		m_pNextButton->setActive(false);
		TheGame::Instance()->changeSceneState(END_SCENE);
	});

	m_pNextButton->addEventListener(MOUSE_OVER, [&]()->void
	{
		m_pNextButton->setAlpha(128);
	});

	m_pNextButton->addEventListener(MOUSE_OUT, [&]()->void
	{
		m_pNextButton->setAlpha(255);
	});

	addChild(m_pNextButton);

	/* Instructions Label */
	m_pInstructionsLabel = new Label("Press the backtick (`) character to toggle Debug View", "Consolas");
	m_pInstructionsLabel->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 500.0f);
	addChild(m_pInstructionsLabel);

	
	//m_pStatsLabel = new Label(std::to_string(PlayScene::distanceLabel), "Consolas");

	
}

void PlayScene::GUI_Function() const
{
	// Always open with a NewFrame
	ImGui::NewFrame();

	// See examples by uncommenting the following - also look at imgui_demo.cpp in the IMGUI filter
	//ImGui::ShowDemoWindow();

	ImGui::Begin("PHYSICS CONTROLLER", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);


	static bool isGravityEnabled = false;
	if (ImGui::Checkbox("Gravity", &isGravityEnabled)) {
		m_pBall->isGravityEnabled = isGravityEnabled;
	}

	ImGui::Separator();

	static int Position = 300;
	if (ImGui::SliderInt("Position X", &Position, 0, 800)) {
		m_pPlayer->getTransform()->position.x = Position;
		m_pBall->getTransform()->position = glm::vec2(Position, 300);
	}

	static float Speed = 0;
	if (ImGui::SliderFloat("Speed", &Speed, 0, 500)) {}

	static float Angle = 0;
	if (ImGui::SliderFloat("Angle", &Angle, 0, 90)) {}

	ImGui::Separator();

	float Velocity_X, Velocity_Y;
	float distanceLabel = 454;
	Label* m_pStatsLabel;	
	if (ImGui::Button("Throw")) {
		Velocity_X = Speed * cos((Angle * 3.14) / 180) * 5; // Set Pixels per metre through velocity here
		Velocity_Y = Speed * sin((Angle * 3.14) / 180) * 5; // Set Pixels per metre through velocity here
		distanceLabel = ((Speed * Speed) * sin(2 * (Angle * 3.14)/180)) / 9.8; // Distance Calculation
		std::cout << "Velocity X: " << Velocity_X << std::endl;
		std::cout << "Velocity Y: " << Velocity_Y << std::endl;
		std::cout << "Distance: " << distanceLabel << std::endl;
		m_pBall->throwSpeed = glm::vec2(abs(Velocity_X), -Velocity_Y);

		//Ignore this
		//m_pStatsLabel = new Label(std::to_string(PlayScene::distanceLabel), "Consolas");
		//m_pStatsLabel->getTransform()->position = glm::vec2(Config::SCREEN_WIDTH * 0.5f, 100.0f);
		//addChild(m_pStatsLabel);

		m_pBall->doThrow();
	}
	
	ImGui::Text("Distance: ",std::to_string((distanceLabel)));

	ImGui::End();
	ImGui::EndFrame();

	// Don't Remove this
	ImGui::Render();
	ImGuiSDL::Render(ImGui::GetDrawData());
	ImGui::StyleColorsDark();
}
