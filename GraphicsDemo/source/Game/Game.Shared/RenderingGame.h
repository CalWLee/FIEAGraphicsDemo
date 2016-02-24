#pragma once
#include "StepTimer.h"
#include "GamePadComponent.h"
#include "Camera.h"
#include "Skybox.h"
#include "Terrain.h"
#include "DemoSystem.h"
namespace Game
{
	class RenderingGame :
		public GameBase
	{
	public:
		RenderingGame();
		~RenderingGame();
		virtual void CreateWindowSizeDependentResources() override;
		virtual void Run() override;
		virtual void Update() override;
		virtual bool Render() override;
		
		virtual void Initialize(void* window, std::function<void(Game::Size&)> getSizeCallBack) override;

		virtual Library::GamePadComponent* GetGamePad() override;

	private:
		DX::StepTimer m_timer;

		std::unique_ptr<Library::GamePadComponent>  mGamePad;
		std::unique_ptr<Library::Camera> mCamera;
		std::unique_ptr<Library::Skybox> mSkybox;
		std::unique_ptr<Library::Terrain> mTerrain;
		std::unique_ptr<Game::DemoSystem> mDemoSystem;

	};

}