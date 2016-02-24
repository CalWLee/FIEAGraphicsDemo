#pragma once
#include "DrawableGameComponent.h"
#include "AnimationPlayer.h"

using namespace DirectX;
using namespace Library;

namespace Library
{
	class PointLight;
	class ProxyModel;
	class Model;
}

namespace Rendering
{
	class AnimationDemo :
		public DrawableGameComponent
	{
		RTTI_DECLARATIONS(AnimationDemo, DrawableGameComponent)

	public:
		AnimationDemo(Game::GameBase& game, Camera& camera);
		~AnimationDemo() = default;

		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		AnimationDemo();
		AnimationDemo(const AnimationDemo& rhs);
		AnimationDemo& operator=(const AnimationDemo& rhs);

		void CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const;
		void CreateVertexBuffer(ID3D11Device* device, VertexSkinnedPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const;

		void UpdateOptions();
		void UpdateAmbientLight(const GameTime& gameTime);
		void UpdatePointLight(const GameTime& gameTime);
		void UpdateSpecularLight(const GameTime& gameTime);

		static const float LightModulationRate;
		static const float LightMovementRate;
		static const uint32_t MAXBONES = 60;

		std::unique_ptr<PointLight> mPointLight;
		XMCOLOR mAmbientColor;
		XMCOLOR mSpecularColor;
		float mSpecularPower;
		XMFLOAT4X4 mWorldMatrix;

		struct VSCBufferPerFrame
		{
			XMFLOAT3 LightPosition = { 0.0f, 0.0f, 0.0f };
			float LightRadius = 10.0f;

			VSCBufferPerFrame()
				: LightPosition(), LightRadius()
			{}

			VSCBufferPerFrame(const XMFLOAT3& lightpos, float radius) :
				LightPosition(lightpos), LightRadius(radius)
			{}
		};
		VSCBufferPerFrame mVSCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerFrame;

		struct VSCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VSCBufferPerObject() : WorldViewProjection(), World() {}
			VSCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) {}

		};
		VSCBufferPerObject mVSCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferPerObject;

		struct VSCBufferSkinning
		{
			XMFLOAT4X4 BoneTransforms[MAXBONES];

			VSCBufferSkinning() : BoneTransforms() {}
		};
		VSCBufferSkinning mVSCBufferSkinningData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVSCBufferSkinning;

		struct PSCBufferPerFrame
		{
			XMFLOAT4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
			XMFLOAT4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			XMFLOAT3 LightPosition = { 0.0f, 0.0f, 0.0f };
			float LightRadius = 10.0f;
			XMFLOAT3 CameraPosition;
			float padding = 0;

			PSCBufferPerFrame() :
				AmbientColor(), LightColor(), LightPosition(), LightRadius(), CameraPosition()
			{}

			PSCBufferPerFrame(const XMFLOAT4& acolor, const XMFLOAT4& lcolor, const XMFLOAT3& lightpos, float radius, const XMFLOAT3& campos) :
				AmbientColor(acolor), LightColor(lcolor), LightPosition(lightpos), LightRadius(radius), CameraPosition(campos)
			{}
		};
		PSCBufferPerFrame mPSCBufferPerFrameData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerFrame;

		struct PSCBufferPerObject
		{
			XMFLOAT3 SpecularColor = { 1.0f, 1.0f, 1.0f };
			float SpecularPower = 25.0f;

			PSCBufferPerObject() : SpecularColor(1,1,1), SpecularPower(25){}
			PSCBufferPerObject(const XMFLOAT3& specColor, float specPower) : SpecularColor(specColor), SpecularPower(specPower)
			{}

		};
		PSCBufferPerObject mPSCBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mPSCBufferPerObject;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> mColorSampler;

		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> mVertexBuffers;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Buffer>> mIndexBuffers;
		std::vector<UINT> mIndexCounts;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> mColorTextures;

		std::unique_ptr<Model> mSkinnedModel;
		std::unique_ptr<AnimationPlayer> mAnimationPlayer;

		RenderStateHelper mRenderStateHelper;
		std::unique_ptr<ProxyModel> mProxyModel;
		std::unique_ptr<SpriteBatch> mSpriteBatch;
		std::unique_ptr<SpriteFont> mSpriteFont;
		XMFLOAT2 mTextPosition;
		bool mManualAdvanceMode;
	};

}