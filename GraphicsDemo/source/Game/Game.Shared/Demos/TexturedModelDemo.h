#pragma once

#include "DrawableGameComponent.h"

using namespace Library;

namespace Library
{
	class Mesh;
}

namespace Rendering
{
	class TexturedModelDemo final : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(TexturedModelDemo, DrawableGameComponent)

	public:
		TexturedModelDemo(Game::GameBase& game, Library::Camera& camera);
		~TexturedModelDemo() = default;

		virtual void Initialize() override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct VertexPositionTexture
		{
			XMFLOAT4 Position;
			XMFLOAT2 TextureCoordinates;

			VertexPositionTexture() { }

			VertexPositionTexture(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates)
				: Position(position), TextureCoordinates(textureCoordinates) { }
		};

		struct CBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			CBufferPerObject() : WorldViewProjection() { }

			CBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) { }
		};
		TexturedModelDemo();
		TexturedModelDemo(const TexturedModelDemo& rhs);
		TexturedModelDemo& operator=(const TexturedModelDemo& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mConstantBuffer;

		CBufferPerObject mCBufferPerObject;
		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;
	};
}
