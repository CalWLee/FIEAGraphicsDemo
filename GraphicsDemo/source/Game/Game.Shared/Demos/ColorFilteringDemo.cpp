#include "pch.h"

#include "PointLightDemo.h"
#include "ColorFilteringDemo.h"
#include "GamePadComponent.h"
#include "WICTextureLoader.h"

namespace Rendering
{
	const float ColorFilteringDemo::BrightnessModulationRate = 1.0f;
	const XMVECTORF32 ColorFilteringDemo::BackgroundColor = ColorHelper::CornflowerBlue;

	RTTI_DEFINITIONS(ColorFilteringDemo);

	ColorFilteringDemo::ColorFilteringDemo(Game::GameBase& game, Camera& camera)
		: DrawableGameComponent(game, camera),
		mRenderStateHelper(game), mVertexShader(nullptr),
		mPointLightDemo(nullptr), mRenderTarget(nullptr), mFullScreenQuad(nullptr),
		mActiveColorFilter(ColorFilter::ColorFilterTV)
	{
	}


	void ColorFilteringDemo::Initialize()
	{
		mPointLightDemo = std::unique_ptr<PointLightDemo>(new PointLightDemo(*mGame, *mCamera)); 
		mFullScreenQuad = std::unique_ptr<FullScreenQuad>(new FullScreenQuad(*mGame));
		mRenderTarget = std::unique_ptr<FullScreenRenderTarget>(new FullScreenRenderTarget(*mGame));

		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\ColorFilterVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");
		
		// Load compiled pixel shadere
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ColorFilterGrayscalePixelShader.cso");
		static Microsoft::WRL::ComPtr<ID3D11PixelShader> grayscaleShader;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, grayscaleShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");
		mPixelShaders.emplace(ColorFilter::ColorFilterGrayScale, grayscaleShader);


		compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ColorFilterInversePixelShader.cso");
		static Microsoft::WRL::ComPtr<ID3D11PixelShader> inverseShader;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, inverseShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");
		mPixelShaders.emplace(ColorFilter::ColorFilterInverse, inverseShader);


		compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ColorFilterSepiaPixelShader.cso");
		static Microsoft::WRL::ComPtr<ID3D11PixelShader> sepiaShader;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, sepiaShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");
		mPixelShaders.emplace(ColorFilter::ColorFilterSepia, sepiaShader);


		compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\TVFilterPixelShader.cso");
		static Microsoft::WRL::ComPtr<ID3D11PixelShader> tvShader;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, tvShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");
		mPixelShaders.emplace(ColorFilter::ColorFilterTV, tvShader);


		compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ColorFilterBrightnessPixelShader.cso");
		static Microsoft::WRL::ComPtr<ID3D11PixelShader> genericShader;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, genericShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");
		mPixelShaders.emplace(ColorFilter::ColorFilterGeneric, genericShader);

		std::wstring textureName = L"Content\\Textures\\ScanDistort.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mDistortionTexture.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		textureName = L"Content\\Textures\\Scanlines.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mScanlineTexture.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		textureName = L"Content\\Textures\\Noise.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mNoiseTexture.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");
		mFullScreenQuad->SetInputLayout(mInputLayout);

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.ReleaseAndGetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");


		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mTVFilterBufferPerFrameData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mTVBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		mTVFilterBufferPerFrameData.TotalTime = 0;
		mTVFilterBufferPerFrameData.ElapsedTime = 0;

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mTVFilterBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mTVBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");
		
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mPixelBufferData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPixelShaderBuffer.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		mPixelBufferData.ColorFilter = MatrixHelper::Identity;

