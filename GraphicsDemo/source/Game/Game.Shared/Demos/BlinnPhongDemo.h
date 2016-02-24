#pragma once
#include "DirectionalLight.h"
#include "DrawableGameComponent.h"
#include "ProxyModel.h"
using namespace Library;
namespace Rendering
{
	class BlinnPhongDemo : public DrawableGameComponent
	{
		RTTI_DECLARATIONS(BlinnPhongDemo, DrawableGameComponent)

	public:
		BlinnPhongDemo(Game::GameBase& game, Camera& camera);
		~BlinnPhongDemo() = default;

		BlinnPhongDemo() = delete;
		BlinnPhongDemo(const BlinnPhongDemo& rhs) = delete;
		BlinnPhongDemo& operator=(const BlinnPhongDemo& rhs) = delete;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:

		static const XMFLOAT2 LightRotationRate;
		static const float LightModulationRate;

		struct VSCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VSCBufferPerObject() : WorldViewProjection() { }

			VSCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};

		struct VSCBufferPerFrame
		{
			XMFLOAT4 LightDirection;
			XMFLOAT4 CameraPosition;

			VSCBufferPerFrame() : LightDirection(0.0f, 0.0f, -1.0f, 0.0f), CameraPosition(0.0f, 0.0f, 0.0f, 1.0f) { }

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

			PSCBufferPerFrame() : AmbientColor(0.0f, 0.0f, 0.0f, 0.0f), LightColor(1.0f, 1.0f, 1.0f, 1.0f) { }

			PSCBufferPerFrame(const XMFLOAT4& ambientColor, const XMFLOAT4& lightColor) : AmbientColor(ambientColor), LightColor(lightColor) { }
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


		XMFLOAT4X4 mWorldMatrix;
		UINT mIndexCount;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		std::unique_ptr<DirectionalLight> mDirectionalLight;
		std::unique_ptr<ProxyModel> mProxyModel;
	};
}