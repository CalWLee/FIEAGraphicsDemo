#pragma once
#include "PointLight.h"
#include "DrawableGameComponent.h"
#include "ProxyModel.h"
using namespace Library;
namespace Rendering
{
	class PointLightDemo :
		public DrawableGameComponent
	{
		RTTI_DECLARATIONS(PointLightDemo, DrawableGameComponent)
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		PointLightDemo(Game::GameBase& game, Camera& camera);
		~PointLightDemo() = default;

		PointLightDemo() = delete;
		PointLightDemo(const PointLightDemo& rhs) = delete;
		PointLightDemo& operator=(const PointLightDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;
	private:

		struct VSCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VSCBufferPerObject() : WorldViewProjection(), World() { }

			VSCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VSCBufferPerFrame
		{
			XMFLOAT3 LightPosition;
			float LightRadius;

			VSCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			VSCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
			{
			}
		};

		struct PSCBufferPerObject
		{
			XMFLOAT3 SpecularColor;
			float SpecularPower;

			PSCBufferPerObject() : SpecularColor(1.0f, 1.0f, 1.0f), SpecularPower(25.0f) { }

			PSCBufferPerObject(const XMFLOAT3& specularColor, float specularPower)
				: SpecularColor(specularColor), SpecularPower(specularPower)
			{
			}
		};

		__declspec(align(16))
		struct PSCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT3 LightPosition;
			float Padding;
			XMFLOAT3 CameraPosition;

			PSCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f),
				LightPosition(0.0f, 0.0f, 0.0f), Padding(D3D11_FLOAT32_MAX), CameraPosition(0.0f, 0.0f, 0.0f)
			{
			}

			PSCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT3& lightPosition, const XMFLOAT3& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor),
				LightPosition(lightPosition), CameraPosition(cameraPosition)
			{
			}
		};

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const size_t Alignment;
		static const float LightModulationRate;
		static const float LightMovementRate;


		//ComPtrs
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;

		VSCBufferPerObject mVertexCBufferPerObjectData;
		VSCBufferPerFrame mVertexCBufferPerFrameData;
		PSCBufferPerObject mPixelCBufferPerObjectData;
		PSCBufferPerFrame mPixelCBufferPerFrameData;

		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		std::unique_ptr<PointLight> mPointLight;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}