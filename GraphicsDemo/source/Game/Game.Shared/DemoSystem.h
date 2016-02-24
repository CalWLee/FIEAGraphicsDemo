#pragma once

#include "DrawableGameComponent.h"
using namespace Library;
namespace Game
{
	class DemoSystem final : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(DemoSystem, DrawableGameComponent)

	public:
		DemoSystem(Game::GameBase& game, Library::Camera& camera);
		~DemoSystem() = default;

		DemoSystem() = delete;
		DemoSystem(const DemoSystem& rhs) = delete;
		DemoSystem& operator=(const DemoSystem& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		std::vector<std::pair<std::unique_ptr<DrawableGameComponent>, bool>> mDrawableComponents;
		std::unique_ptr<DrawableGameComponent> mSkyBox;
		uint32_t mCurrentDemoIndex;
	};

}