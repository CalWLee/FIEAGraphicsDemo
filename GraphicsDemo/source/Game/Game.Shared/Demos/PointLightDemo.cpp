#include "pch.h"
#include "Camera.h"
#include "GamePadComponent.h"
#include "PointLightDemo.h"

namespace Rendering
{
	RTTI_DEFINITIONS(PointLightDemo)

		const size_t PointLightDemo::Alignment = 16;
	const float PointLightDemo::LightModulationRate = UCHAR_MAX;
	const float PointLightDemo::LightMovementRate = 10.0f;

	void* PointLightDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

		void PointLightDemo::operator delete(void *p)
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

	PointLightDemo::PointLightDemo(Game::GameBase& game, Camera& camera)
		: DrawableGameComponent(game, camera), mVertexShader(nullptr), mInputLayout(nullptr), mPixelShader(nullptr), mVertexBuffer(nullptr),
		mIndexBuffer(nullptr), mVSCBufferPerObject(nullptr), mVertexCBufferPerObjectData(), mVSCBufferPerFrame(nullptr), mVertexCBufferPerFrameData(),
		mPSCBufferPerObject(nullptr), mPixelCBufferPerObjectData(), mPSCBufferPerFrame(nullptr), mPixelCBufferPerFrameData(),
		mWorldMatrix(MatrixHelper::Identity), mPointLight(nullptr),
		mIndexCount(0), mColorTexture(nullptr), mColorSampler(nullptr), mProxyModel(nullptr)
	{
	}

	void PointLightDemo::Initialize() 
	{
		// Load a compiled vertex shader
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\PointLightVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		// Load a compiled pixel shader
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\PointLightPixelShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedPixelShader() failed.");

		// Create an input layout
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Load the model
		std::unique_ptr<Model> model = std::make_unique<Model>(*mGame, "Content\\Models\\Sphere.bin", true);

		// Create vertex and index buffers for the model
		Mesh* mesh = model->Meshes().at(0);
		CreateVertexBuffer(mGame->GetD3DDevice(), *mesh, mVertexBuffer.ReleaseAndGetAddressOf());
		mesh->CreateIndexBuffer(&mIndexBuffer);
		mIndexCount = static_cast<UINT>(mesh->Indices().size());

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
		std::wstring textureName = L"Content\\Textures\\Earthatday.dds";
		Utility::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(mGame->GetD3DDevice(), textureName.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()), "CreateDDSTextureFromFile() failed.");

		// Create a texture sampler
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mColorSampler.ReleaseAndGetAddressOf()), "ID3D11Device::CreateSamplerState() failed.");

		mProxyModel = std::make_unique<ProxyModel>(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj.bin", 0.5f);
		mProxyModel->Initialize();

		mPointLight = std::make_unique<PointLight>(*mGame);
		mPointLight->SetRadius(50.0f);
		mPointLight->SetPosition(5.0f, 0.0f, 10.0f);
		mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();
		mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();

		mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
		mPixelCBufferPerFrameData.LightColor = ColorHelper::ToFloat4(mPointLight->Color(), true);
		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();
	}

	void PointLightDemo::Update(const GameTime& gameTime) 
	{
		if (!Camera::CameraControlFlag)
		{
			UpdateAmbientLight(gameTime);
			UpdatePointLight(gameTime);
			UpdateSpecularLight(gameTime);
		}
		mProxyModel->Update(gameTime);
	}

	void PointLightDemo::Draw(const GameTime& gameTime) 
	{

		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		UINT stride = sizeof(VertexPositionTextureNormal);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewProjectionMatrix();
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
		XMStoreFloat4x4(&mVertexCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));

		mPixelCBufferPerFrameData.CameraPosition = mCamera->Position();

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVertexCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVertexCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPixelCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPixelCBufferPerObjectData, 0, 0);

		ID3D11Buffer* VSConstantBuffers[] = { mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		direct3DDeviceContext->PSSetShaderResources(0, 1, mColorTexture.GetAddressOf());
		direct3DDeviceContext->PSSetSamplers(0, 1, mColorSampler.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mIndexCount, 0, 0);

		mProxyModel->Draw(gameTime);
	}

	void PointLightDemo::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());
		const std::vector<XMFLOAT3>& normals = mesh.Normals();
		assert(normals.size() == sourceVertices.size());

		std::vector<VertexPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			vertices.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}

		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionTextureNormal) * vertices.size());
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = &vertices[0];
		Utility::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer), "ID3D11Device::CreateBuffer() failed.");
	}

	void PointLightDemo::UpdateAmbientLight(const GameTime& gameTime)
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

	void PointLightDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		// Update point light intensity		
		if (gp->IsButtonDown(GamePadButton::X) && pointLightIntensity < 1.0f)
		{
			pointLightIntensity += elapsedTime;
			pointLightIntensity = min(pointLightIntensity, 1.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}
		if (gp->IsButtonDown(GamePadButton::Y) && pointLightIntensity > 0.0f)
		{
			pointLightIntensity -= elapsedTime;
			pointLightIntensity = max(pointLightIntensity, 0.0f);

			mPixelCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPixelCBufferPerFrameData.LightColor);
		}

		// Move point light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		
		if (gp != nullptr)
		{
			movementAmount.x = gp->GamePad()->GetState(0).thumbSticks.leftX;
			movementAmount.z = -gp->GamePad()->GetState(0).thumbSticks.leftY;
			movementAmount.y = gp->GamePad()->GetState(0).thumbSticks.rightY;


			//Adjust radius of point Light.
			float newRadius = min(max(mPointLight->Radius() - gp->GamePad()->GetState(0).triggers.left + gp->GamePad()->GetState(0).triggers.right, 0), 100);
			
			mPointLight->SetRadius(newRadius);
		}
		
		mVertexCBufferPerFrameData.LightRadius = mPointLight->Radius();


		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
		mVertexCBufferPerFrameData.LightPosition = mPointLight->Position();

		mPixelCBufferPerFrameData.LightPosition = mPointLight->Position();
	}

	void PointLightDemo::UpdateSpecularLight(const GameTime& gameTime)
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
