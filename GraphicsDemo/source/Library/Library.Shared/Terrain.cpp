#include "pch.h"
#include "WICTextureLoader.h"
#include "Camera.h"
#include "Terrain.h"
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include "Shlwapi.h"
#endif

namespace Library
{
	const size_t Terrain::Alignment = 16;
	RTTI_DEFINITIONS(Terrain)

		void* Terrain::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

		void Terrain::operator delete(void *p)
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


	Terrain::Terrain(Game::GameBase& game, Camera& camera, const std::wstring& textureName, const std::wstring& heightMap)
		: DrawableGameComponent(game, camera),
		mInputLayout(nullptr), mVertexBuffer(nullptr), mHeightMap(nullptr), mHeightMapPath(heightMap), mColorTexturePath(textureName), mRenderStateHelper(game)
	{
	}
	void Terrain::Initialize()
	{

#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		WCHAR buffer[MAX_PATH];
		GetModuleFileName(nullptr, buffer, MAX_PATH);
		PathRemoveFileSpec(buffer);

		SetCurrentDirectory(std::wstring(buffer).c_str());
#endif
		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\TerrainVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");
		
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(
			inputElementDescriptions,
			ARRAYSIZE(inputElementDescriptions),
			&compiledVertexShader[0],
			compiledVertexShader.size(),
			mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\TerrainPixelShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

		std::string compiledHullShader = Utility::LoadBinaryFile("Content\\Shaders\\TerrainHullShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateHullShader(&compiledHullShader[0], compiledHullShader.size(), nullptr, mHullShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedHullShader() failed.");

		// Load a compiled pixel shader
		std::string compiledDomainShader = Utility::LoadBinaryFile("Content\\Shaders\\TerrainDomainShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateDomainShader(&compiledDomainShader[0], compiledDomainShader.size(), nullptr, mDomainShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedDomainShader() failed.");
		
		float bound = 100, bound2 = 0;


		VertexPositionTexture vertices[] =
		{
			VertexPositionTexture(XMFLOAT4(-bound, bound2, -bound, bound2), XMFLOAT2(0.0f, 0.0f)), // upper-left
			VertexPositionTexture(XMFLOAT4(bound, bound2, -bound, bound2), XMFLOAT2(1.0f, 0.0f)),  // upper-right
			VertexPositionTexture(XMFLOAT4(-bound, bound2, bound, bound2), XMFLOAT2(0.0f, 1.0f)),  // lower-left
			VertexPositionTexture(XMFLOAT4(bound, bound2, bound, bound2), XMFLOAT2(1.0f, 1.0f))    // lower-right
		};

		CreateVertexBuffer(mGame->GetD3DDevice(), vertices, ARRAYSIZE(vertices), mVertexBuffer.ReleaseAndGetAddressOf());


		// Load textures
		std::wstring extension = L".dds";

		//Load the heightmap
		std::size_t found = mHeightMapPath.find(extension);
		if (found != std::string::npos)
		{
			Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), mHeightMapPath.c_str(), nullptr, mHeightMap.ReleaseAndGetAddressOf()));
		}
		else
		{
			Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), mHeightMapPath.c_str(), nullptr, mHeightMap.ReleaseAndGetAddressOf()));
		}

		//load the color Texture.
		found = mColorTexturePath.find(extension);
		if (found != std::string::npos)
		{
			Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), mColorTexturePath.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()));
		}
		else
		{
			Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), mColorTexturePath.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()));
		}

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mSampler.ReleaseAndGetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

		// Create a constant buffer
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(CBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(CBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(LightBuffer);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mLightBuffer.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		mLightBufferData.LightDirection = XMFLOAT3(-1, 1, -1);
		mLightBufferData.CameraPosition = mCamera->Position();

		mCBufferPerObjectData.DisplacementScale = 12;

		XMVECTOR data = XMLoadFloat4(&mCBufferPerFrameData.EdgeFactors);

		for (int i = 0; i < 4; ++i)
		{
			data.m128_f32[i] = 20;
		}

		XMStoreFloat4(&mCBufferPerFrameData.EdgeFactors, data);

		data = XMLoadFloat2(&mCBufferPerFrameData.InsideFactors);
		for (int i = 0; i < 2; ++i)
		{
			data.m128_f32[i] = 20;
		}
		XMStoreFloat2(&mCBufferPerFrameData.InsideFactors, data);
	}

	void Terrain::Update(const GameTime& gameTime)
	{
		//for now do nothing.
	}


	void Terrain::Draw(const GameTime& gameTime)
	{
		XMMATRIX worldMatrix = XMLoadFloat4x4(&MatrixHelper::Identity);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
		mLightBufferData.CameraPosition = mCamera->Position();

		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();

		direct3DDeviceContext->UpdateSubresource(mCBufferPerObject.Get(), 0, nullptr, &mCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mCBufferPerFrame.Get(), 0, nullptr, &mCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mLightBuffer.Get(), 0, nullptr, &mLightBufferData, 0, 0);

		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		
		UINT stride = sizeof(VertexPositionTexture);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		
		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0); 
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mCBufferPerObject.GetAddressOf());

		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0); 
		static ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->PSSetSamplers(0, 1, mSampler.GetAddressOf());
		
		direct3DDeviceContext->HSSetShader(mHullShader.Get(), nullptr, 0);		
		direct3DDeviceContext->HSSetConstantBuffers(0, 1, mCBufferPerFrame.GetAddressOf());

		direct3DDeviceContext->DSSetShader(mDomainShader.Get(), nullptr, 0);		
		static ID3D11Buffer* DSConstantBuffers[] = { mCBufferPerObject.Get(), mLightBuffer.Get() };
		direct3DDeviceContext->DSSetConstantBuffers(0, ARRAYSIZE(DSConstantBuffers), DSConstantBuffers);
		direct3DDeviceContext->DSSetShaderResources(0, 1, mHeightMap.GetAddressOf());
		direct3DDeviceContext->DSSetSamplers(0, 1, mSampler.GetAddressOf());

		direct3DDeviceContext->Draw(4, 0);
		
		direct3DDeviceContext->HSSetShader(nullptr, nullptr, 0);
		direct3DDeviceContext->DSSetShader(nullptr, nullptr, 0);
	}

	void Terrain::CreateVertexBuffer(ID3D11Device* device, VertexPositionTexture* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = sizeof(VertexPositionTexture) * vertexCount;
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		Utility::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}
}