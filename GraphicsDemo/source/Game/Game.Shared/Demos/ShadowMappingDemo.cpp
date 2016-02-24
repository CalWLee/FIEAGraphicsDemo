#include "pch.h"
#include "PointLight.h"
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "ProxyModel.h"
#include "Projector.h"
#include "RenderableFrustum.h"
#include "DepthMap.h"
#include "Camera.h"
#include "GamePadComponent.h"
#include "ShadowMappingDemo.h"

namespace Rendering
{
	RTTI_DEFINITIONS(ShadowMappingDemo);

	const float ShadowMappingDemo::LightModulationRate = UCHAR_MAX;
	const float ShadowMappingDemo::LightMovementRate = 10.0f;
	const XMFLOAT2 ShadowMappingDemo::LightRotationRate = XMFLOAT2(XM_2PI, XM_2PI);
	const UINT ShadowMappingDemo::DepthMapWidth = 1024U;
	const UINT ShadowMappingDemo::DepthMapHeight = 1024U;
	const RECT ShadowMappingDemo::DepthMapDestinationRectangle = { 0, 344, 256, 600 };
	const float ShadowMappingDemo::DepthBiasModulationRate = 10000;
	const size_t ShadowMappingDemo::Alignment = 16;


	void* ShadowMappingDemo::operator new(size_t size)
	{
#if defined(DEBUG) || defined(_DEBUG)
		return _aligned_malloc_dbg(size, Alignment, __FILE__, __LINE__);
#else
		return _aligned_malloc(size, Alignment);
#endif
	}

	void ShadowMappingDemo::operator delete(void *p)
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

	ShadowMappingDemo::ShadowMappingDemo(Game::GameBase& game, Camera& camera)
		: DrawableGameComponent(game, camera), mCheckerboardTexture(nullptr),
		mPlanePositionVertexBuffer(nullptr), mPlanePositionUVNormalVertexBuffer(nullptr), mPlaneIndexBuffer(nullptr), mPlaneVertexCount(0),
		mAmbientColor(1.0f, 1.0f, 1.0, 0.0f), mPointLight(nullptr),
		mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mPlaneWorldMatrix(MatrixHelper::Identity), mProxyModel(nullptr),
		mProjector(nullptr), mRenderableProjectorFrustum(nullptr),
		mProjectorFrustum(std::make_unique<Frustum>(XMMatrixIdentity())),
		mProjectedTextureScalingMatrix(MatrixHelper::Zero), mRenderStateHelper(game),
		mModelPositionVertexBuffer(nullptr), mModelPositionUVNormalVertexBuffer(nullptr), mModelIndexBuffer(nullptr), mModelIndexCount(0),
		mModelWorldMatrix(MatrixHelper::Identity), mDepthMap(nullptr), mDrawDepthMap(true),
		mVertexShader(nullptr), mDepthMapShader(nullptr), mPixelShader(nullptr),
		mInputLayout(nullptr), mDepthMapInputLayout(nullptr), mDepthMapCBufferPerObject(nullptr), mVSCBufferPerObject(nullptr), mVSCBufferPerFrame(nullptr),
		mPSCBufferPerObject(nullptr), mPSCBufferPerFrame(nullptr), mTextPosition(0.0f, 40.0f),
		mDepthBiasState(nullptr), mDepthBias(0), mSlopeScaledDepthBias(2.0f), mCurrentTechnique(ShadowMappingTechnique::ShadowMappingTechniqueSimple)
	{
	}

