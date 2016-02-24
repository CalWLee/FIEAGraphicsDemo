#pragma once
#include "pch.h"
#include "GameComponent.h"
using namespace DirectX::PackedVector;
namespace Library
{
	class Light : public GameComponent
	{
		RTTI_DECLARATIONS(Light, GameComponent)

	public:
		Light(Game::GameBase& game);
		virtual ~Light();

		const XMCOLOR& Color() const;
		XMVECTOR ColorVector() const;
		void SetColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a);
		void SetColor(const XMFLOAT4& color);
		void SetColor(const XMCOLOR& color);
		void SetColor(FXMVECTOR color);

	protected:
		XMCOLOR mColor;
	};

}