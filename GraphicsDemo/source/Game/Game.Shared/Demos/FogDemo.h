#pragma once
#include "RenderStateHelper.h"
#include "DirectionalLight.h"
#include "DrawableGameComponent.h"
#include "ProxyModel.h"
using namespace Library;
namespace Rendering
{
	class FogDemo :
		public DrawableGameComponent
	{
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		FogDemo(Game::GameBase& game, Camera& camera);
		~FogDemo() = default;

		FogDemo() = delete;
		FogDemo(const FogDemo& rhs) = delete;
		FogDemo& operator=(const FogDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:

		static const size_t Alignment;
		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;
		static const XMFLOAT4 FogColor;

		struct VSCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VSCBufferPerObject() : WorldViewProjection() { }

			VSCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		__declspec(align(16))
		struct VSCBufferPerFrame
		{
			XMFLOAT4 LightDirection;
			XMFLOAT4 CameraPosition;
			float FogStart;
			float FogRange;

			VSCBufferPerFrame() : LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f), FogStart(20.0f), FogRange(40.0f) { }

			VSCBufferPerFrame(const XMFLOAT4& lightDirection, const XMFLOAT4& cameraPosition)
				: LightDirection(lightDirection), CameraPosition(cameraPosition) { }
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

		struct PSCBufferPerFrame
		{
			XMFLOAT4 AmbientColor;
			XMFLOAT4 LightColor;
			XMFLOAT4 FogColor;
			XMFLOAT4 CameraPosition;

			PSCBufferPerFrame()
				: AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f), FogColor(&ColorHelper::CornflowerBlue[0]),
				CameraPosition(0.0f, 0.0f, 0.0f, 1.0f)
			{
			}

			PSCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor, const XMFLOAT4& fogColor, const XMFLOAT4& cameraPosition)
				: AmbientColor(ambientColor), LightColor(lightColor), FogColor(fogColor), CameraPosition(cameraPosition)
			{
			}
		};


		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdateDirectionalLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);


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

		RenderStateHelper mRenderStateHelper;

		XMFLOAT4X4 mWorldMatrix;
		uint32_t mIndexCount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		std::unique_ptr<DirectionalLight> mDirectionalLight;
		std::unique_ptr<ProxyModel> mProxyModel;

		float mFogStart;
		float mFogRange;
	};
}