	void ShadowMappingDemo::Initialize()
	{
		// Initialize Textures
		mDepthMap = std::make_unique<DepthMap>(*mGame, DepthMapWidth, DepthMapHeight);

		//Load DepthMap Shaders
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\DepthMapVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mDepthMapShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mDepthMapInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		// Create a constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(DepthMapCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mDepthMapCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PointLightCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PointLightCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(PSCBufferPerFrame);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mPSCBufferPerFrame.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");


		//Load ShadowMap shaders
		compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\ShadowMappingVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		
		//Shadow Map Materials
		D3D11_INPUT_ELEMENT_DESC shadowMapInputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(shadowMapInputElementDescriptions, ARRAYSIZE(shadowMapInputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateClassLinkage(mClassLinkage.ReleaseAndGetAddressOf()), "Cannot Create Class Linkage.");
		
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\ShadowMappingPixelShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), mClassLinkage.Get(), mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");
		
		for (uint32_t i = 0; i < static_cast<uint32_t>(ShadowMappingTechnique::ShadowMappingTechniqueEnd); ++i)
		{
			Microsoft::WRL::ComPtr<ID3D11ClassInstance> instance = nullptr;
			mClassLinkage->CreateClassInstance(ShadowMappingClassNames[i].c_str(), 0, 0, 0, 0, instance.ReleaseAndGetAddressOf());
			mClassInstances.emplace(static_cast<ShadowMappingTechnique>(i), instance);
		}
		
		// Plane vertex buffers
		VertexPositionTextureNormal positionUVNormalVertices[] =
		{
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),

			VertexPositionTextureNormal(XMFLOAT4(-0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f), Vector3Helper::Backward),
			VertexPositionTextureNormal(XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f), Vector3Helper::Backward),
		};

		mPlaneVertexCount = ARRAYSIZE(positionUVNormalVertices);
		std::vector<VertexPositionNormal> positionNormalVertices;
		positionNormalVertices.reserve(mPlaneVertexCount);
		std::vector<VertexPosition> positionVertices;
		positionVertices.reserve(mPlaneVertexCount);
		for (UINT i = 0; i < mPlaneVertexCount; i++)
		{
			positionNormalVertices.push_back(VertexPositionNormal(positionUVNormalVertices[i].Position, positionUVNormalVertices[i].Normal));
			positionVertices.push_back(VertexPosition(positionUVNormalVertices[i].Position));
		}

		CreateVertexBuffer(mGame->GetD3DDevice(), &positionVertices[0], mPlaneVertexCount, mPlanePositionVertexBuffer.ReleaseAndGetAddressOf());
		CreateVertexBuffer(mGame->GetD3DDevice(), positionUVNormalVertices, mPlaneVertexCount, mPlanePositionUVNormalVertexBuffer.ReleaseAndGetAddressOf());

		std::wstring textureName = L"Content\\Textures\\Checkerboard.png";
		Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.c_str(), nullptr, mColorTexture.ReleaseAndGetAddressOf()), "CreateWICTextureFromFile() failed.");

		mPointLight = std::make_unique<PointLight>(*mGame);
		mPointLight->SetRadius(50.0f);
		mPointLight->SetPosition(0.0f, 5.0f, 2.0f);
		
		mProxyModel = std::make_unique<ProxyModel>(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj.bin", 0.5f);
		mProxyModel->Initialize();

		XMStoreFloat4x4(&mPlaneWorldMatrix, XMMatrixScaling(10.0f, 10.0f, 10.0f));

		mProjector = std::make_unique<Projector>(*mGame);
		mProjector->Initialize();

		mProjectorFrustum->SetMatrix(mProjector->ViewProjectionMatrix());

		mRenderableProjectorFrustum = std::make_unique<RenderableFrustum>(*mGame, *mCamera);
		mRenderableProjectorFrustum->Initialize();
		mRenderableProjectorFrustum->InitializeGeometry(*(mProjectorFrustum.get()));

		InitializeProjectedTextureScalingMatrix();

		// Vertex and index buffers for a second model to render
		std::unique_ptr<Model> model(new Model(*mGame, "Content\\Models\\teapot.obj.bin", true));

		Mesh* mesh = model->Meshes().at(0);
		
		//Create Vertex Model Buffer for DepthMap
		const std::vector<XMFLOAT3>& sourceVertices = mesh->Vertices();
        std::vector<VertexPosition> vertices;
        vertices.reserve(sourceVertices.size());
        for (UINT i = 0; i < sourceVertices.size(); i++)
        {
            XMFLOAT3 position = sourceVertices.at(i);
            vertices.push_back(VertexPosition(XMFLOAT4(position.x, position.y, position.z, 1.0f)));
        }
		CreateVertexBuffer(mGame->GetD3DDevice(), &vertices[0], static_cast<UINT>(vertices.size()), mModelPositionVertexBuffer.ReleaseAndGetAddressOf());

		//Create Vertex Model Buffer for ShadowMap
		std::vector<XMFLOAT3>* textureCoordinates = mesh->TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());
		const std::vector<XMFLOAT3>& normals = mesh->Normals();
		assert(textureCoordinates->size() == sourceVertices.size());

		std::vector<VertexPositionTextureNormal> verticesNormal;
		verticesNormal.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			verticesNormal.push_back(VertexPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal));
		}
		CreateVertexBuffer(mGame->GetD3DDevice(), &verticesNormal[0], static_cast<UINT>(verticesNormal.size()), mModelPositionUVNormalVertexBuffer.ReleaseAndGetAddressOf());

		mesh->CreateIndexBuffer(mModelIndexBuffer.ReleaseAndGetAddressOf());
		mModelIndexCount = static_cast<UINT>(mesh->Indices().size());

		XMStoreFloat4x4(&mModelWorldMatrix, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixTranslation(0.0f, 5.0f, 2.5f));

		mDepthMap = std::make_unique<DepthMap>(*mGame, DepthMapWidth, DepthMapHeight);
		mSpriteBatch = std::unique_ptr<SpriteBatch>(new SpriteBatch(mGame->GetD3DDeviceContext()));
		mSpriteFont = std::unique_ptr<SpriteFont>(new SpriteFont(mGame->GetD3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont"));

		//Create samplers
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mColorSampler.ReleaseAndGetAddressOf()));

		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		for (int i = 0; i < 4; ++i)
		{
			samplerDesc.BorderColor[i] = 1;
		}
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mShadowMapSampler.ReleaseAndGetAddressOf()));

		samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mPCFShadowMapSampler.ReleaseAndGetAddressOf()));

		
		UpdateDepthBiasState();
	}

	void ShadowMappingDemo::UpdateDepthBias(const GameTime& gameTime)
	{
		GamePadComponent* gamepad = mGame->GetGamePad();
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;
		
		if (gamepad)
		{
			if ((gamepad->CurrentState().IsRightTriggerPressed() || gamepad->CurrentState().IsLeftTriggerPressed()))
			{
				return;
			}

			if (gamepad->IsButtonHeldDown(GamePadButton::DPadUp))
			{
				mSlopeScaledDepthBias += elapsedTime;
				UpdateDepthBiasState();
			}

			if (gamepad->IsButtonHeldDown(GamePadButton::DPadDown) && mSlopeScaledDepthBias > 0)
			{
				mSlopeScaledDepthBias -= elapsedTime;
				mSlopeScaledDepthBias = XMMax(mSlopeScaledDepthBias, 0.0f);
				UpdateDepthBiasState();
			}

			if (gamepad->IsButtonHeldDown(GamePadButton::DPadRight))
			{
				mDepthBias += DepthBiasModulationRate * elapsedTime;
				UpdateDepthBiasState();
			}

			if (gamepad->IsButtonHeldDown(GamePadButton::DPadLeft) && mDepthBias > 0)
			{
				mDepthBias -= DepthBiasModulationRate * elapsedTime;
				mDepthBias = XMMax(mDepthBias, 0.0f);
				UpdateDepthBiasState();
			}
		}
	}

	void ShadowMappingDemo::UpdateDepthBiasState()
	{
		D3D11_RASTERIZER_DESC rasterizerStateDesc;
		ZeroMemory(&rasterizerStateDesc, sizeof(rasterizerStateDesc));
		rasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerStateDesc.CullMode = D3D11_CULL_BACK;
		rasterizerStateDesc.DepthClipEnable = true;
		rasterizerStateDesc.DepthBias = (int)mDepthBias;
		rasterizerStateDesc.SlopeScaledDepthBias = mSlopeScaledDepthBias;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateRasterizerState(&rasterizerStateDesc, mDepthBiasState.ReleaseAndGetAddressOf()), "ID3D11Device::CreateRasterizerState() failed.");
		
	}

	void ShadowMappingDemo::Update(const GameTime& gameTime)
	{
		GamePadComponent* gamepad = mGame->GetGamePad();
		if (gamepad)
		{
			if (gamepad->WasButtonPressedThisFrame(GamePadButton::Start))
			{
				mCurrentTechnique = static_cast<ShadowMappingTechnique>(static_cast<int>(mCurrentTechnique)+1);
				if (mCurrentTechnique == ShadowMappingTechnique::ShadowMappingTechniqueEnd)
				{
					mCurrentTechnique = ShadowMappingTechnique::ShadowMappingTechniqueSimple;
				}
			}
		}

	
		if (gamepad->WasButtonPressedThisFrame(GamePadButton::Back))
		{
			mDrawDepthMap = !mDrawDepthMap;
		}
		
		if (!Camera::CameraControlFlag)
		{
			UpdateDepthBias(gameTime);
			UpdateAmbientLight(gameTime);
			UpdatePointLightAndProjector(gameTime);
			UpdateSpecularLight(gameTime);
		}
		mProxyModel->Update(gameTime);
		mProjector->Update(gameTime);
		mRenderableProjectorFrustum->Update(gameTime);
	}

	void ShadowMappingDemo::Draw(const GameTime& gameTime)
	{
		static float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
#pragma region DepthMap
		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		direct3DDeviceContext->IASetInputLayout(mDepthMapInputLayout.Get());
		
		// Depth map pass (render the teapot model only)
		mRenderStateHelper.SaveRasterizerState();
		mDepthMap->Begin();
				
		direct3DDeviceContext->ClearDepthStencilView(mDepthMap->DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
		direct3DDeviceContext->VSSetShader(mDepthMapShader.Get(), nullptr, 0);
		direct3DDeviceContext->PSSetShader(nullptr, nullptr, 0);

		direct3DDeviceContext->RSSetState(mDepthBiasState.Get());
		
		UINT stride = sizeof(VertexPosition);
		UINT offset = 0;
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mModelPositionVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		
		XMMATRIX modelWorldMatrix = XMLoadFloat4x4(&mModelWorldMatrix);
		XMStoreFloat4x4(&mDepthMapBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(modelWorldMatrix  * mProjector->ViewMatrix() * mProjector->ProjectionMatrix()));
		
		direct3DDeviceContext->UpdateSubresource(mDepthMapCBufferPerObject.Get(), 0, nullptr, &mDepthMapBufferPerObjectData, 0, 0);
		direct3DDeviceContext->VSSetConstantBuffers(0, 1, mDepthMapCBufferPerObject.GetAddressOf());

		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);

		mDepthMap->End();
		mRenderStateHelper.RestoreRasterizerState();
#pragma endregion		

		// Projective texture mapping pass
		direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);

		ID3D11ClassInstance* instance = mClassInstances[mCurrentTechnique].Get();
		direct3DDeviceContext->PSSetShader(mPixelShader.Get(), &instance, 1);
		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		// Draw teapot model
		stride = sizeof(VertexPositionTextureNormal);
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mPlanePositionUVNormalVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mPlaneIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX planeWorldMatrix = XMLoadFloat4x4(&mPlaneWorldMatrix);
		XMMATRIX planeWVP = planeWorldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		XMMATRIX projectiveTextureMatrix = planeWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);		
		XMVECTOR shadowMapSize = XMVectorSet(static_cast<float>(DepthMapWidth), static_cast<float>(DepthMapHeight), 0.0f, 0.0f);

		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(planeWVP));
		XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(planeWorldMatrix));
		XMStoreFloat4x4(&mVSCBufferPerObjectData.ProjectiveTextureMatrix, XMMatrixTranspose(projectiveTextureMatrix));

		XMStoreFloat4(&mPSCBufferPerObjectData.SpecularColor, specularColor);
		mPSCBufferPerObjectData.SpecularPower = mSpecularPower;

		XMStoreFloat4(&mPSCBufferPerFrameData.AmbientColor, ambientColor);
		XMStoreFloat4(&mPSCBufferPerFrameData.LightColor, mPointLight->ColorVector());

		XMStoreFloat3(&mPSCBufferPerFrameData.LightPosition, mPointLight->PositionVector());
		mPSCBufferPerFrameData.LightRadius = mPointLight->Radius();
		XMStoreFloat3(&mVSCBufferPerFrameData.LightPosition, mPointLight->PositionVector());
		mVSCBufferPerFrameData.LightRadius = mPointLight->Radius();

		XMStoreFloat3(&mPSCBufferPerFrameData.CameraPosition, mCamera->PositionVector());
		XMStoreFloat2(&mPSCBufferPerFrameData.ShadowMapSize, shadowMapSize);

		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);

		ID3D11Buffer* VSConstantBuffers[] = { mVSCBufferPerObject.Get(), mVSCBufferPerFrame.Get() };
		direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

		ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
		direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

		
		static ID3D11ShaderResourceView* PSShaderResources[] = { mColorTexture.Get(), mDepthMap->OutputTexture() };
		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		static ID3D11SamplerState* samplers[] = { mColorSampler.Get(), mShadowMapSampler.Get(), mPCFShadowMapSampler.Get() };
		direct3DDeviceContext->PSSetSamplers(0, ARRAYSIZE(samplers), samplers);

		direct3DDeviceContext->Draw(mPlaneVertexCount, 0);
		
		static ID3D11ShaderResourceView* emptySRV[] = { nullptr, nullptr };

		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(emptySRV), emptySRV);

		// Draw teapot model
		direct3DDeviceContext->IASetVertexBuffers(0, 1, mModelPositionUVNormalVertexBuffer.GetAddressOf(), &stride, &offset);
		direct3DDeviceContext->IASetIndexBuffer(mModelIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX modelWVP = modelWorldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		projectiveTextureMatrix = modelWorldMatrix * mProjector->ViewMatrix() * mProjector->ProjectionMatrix() * XMLoadFloat4x4(&mProjectedTextureScalingMatrix);

		XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(modelWVP));
		XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(modelWorldMatrix));
		XMStoreFloat4x4(&mVSCBufferPerObjectData.ProjectiveTextureMatrix, XMMatrixTranspose(projectiveTextureMatrix));

		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
		direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);

		direct3DDeviceContext->DrawIndexed(mModelIndexCount, 0, 0);

		direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(emptySRV), emptySRV);

		mProxyModel->Draw(gameTime);
		mRenderableProjectorFrustum->Draw(gameTime);

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		if (mDrawDepthMap)
		{
			mSpriteBatch->Draw(mDepthMap->OutputTexture(), DepthMapDestinationRectangle);
		}

		
		std::wostringstream helpLabel;
		helpLabel << L"Ambient Intensity (+A/-B): " << mAmbientColor.r << "\n";
		helpLabel << L"Point Light Intensity (+DpadUp/-DpadDown): " << mPointLight->Color().r << "\n";
		helpLabel << L"Specular Power (+DPadRight/-DPadLeft): " << mSpecularPower << "\n";
		helpLabel << L"Move Projector/Light (XZ: Left Stick, XY: Right Stick)\n";
		helpLabel << L"Rotate Projector (Hold Trigger and Use Right Stick)\n";
		helpLabel << L"Show Shadow Map (Back): " << (mDrawDepthMap ? "Yes" : "No") << "\n";
		helpLabel << std::setprecision(5) << L"Active Technique (Space): " << ShadowMappingDisplayNames[static_cast<uint32_t>(mCurrentTechnique)].c_str() << "\n";

		if (mCurrentTechnique == ShadowMappingTechnique::ShadowMappingTechniquePCF)
		{
			if (mGame->GetGamePad())
			{

				if (mGame->GetGamePad()->CurrentState().IsRightTriggerPressed() || mGame->GetGamePad()->CurrentState().IsLeftTriggerPressed())
				{
					helpLabel << "Release Trigger to Adjust Depth Bias" << "\n";
				}
				else
				{
					helpLabel << L"Depth Bias [(+DPadUp/-DPadDown)]: " << (int)mDepthBias << "\n"
						<< L"Slope-Scaled Depth Bias [(+DPadRight/-DPadLeft)]: " << mSlopeScaledDepthBias;
				}
			}
		}
		if (Camera::CameraControlFlag)
		{
			helpLabel << L"Camera Control On" << "\n";
		}

		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}

	void ShadowMappingDemo::CreateVertexBuffer(ID3D11Device* device, VertexPosition* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPosition) *  vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		Utility::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer));
	}

	void ShadowMappingDemo::CreateVertexBuffer(ID3D11Device* device, VertexPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexPositionTextureNormal) * vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;

		Utility::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer));
	}
	void ShadowMappingDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a; 
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		if (gp->IsButtonDown(GamePadButton::A) && ambientIntensity < 1.0f)
		{
			ambientIntensity += elapsedTime;
			ambientIntensity = min(ambientIntensity, 1.0f);
			XMFLOAT4 newColor(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);

			XMVECTOR ambientColor = XMLoadFloat4(&newColor);
			XMStoreColor(&mAmbientColor, ambientColor);
		}

		if (gp->IsButtonDown(GamePadButton::B) && ambientIntensity > 0.0f)
		{
			ambientIntensity -= elapsedTime;
			ambientIntensity = max(ambientIntensity, 0.0f);

			XMFLOAT4 newColor(ambientIntensity, ambientIntensity, ambientIntensity, 1.0f);

			XMVECTOR ambientColor = XMLoadFloat4(&newColor);
			XMStoreColor(&mAmbientColor, ambientColor);
		}
	}

	void ShadowMappingDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = 1.0f;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		Library::GamePadComponent* gp = mGame->GetGamePad();

		if (!gp ||!(gp->CurrentState().IsRightTriggerPressed() || gp->CurrentState().IsLeftTriggerPressed()))
		{
			return;
		}

		// Update point light intensity		
		if (gp->IsButtonDown(GamePadButton::DPadUp) && pointLightIntensity < 1.0f)
		{
			pointLightIntensity += elapsedTime;
			pointLightIntensity = min(pointLightIntensity, 1.0f);

			mPSCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPSCBufferPerFrameData.LightColor);
		}
		if (gp->IsButtonDown(GamePadButton::DPadDown) && pointLightIntensity > 0.0f)
		{
			pointLightIntensity -= elapsedTime;
			pointLightIntensity = max(pointLightIntensity, 0.0f);

			mPSCBufferPerFrameData.LightColor = XMFLOAT4(pointLightIntensity, pointLightIntensity, pointLightIntensity, 1.0f);
			mPointLight->SetColor(mPSCBufferPerFrameData.LightColor);
		}

		static float specularPower = mSpecularPower;

		if (gp->IsButtonDown(GamePadButton::DPadRight) && specularPower < UCHAR_MAX)
		{
			specularPower += LightModulationRate * elapsedTime;
			specularPower = min(specularPower, static_cast<float>(UCHAR_MAX));

			mSpecularPower = specularPower;
		}

		if (gp->IsButtonDown(GamePadButton::DPadLeft) && specularPower > 1.0f)
		{
			specularPower -= LightModulationRate * elapsedTime;
			specularPower = max(specularPower, 1.0f);

			mSpecularPower = specularPower;
		}
	}

	void ShadowMappingDemo::UpdatePointLightAndProjector(const GameTime& gameTime)
	{
		static float pointLightIntensity = mPointLight->Color().a;
		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;

		// Update point light intensity		

		// Move point light and projector		
		XMFLOAT3 movementAmount = Vector3Helper::Zero;

		// Rotate projector
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;

		GamePadComponent* gp = mGame->GetGamePad();
		if (gp)
		{
			if (!(gp->CurrentState().IsRightTriggerPressed() || gp->CurrentState().IsLeftTriggerPressed()))
			{
				movementAmount.x = abs(gp->CurrentState().thumbSticks.leftX) > abs(gp->CurrentState().thumbSticks.rightX) ? gp->CurrentState().thumbSticks.leftX : gp->CurrentState().thumbSticks.rightX;
				movementAmount.y = gp->CurrentState().thumbSticks.rightY;
				movementAmount.z = -gp->CurrentState().thumbSticks.leftY;
			}
			else
			{
				rotationAmount.x = gp->CurrentState().thumbSticks.rightX * elapsedTime;
				rotationAmount.y = -gp->CurrentState().thumbSticks.rightY * elapsedTime;
			}
		}

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
		mProjector->SetPosition(mPointLight->Position());
		mRenderableProjectorFrustum->SetPosition(mPointLight->Position());

		

		XMMATRIX projectorRotationMatrix = XMMatrixIdentity();
		if (rotationAmount.x != 0)
		{
			projectorRotationMatrix = XMMatrixRotationY(rotationAmount.x);
		}

		if (rotationAmount.y != 0)
		{
			XMMATRIX projectorRotationAxisMatrix = XMMatrixRotationAxis(mProjector->RightVector(), rotationAmount.y);
			projectorRotationMatrix *= projectorRotationAxisMatrix;
		}

		if (rotationAmount.x != Vector2Helper::Zero.x || rotationAmount.y != Vector2Helper::Zero.y)
		{
			mProjector->ApplyRotation(projectorRotationMatrix);
			mRenderableProjectorFrustum->ApplyRotation(projectorRotationMatrix);
		}
	}
	void ShadowMappingDemo::InitializeProjectedTextureScalingMatrix()
	{
		mProjectedTextureScalingMatrix._11 = 0.5f;
		mProjectedTextureScalingMatrix._22 = -0.5f;
		mProjectedTextureScalingMatrix._33 = 1.0f;
		mProjectedTextureScalingMatrix._41 = 0.5f;
		mProjectedTextureScalingMatrix._42 = 0.5f;
		mProjectedTextureScalingMatrix._44 = 1.0f;
	}
}
