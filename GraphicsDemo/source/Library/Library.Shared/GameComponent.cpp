#include "pch.h"
#include "GameTime.h"

namespace Library
{
    RTTI_DEFINITIONS(GameComponent)

    GameComponent::GameComponent()
        : mGame(nullptr), mEnabled(true)
    {
    }

	GameComponent::GameComponent(Game::GameBase& game)
        : mGame(&game), mEnabled(true)
    {
    }

    GameComponent::~GameComponent()
    {
    }

	Game::GameBase* GameComponent::GetGame()
    {
        return mGame;
    }

	void GameComponent::SetGame(Game::GameBase& game)
    {
        mGame = &game;
    }

    bool GameComponent::Enabled() const
    {
        return mEnabled;
    }

    void GameComponent::SetEnabled(bool enabled)
    {
        mEnabled = enabled;
    }

    void GameComponent::Initialize()
    {
    }

    void GameComponent::Update(const GameTime& gameTime)
    {
    }
	GameComponent& GameComponent::operator=(const GameComponent & rhs)
	{
		// TODO: insert return statement here
		mEnabled = rhs.mEnabled;
		mGame = rhs.mGame;

		return *this;
	}
}
