#pragma once
#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
using namespace DirectX;
namespace Library
{
	class Terrain : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(Terrain, DrawableGameComponent);

	public:
		Terrain() = delete;
		Terrain(Game::GameBase& game, Camera& camera, const std::wstring& textureName, const std::wstring& heightMap);
		~Terrain() = default;
		
		static void* operator new(size_t size);
		static void operator delete(void *p);

		Terrain(const Terrain& rhs) = delete;
		Terrain& operator=(const Terrain& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:		
		static const size_t Alignment;

		struct VertexPositionTexture
		{
			XMFLOAT4 Position;
			XMFLOAT2 TextureCoordinates;

			VertexPositionTexture() { }

			VertexPositionTexture(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates)
				: Position(position), TextureCoordinates(textureCoordinates) { }
		};

		__declspec(align(16))
		struct CBufferPerObject
		{
			XMFLOAT4X4 World;
			XMFLOAT4X4 WorldViewProjection;
			float DisplacementScale;

			CBufferPerObject() : World(), WorldViewProjection(), DisplacementScale(0) {}

			CBufferPerObject(const XMFLOAT4X4& w, const XMFLOAT4X4& wvp, const float& f) : World(w), WorldViewProjection(wvp), DisplacementScale(f) { }
		};

		__declspec(align(16))
		struct CBufferPerFrame
		{
			XMFLOAT4 EdgeFactors;
			XMFLOAT2 InsideFactors;

			CBufferPerFrame() : EdgeFactors(), InsideFactors() { }
		};

		__declspec(align(16))
		struct LightBuffer
		{
			XMFLOAT3 LightDirection;
			XMFLOAT3 CameraPosition;

			LightBuffer() : LightDirection(), CameraPosition() { }
		};

		struct PSCBufferPerObject
		{
			XMFLOAT3 SpecularColor;
			float SpecularPower;

			PSCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(10.0f) { }

			PSCBufferPerObject(const XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		struct PSCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;

			PSCBufferPerFrame() : AmbientColor(0.15f, 0.18f, 0.17f, 1.0f), LightColor(0.15f, 0.15f, 0.15f, 1.0f) { }

			PSCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor) : AmbientColor(ambientColor), LightColor(lightColor) { }
		};

		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11HullShader> mHullShader;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> mDomainShader;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mLightBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;

		CBufferPerObject mCBufferPerObjectData;
		CBufferPerFrame mCBufferPerFrameData;
		LightBuffer mLightBufferData;
		PSCBufferPerObject mPSCBufferPerObjectData;
		PSCBufferPerFrame mPSCBufferPerFrameData;

		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		
		std::wstring mColorTexturePath;
		std::wstring mHeightMapPath;

		RenderStateHelper mRenderStateHelper;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mHeightMap;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> mSampler;
	};
}