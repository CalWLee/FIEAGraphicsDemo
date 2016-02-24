#pragma once
#include "DrawableGameComponent.h"
#include "RenderStateHelper.h"
#include "FullScreenQuad.h"
#include "FullScreenRenderTarget.h"


using namespace Library;

namespace Rendering
{
	class PointLightDemo;

	enum class ColorFilter
	{
		ColorFilterGrayScale,
		ColorFilterInverse,
		ColorFilterSepia,
		ColorFilterGeneric,
		ColorFilterTV,
		ColorFilterEnd
	};

	const std::string ColorFilterTechniqueNames[] = { "grayscale_filter", "inverse_filter", "sepia_filter", "generic_filter" };
	const std::string ColorFilterDisplayNames[] = { "Grayscale", "Inverse", "Sepia", "Generic" };


	class ColorFilteringDemo :
		public DrawableGameComponent
	{
		RTTI_DECLARATIONS(ColorFilteringDemo, DrawableGameComponent);

	public:
		ColorFilteringDemo(Game::GameBase& game, Camera& camera);
		~ColorFilteringDemo() = default;
		
		virtual void Initialize() override;
		virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

	private:
		struct PixelShaderBuffer
		{
			XMFLOAT4X4 ColorFilter;

			PixelShaderBuffer() : ColorFilter(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1) {}

			PixelShaderBuffer(XMFLOAT4X4& colorFilter) : ColorFilter(colorFilter) {}
		};

		struct TVFilterBufferPerFrame
		{
			float TotalTime;
			float ElapsedTime;
			float RandomStatic;
			float RandomStatic2;

			TVFilterBufferPerFrame() : TotalTime(0), ElapsedTime(0), RandomStatic(0), RandomStatic2(0)  {};
				TVFilterBufferPerFrame(float t, float et) : TotalTime(t), ElapsedTime(et), RandomStatic(0), RandomStatic2(0)  {};
		};

		struct TVFilterBufferPerObject
		{
			XMFLOAT2 DistortionPanningSpeed;
			XMFLOAT2 ScanlinePanningSpeed;
			XMFLOAT2 ScanlinePanningSpeed2;
			XMFLOAT2 NoisePanningSpeed;
			XMFLOAT2 NoisePanningSpeed2;
			XMFLOAT2 padding;

			TVFilterBufferPerObject() : DistortionPanningSpeed(0, 0), 
				ScanlinePanningSpeed(0, 0), ScanlinePanningSpeed2(0, 0),
				NoisePanningSpeed(0, 0), NoisePanningSpeed2(0, 0), padding(0,0) {};
		};

		void UpdateGenericColorFilter(const GameTime& gameTime);

		static const float BrightnessModulationRate;
		static const XMVECTORF32 BackgroundColor;

		//ComPtrs
		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		std::map<ColorFilter, Microsoft::WRL::ComPtr<ID3D11PixelShader>> mPixelShaders;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;

		Microsoft::WRL::ComPtr<ID3D11Buffer> mPixelShaderBuffer;

		TVFilterBufferPerFrame mTVFilterBufferPerFrameData;
		TVFilterBufferPerObject mTVFilterBufferPerObjectData;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTVBufferPerFrame;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mTVBufferPerObject;

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mDistortionTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mScanlineTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mNoiseTexture;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> mTrilinearSampler;

		std::unique_ptr<FullScreenRenderTarget> mRenderTarget;
		std::unique_ptr<FullScreenQuad> mFullScreenQuad;

		std::unique_ptr<PointLightDemo> mPointLightDemo;

		PixelShaderBuffer mPixelBufferData;

		RenderStateHelper mRenderStateHelper;

		ColorFilter mActiveColorFilter;
	};

}