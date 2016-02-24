#pragma once
#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
using namespace Library;
using namespace DirectX;

namespace Library
{
	class PointLight;
	class ProxyModel;
	class Projector;
	class Frustum;
	class DepthMap;
	class RenderableFrustum;
	class Camera;
}

namespace Rendering
{
	enum class ShadowMappingTechnique
	{
		ShadowMappingTechniqueSimple = 0,
		ShadowMappingTechniqueManualPCF,
		ShadowMappingTechniquePCF,
		ShadowMappingTechniqueEnd
	};

	const std::string ShadowMappingClassNames[] = { "ShadowMapper", "ManualPCFShadowMapper", "PCFShadowMapper" };
	const std::string ShadowMappingDisplayNames[] = { "Shadow Mapping Simple", "Shadow Mapping w/ Manual PCF", "Shadow Mapping w/ PCF" };
	const std::string DepthMappingTechniqueNames[] = { "create_depthmap", "create_depthmap", "create_depthmap_w_bias", };

	class ShadowMappingDemo :
		public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ShadowMappingDemo, DrawableGameComponent);
	public:
		static void* operator new(size_t size);
		static void operator delete(void *p);

		ShadowMappingDemo(Game::GameBase& game, Camera& camera);
		~ShadowMappingDemo() = default;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		ShadowMappingDemo();
		ShadowMappingDemo(const ShadowMappingDemo& rhs);
		ShadowMappingDemo& operator=(const ShadowMappingDemo& rhs);

		void UpdateDepthBias(const GameTime& gameTime);
		void UpdateDepthBiasState();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLightAndProjector(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		void CreateVertexBuffer(ID3D11Device* device, VertexPosition* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;
		void CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;

		void InitializeProjectedTextureScalingMatrix();

		static const float LightModulationRate;
		static const float LightMovementRate;
		static const XMFLOAT2 LightRotationRate;
		static const UINT DepthMapWidth;
		static const UINT DepthMapHeight;
		static const RECT DepthMapDestinationRectangle;
		static const float DepthBiasModulationRate;
		static const size_t Alignment;

		XMCOLOR mAmbientColor;
		std::unique_ptr<PointLight> mPointLight;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		std::unique_ptr<ProxyModel> mProxyModel;
		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<Projector> mProjector;
		std::unique_ptr<Frustum> mProjectorFrustum;
		std::unique_ptr<RenderableFrustum> mRenderableProjectorFrustum;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlanePositionVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlanePositionUVNormalVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPlaneIndexBuffer;
		UINT mPlaneVertexCount;
		XMFLOAT4X4 mPlaneWorldMatrix;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mCheckerboardTexture;
		
		struct PointLightCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;
			XMFLOAT4X4 ProjectiveTextureMatrix;

			PointLightCBufferPerObject() : WorldViewProjection(), World(), ProjectiveTextureMatrix() {}

			PointLightCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world, const XMFLOAT4X4& ptm) : WorldViewProjection(wvp), World(world), ProjectiveTextureMatrix(ptm) {}
		};
		PointLightCBufferPerObject mVSCBufferPerObjectData;
		struct PointLightCBufferPerFrame
		{
			XMFLOAT3 LightPosition;
			float LightRadius;

			PointLightCBufferPerFrame() : LightPosition(0.0f, 0.0f, 0.0f), LightRadius(10.0f) { }

			PointLightCBufferPerFrame(const XMFLOAT3& lightPosition, float lightRadius)
				: LightPosition(lightPosition), LightRadius(lightRadius)
			{
			}
		};

		PointLightCBufferPerFrame mVSCBufferPerFrameData;

		struct PSCBufferPerFrame
		{
			XMFLOAT4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
			XMFLOAT4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			XMFLOAT3 LightPosition = { 0.0f, 0.0f, 0.0f };
			float LightRadius = 10.0f;

			XMFLOAT3 CameraPosition;
			float padding = 0;

			XMFLOAT2 ShadowMapSize = { 1024.0f, 1024.0f };
			XMFLOAT2 padding2 = { 0, 0 };
		};
		PSCBufferPerFrame mPSCBufferPerFrameData;

		struct PSCBufferPerObject
		{			
			XMFLOAT4 SpecularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			float SpecularPower = 25.0f;
			XMFLOAT3 padding = { 0, 0, 0 };

		};
		PSCBufferPerObject mPSCBufferPerObjectData;

		struct DepthMapCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			
			DepthMapCBufferPerObject() : WorldViewProjection() { }
			DepthMapCBufferPerObject(const XMFLOAT4X4& wvp) : WorldViewProjection(wvp) {}
		};
		DepthMapCBufferPerObject mDepthMapBufferPerObjectData;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mDepthMapShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout; 
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mDepthMapInputLayout;

		Microsoft::WRL::ComPtr<ID3D11ClassLinkage> mClassLinkage;
		std::map <ShadowMappingTechnique, Microsoft::WRL::ComPtr <ID3D11ClassInstance>> mClassInstances;
		ShadowMappingTechnique mCurrentTechnique;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mDepthMapCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelPositionVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelPositionUVNormalVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mModelIndexBuffer;
		UINT mModelIndexCount;
		XMFLOAT4X4 mModelWorldMatrix;
		XMFLOAT4X4 mProjectedTextureScalingMatrix;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mColorTexture;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mShadowMapSampler;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> mPCFShadowMapSampler;

		std::unique_ptr<DepthMap> mDepthMap;
		bool mDrawDepthMap;

		
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;

		XMFLOAT2 mTextPosition;
		
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> mDepthBiasState;
		float mDepthBias;
		float mSlopeScaledDepthBias;
	};
}