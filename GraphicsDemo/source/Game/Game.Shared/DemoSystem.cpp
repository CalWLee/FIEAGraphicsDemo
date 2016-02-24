#include "pch.h"
#include "Skybox.h"
#include "DemoSystem.h"
#include "Camera.h"
#include "Projector.h"
#include "RenderableFrustum.h"
#include "DepthMap.h"
#include "GamePadComponent.h"
#include "Demos\TexturedModelDemo.h"
#include "Demos\BlinnPhongDemo.h"
#include "Demos\PointLightDemo.h"
#include "Demos\EnvironmentMappingDemo.h"
#include "Demos\FogDemo.h"
#include "Demos\TransparencyDemo.h"
#include "Demos\ColorFilteringDemo.h"
#include "Demos\ShadowMappingDemo.h"
#include "Demos\AnimationDemo.h"
#include "Terrain.h"

using namespace Library;

namespace Game
{
	RTTI_DEFINITIONS(DemoSystem)
	DemoSystem::DemoSystem(Game::GameBase& game, Library::Camera& camera)
		: DrawableGameComponent(game, camera), mCurrentDemoIndex(0), mDrawableComponents()
	{
	}

	void DemoSystem::Initialize()
	{
		//mTerrain = std::unique_ptr<Terrain>(new Terrain(*this, *mCamera, L"Content\\Textures\\colorm01.bmp", L"Content\\Textures\\Heightmap.jpg"));
		//mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Terrain>(new Terrain(*mGame, *mCamera, L"Content\\Textures\\colorm01.bmp", L"Content\\Textures\\Heightmap.jpg")), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::ColorFilteringDemo>(new Rendering::ColorFilteringDemo(*mGame, *mCamera)), true));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::AnimationDemo>(new Rendering::AnimationDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::TexturedModelDemo>(new Rendering::TexturedModelDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::BlinnPhongDemo>(new Rendering::BlinnPhongDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::PointLightDemo>(new Rendering::PointLightDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::EnvironmentMappingDemo>(new Rendering::EnvironmentMappingDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::FogDemo>(new Rendering::FogDemo(*mGame, *mCamera)), false));
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::TransparencyDemo>(new Rendering::TransparencyDemo(*mGame, *mCamera)), false)); 
		mDrawableComponents.push_back(std::make_pair(std::unique_ptr<Rendering::ShadowMappingDemo>(new Rendering::ShadowMappingDemo(*mGame, *mCamera)), false));
		
		for (auto& pair : mDrawableComponents)
		{
			auto& component = pair.first;
			component->Initialize();
		}
	}

	void DemoSystem::Update(const GameTime& gameTime)
	{
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();
		if (gp->WasButtonPressedThisFrame(GamePadButton::LeftShoulder) || gp->WasButtonPressedThisFrame(GamePadButton::RightShoulder))
		{
			mDrawableComponents[mCurrentDemoIndex].first->SetEnabled(false);
			mDrawableComponents[mCurrentDemoIndex].first->SetVisible(false);

			if (gp->WasButtonPressedThisFrame(GamePadButton::LeftShoulder))
			{

				if (mCurrentDemoIndex == 0)
				{
					mCurrentDemoIndex = static_cast<uint32_t>(mDrawableComponents.size() - 1);
				}
				else
				{
					--mCurrentDemoIndex;
				}
			}
			else if (gp->WasButtonPressedThisFrame(GamePadButton::RightShoulder))
			{
				mCurrentDemoIndex = (mCurrentDemoIndex + 1) % mDrawableComponents.size();;
			}

			mDrawableComponents[mCurrentDemoIndex].first->SetEnabled(true);
			mDrawableComponents[mCurrentDemoIndex].first->SetVisible(true);

			mCamera->Reset();
			mCamera->SetPosition(0.0f, 0.0f, 25.0f);
		}
		mDrawableComponents[mCurrentDemoIndex].first->Update(gameTime);
	}


	void DemoSystem::Draw(const GameTime& gameTime)
	{
		if (mDrawableComponents[mCurrentDemoIndex].second)
		{
			mDrawableComponents[mCurrentDemoIndex].first->Draw(gameTime);
		}
		else
		{
			mGame->Clear();
			mGame->Game::GameBase::Draw(gameTime);
			mDrawableComponents[mCurrentDemoIndex].first->Draw(gameTime);
		}
	}
}