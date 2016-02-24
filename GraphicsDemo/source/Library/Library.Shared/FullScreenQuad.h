#pragma once

#include <functional>
#include "DrawableGameComponent.h"

namespace Library
{
	class FullScreenQuad : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(FullScreenQuad, DrawableGameComponent)

	public:
		FullScreenQuad(Game::GameBase& game);
		//FullScreenQuad(Game::GameBase& game, Material& material);
		~FullScreenQuad() = default;

		void SetInputLayout(Microsoft::WRL::ComPtr<ID3D11InputLayout>& layout);

		void SetCustomUpdateMaterial(std::function<void()> callback);

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		FullScreenQuad();
		FullScreenQuad(const FullScreenQuad& rhs);
		FullScreenQuad& operator=(const FullScreenQuad& rhs);

		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		UINT mIndexCount;
		std::function<void()> mCustomUpdateMaterial;
	};
}