#pragma once

#include "pch.h"

namespace Library
{
    class Game::GameBase;
    class GameTime;

    class GameComponent : public RTTI
    {
        RTTI_DECLARATIONS(GameComponent, RTTI)

    public:
        GameComponent();
		GameComponent(Game::GameBase& game);
        virtual ~GameComponent();

		Game::GameBase* GetGame();
		void SetGame(Game::GameBase& game);
        bool Enabled() const;
        void SetEnabled(bool enabled);

        virtual void Initialize();
        virtual void Update(const GameTime& gameTime);

    protected:
		Game::GameBase* mGame;
        bool mEnabled;

    private:
        GameComponent(const GameComponent& rhs);
        GameComponent& operator=(const GameComponent& rhs);
    };
}