		mFullScreenQuad->Initialize();
		mFullScreenQuad->SetCustomUpdateMaterial(
			[&]
		{
			ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();

			direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
			direct3DDeviceContext->PSSetShader(mPixelShaders[mActiveColorFilter].Get(), nullptr, 0);

			if (mActiveColorFilter == ColorFilter::ColorFilterGeneric)
			{
				direct3DDeviceContext->UpdateSubresource(mPixelShaderBuffer.Get(), 0, nullptr, &mPixelBufferData, 0, 0);

				static ID3D11Buffer* PSConstantBuffers[] = { mPixelShaderBuffer.Get() };
				direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);
			}
			else if (mActiveColorFilter == ColorFilter::ColorFilterTV)
			{
				mTVFilterBufferPerFrameData.RandomStatic = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
				mTVFilterBufferPerFrameData.RandomStatic2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);


				direct3DDeviceContext->UpdateSubresource(mTVBufferPerFrame.Get(), 0, nullptr, &mTVFilterBufferPerFrameData, 0, 0);
				direct3DDeviceContext->UpdateSubresource(mTVBufferPerObject.Get(), 0, nullptr, &mTVFilterBufferPerObjectData, 0, 0);

				static ID3D11Buffer* PSConstantBuffers[] = { mTVBufferPerFrame.Get(), mTVBufferPerObject.Get() };
				direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);
			}

			if (mActiveColorFilter != ColorFilter::ColorFilterTV)
			{
				static ID3D11ShaderResourceView* PSShaderResources[] = { mRenderTarget->OutputTexture() };
				direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
			}
			else
			{
				static ID3D11ShaderResourceView* PSShaderResources[] = { mRenderTarget->OutputTexture(), mDistortionTexture.Get(), mScanlineTexture.Get(), mNoiseTexture.Get() };
				direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);

			}
			direct3DDeviceContext->PSSetSamplers(0, 1, mTrilinearSampler.GetAddressOf());

		});

		mTVFilterBufferPerObjectData.NoisePanningSpeed.x = 0.009174f;
		mTVFilterBufferPerObjectData.NoisePanningSpeed.y = -0.064221f;

		mTVFilterBufferPerObjectData.NoisePanningSpeed2.x = 0.0045872f;
		mTVFilterBufferPerObjectData.NoisePanningSpeed2.y = 0.0091743f;

		mTVFilterBufferPerObjectData.ScanlinePanningSpeed.y = 0.1f;
		mTVFilterBufferPerObjectData.ScanlinePanningSpeed2.y = -0.01f;

		mPointLightDemo->Initialize();
	}

	void ColorFilteringDemo::Update(const GameTime &gameTime)
	{
		GamePadComponent* gamepad = mGame->GetGamePad();
		if (gamepad)
		{
			if (gamepad->WasButtonPressedThisFrame(GamePadButton::Start))
			{
				mActiveColorFilter = static_cast<ColorFilter>(static_cast<int>(mActiveColorFilter)+1);
				if (mActiveColorFilter == ColorFilter::ColorFilterEnd)
				{
					mActiveColorFilter = ColorFilter::ColorFilterGrayScale;
				}
			}
		}

		if (mActiveColorFilter == ColorFilter::ColorFilterGeneric)
		{
			UpdateGenericColorFilter(gameTime);
		}
		else if (mActiveColorFilter == ColorFilter::ColorFilterTV)
		{
			float tick = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;
			mTVFilterBufferPerFrameData.TotalTime += tick;
			mTVFilterBufferPerFrameData.ElapsedTime = tick;
		}


		mPointLightDemo->Update(gameTime);
	}

	void ColorFilteringDemo::UpdateGenericColorFilter(const GameTime& gameTime)
	{
		static float brightness = 1.0f;

		if (mGame->GetGamePad())
		{
			GamePadComponent* gamepad = mGame->GetGamePad();

			brightness += BrightnessModulationRate * static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f * gamepad->GamePad()->GetState(0).thumbSticks.rightX;

			brightness = XMMin<float>(XMMax<float>(brightness, 0.0f), 1.0f);
			XMStoreFloat4x4(&mPixelBufferData.ColorFilter, XMMatrixScaling(brightness, brightness, brightness));
			
		}
	}

	void ColorFilteringDemo::Draw(const GameTime &gameTime)
	{
		//Unbind pixel shader resources
		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();
		static ID3D11ShaderResourceView* PSShaderResources[] = { nullptr };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);

		//bind render target
		mRenderTarget->Begin();

		mGame->Clear();
		mGame->Game::GameBase::Draw(gameTime);
		mPointLightDemo->Draw(gameTime);

		mRenderTarget->End();
	
		mGame->Clear();

		mFullScreenQuad->Draw(gameTime);
	}
}