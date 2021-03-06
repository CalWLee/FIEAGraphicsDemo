#include "pch.h"
#include "WICTextureLoader.h"
#include "Camera.h"
#include "TransparencyDemo.h"
#include "GamePadComponent.h"

namespace Rendering
{
	const size_t TransparencyDemo::Alignment = 16;
	const XMFLOAT2 TransparencyDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const float TransparencyDemo::LightModulationRate = UCHAR_MAX;
	const XMFLOAT4 TransparencyDemo::FogColor = XMFLOAT4(&ColorHelper::CornflowerBlue[0]);

	RTTI_DEFINITIONS(TransparencyDemo)


	void* TransparencyDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

		void TransparencyDemo::operator delete(void *p)
	{
		if (p != nullptr)
		{
#if defined(DEBUG) || defined(_DEBUG)
			_aligned_free_dbg(p);
#else
			_aligned_free(p);
#endif
		}
	}

	TransparencyDemo::TransparencyDemo(Game::GameBase& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		mIndexBuffer(nullptr), mVSCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mVSCBufferPerFrame(nullptr), mVertexCBufferPerFrameData(),
		mPSCBufferPerObject(nullptr), mPixelCBufferPerObjectData(), mPSCBufferPerFrame(nullptr), mPixelCBufferPerFrameData(),
		mWorldMatrix(MatrixHelper::Identity), mDirectionalLight(nullptr), mVertexCount(0), mColorTexture(nullptr), mTransparencyMap(nullptr), mTrilinearSampler(nullptr),
		mProxyModel(nullptr), mFogStart(75.0f), mFogRange(40.0f), mRenderStateHelper(game)
	{
	}

	void TransparencyDemo::Initialize()
	{

		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\TransparencyVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\TransparencyPixelShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Create a vertex buffer
		VertexPositionTextureNormal vertices[] =
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
		};

		mVertexCount = ARRAYSIZE(vertices);

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionTextureNormal) * mVertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, mVertexBuffer.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		
		// Create constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerObjectData);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mVertexCBufferPerFrameData);
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerObjectData);
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		constantBufferDesc.ByteWidth = sizeof(mPixelCBufferPerFrameData);
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		// Load a texture
		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		textureName = L"Content\\Textures\\AlphaMask_32bpp.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mTransparencyMap.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");


		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mTrilinearSampler.ReleaseAndGetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

		mProxyModel = std::make_unique<ProxyModel>(*mGame, *mCamera, "Content\\Models\\DirectionalLightProxy.obj.bin", 0.5f);
		mProxyModel->Initialize();
		mProxyModel->SetPosition(10.0f, 0.0, 0.0f);
		mProxyModel->ApplyRotation(XMMatrixRotationY(XM_PIDIV2));

		mDirectionalLight = std::make_unique<DirectionalLight>(*mGame);
		const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
		mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
		mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

		mVertexCBufferPerFrameData.FogStart = mFogStart;
		mVertexCBufferPerFrameData.FogRange = mFogRange;
		mPixelCBufferPerFrameData.FogColor = FogColor;

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));
	}

	void TransparencyDemo::Update(const GameTime& gameTime)
	{
		if (!Camera::CameraControlFlag)
		{
			UpdateAmbientLight(gameTime);
			UpdateDirectionalLight(gameTime);
			UpdateSpecularLight(gameTime);
		}
		mProxyModel->Update(gameTime);
	}

	void TransparencyDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = static_cast<UINT>(sizeof(VertexPositionTextureNormal));
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

		const XMFLOAT3& cameraPosition = mCamera->Position();
		mVertexCBufferPerFrameData.CameraPosition = XMFLOAT4(cameraPosition.x, cameraPosition.y, cameraPosition.z, 1.0f);
		mPixelCBufferPerFrameData.CameraPosition = mVertexCBufferPerFrameData.CameraPosition;

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		static ID3D11Buffer* VSConstantBuffers[] = { mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		static ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mTransparencyMap.Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, mTrilinearSampler.GetAddressOf());

		mRenderStateHelper.SaveAll();

		direct3DDeviceContext->OMSetBlendState(BlendStates::AlphaBlending.Get(), 0, 0xFFFFFFFF);
		direct3DDeviceContext->Draw(mVertexCount, 0);
		mProxyModel->Draw(gameTime);

		mRenderStateHelper.RestoreBlendState();
		mRenderStateHelper.RestoreAll();
	}

	void TransparencyDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = 0.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		if (gp->IsButtonDown(GamePadButton::A) && ambientIntensity < 1.0f)
		{
			ambientIntensity += elapsedTime;
			ambientIntensity = min(ambientIntensity, 1.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}

		if (gp->IsButtonDown(GamePadButton::B) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= elapsedTime;
			ambientIntensity = max(ambientIntensity, 0.0f);

			mPixelCBufferPerFrameData.AmbientColor = XMFLOAT4(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);
		}

	}

	void TransparencyDemo::UpdateDirectionalLight(const GameTime& gameTime)
	{
		static float directionalIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		// Update directional light intensity		
		if (gp->IsButtonDown(GamePadButton::Y) && directionalIntensity < 1.0f)
		{
			directionalIntensity += elapsedTime;
			directionalIntensity = min(directionalIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (gp->IsButtonDown(GamePadButton::X) && directionalIntensity > 0.0f)
		{
			directionalIntensity -= elapsedTime;
			directionalIntensity = max(directionalIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(directionalIntensity, directionalIntensity, directionalIntensity, 1.0f);
			mDirectionalLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}


		// Rotate directional light
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;
		if (gp->CurrentState().IsLeftThumbStickRight())
		{
			rotationAmount.x += LightRotationRate.x * elapsedTime;
		}
		if (gp->CurrentState().IsLeftThumbStickLeft())
		{
			rotationAmount.x -= LightRotationRate.x * elapsedTime;
		}
		if (gp->CurrentState().IsLeftThumbStickUp())
		{
			rotationAmount.y += LightRotationRate.y * elapsedTime;
		}
		if (gp->CurrentState().IsLeftThumbStickDown())
		{
			rotationAmount.y -= LightRotationRate.y * elapsedTime;
		}

		XMMATRIX lightRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			lightRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX lightRotationAxisMatrix = XMMatrixRotationAxis(mDirectionalLight->RightVector(), rotationAmount.y);
			lightRotationMatrix *= lightRotationAxisMatrix;
		}

		if (rotationAmount.x != 0.0f || rotationAmount.y != 0.0f)
		{
			mDirectionalLight->ApplyRotation(lightRotationMatrix);
			mProxyModel->ApplyRotation(lightRotationMatrix);

			const XMFLOAT3& lightdirection = mDirectionalLight->Direction();
			mVertexCBufferPerFrameData.LightDirection = XMFLOAT4(lightdirection.x, lightdirection.y, lightdirection.z, 0.0f);
		}
	}

	void TransparencyDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		if (gp->IsButtonDown(GamePadButton::DPadUp) && specularIntensity < 1.0f)
		{
			specularIntensity += elapsedTime;
			specularIntensity = min(specularIntensity, 1.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		if (gp->IsButtonDown(GamePadButton::DPadDown) && specularIntensity > 0.0f)
		{
			specularIntensity -= elapsedTime;
			specularIntensity = max(specularIntensity, 0.0f);

			mPixelCBufferPerObjectData.SpecularColor = XMFLOAT3(specularIntensity, specularIntensity, specularIntensity);
		}

		static float specularPower = mPixelCBufferPerObjectData.SpecularPower;

		if (gp->IsButtonDown(GamePadButton::DPadRight) && specularPower < UCHAR_MAX)
		{
			specularPower += LightModulationRate * elapsedTime;
			specularPower = min(specularPower, static_cast<float>(UCHAR_MAX));

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}

		if (gp->IsButtonDown(GamePadButton::DPadLeft) && specularPower > 1.0f)
		{
			specularPower -= LightModulationRate * elapsedTime;
			specularPower = max(specularPower, 1.0f);

			mPixelCBufferPerObjectData.SpecularPower = specularPower;
		}

	}
}