#include "pch.h"
#include "Camera.h"
#include "PointLight.h"
#include "RenderStateHelper.h"
#include "VectorHelper.h"
#include "ColorHelper.h"
#include "AnimationPlayer.h"
#include "AnimationClip.h"
#include "ProxyModel.h"
#include "Bone.h"
#include <WICTextureLoader.h>
#include <SpriteBatch.h>
#include <SpriteFont.h>
#include <sstream>
#include <iomanip>
#include "AnimationDemo.h"
#include "GamePadComponent.h"

using namespace DirectX;
using namespace Library;

namespace Rendering
{
	RTTI_DEFINITIONS(AnimationDemo)
	AnimationDemo::AnimationDemo(Game::GameBase& game, Camera& camera)
	: DrawableGameComponent(game, camera), mWorldMatrix(MatrixHelper::Identity),
	mVertexBuffers(), mIndexBuffers(), mIndexCounts(), mColorTextures(), mAmbientColor(reinterpret_cast<const float*>(&ColorHelper::White)), mPointLight(nullptr),
	mSpecularColor(1.0f, 1.0f, 1.0f, 1.0f), mSpecularPower(25.0f), mSkinnedModel(nullptr), mAnimationPlayer(nullptr),
	mRenderStateHelper(game), mProxyModel(nullptr), mSpriteBatch(nullptr), mSpriteFont(nullptr), mTextPosition(0.0f, 40.0f), mManualAdvanceMode(true)
	{
	}

	const float AnimationDemo::LightModulationRate = UCHAR_MAX;
	const float AnimationDemo::LightMovementRate = 10.0f;

	void AnimationDemo::Initialize()
	{

		//Load shaders
		//Load ShadowMap shaders
		std::string compiledVertexShader = Utility::LoadBinaryFile("Content\\Shaders\\SkinnedModelVertexShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateVertexShader(&compiledVertexShader[0], compiledVertexShader.size(), nullptr, mVertexShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");
		
		//Shadow Map Materials
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptions[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateInputLayout(inputElementDescriptions, ARRAYSIZE(inputElementDescriptions), &compiledVertexShader[0], compiledVertexShader.size(), mInputLayout.ReleaseAndGetAddressOf()), "ID3D11Device::CreateInputLayout() failed.");
		
		std::string compiledPixelShader = Utility::LoadBinaryFile("Content\\Shaders\\SkinnedModelPixelShader.cso");
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreatePixelShader(&compiledPixelShader[0], compiledPixelShader.size(), nullptr, mPixelShader.ReleaseAndGetAddressOf()), "ID3D11Device::CreatedVertexShader() failed.");

		// Create a constant buffers
		D3D11_BUFFER_DESC constantBufferDesc;
		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(VSCBufferSkinning);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferSkinning.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerObject);
		constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateBuffer(&constantBufferDesc, nullptr, mVSCBufferPerObject.ReleaseAndGetAddressOf()), "ID3D11Device::CreateBuffer() failed.");

		ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
		constantBufferDesc.ByteWidth = sizeof(VSCBufferPerFrame);
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

		// Load the model
		mSkinnedModel = std::make_unique<Model>(*mGame, "Content\\Models\\NoTPoseRunning.dae.bin", true);
		//load index and vertex Buffers.
		mVertexBuffers.resize(mSkinnedModel->Meshes().size());
		mIndexBuffers.resize(mSkinnedModel->Meshes().size());
		mIndexCounts.resize(mSkinnedModel->Meshes().size());
		mColorTextures.resize(mSkinnedModel->Meshes().size());
		for (UINT i = 0; i < mSkinnedModel->Meshes().size(); i++)
		{
			Mesh* mesh = mSkinnedModel->Meshes().at(i);

			Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
			CreateVertexBuffer(mGame->GetD3DDevice(), *mesh, vertexBuffer.ReleaseAndGetAddressOf());
			mVertexBuffers[i] = vertexBuffer;

			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;
			mesh->CreateIndexBuffer(indexBuffer.ReleaseAndGetAddressOf());
			mIndexBuffers[i] = indexBuffer;

			mIndexCounts[i] = static_cast<uint32_t>(mesh->Indices().size());

			mColorTextures[i] = nullptr;
			ModelMaterial* material = mesh->GetMaterial();
			
			auto it = std::find(material->TextureTypes().begin(), material->TextureTypes().end(), TextureTypeDifffuse);
			if (material != nullptr && it != material->TextureTypes().end())
			{
				std::wostringstream textureName;
				textureName << L"Content\\Textures\\Soldier.png";
				Utility::ThrowIfFailed(DirectX::CreateWICTextureFromFile(mGame->GetD3DDevice(), mGame->GetD3DDeviceContext(), textureName.str().c_str(), nullptr, mColorTextures[i].ReleaseAndGetAddressOf()));
			}
		}

		//Create samplers
		D3D11_SAMPLER_DESC samplerDesc;
		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		Utility::ThrowIfFailed(mGame->GetD3DDevice()->CreateSamplerState(&samplerDesc, mColorSampler.ReleaseAndGetAddressOf()));

		XMStoreFloat4x4(&mWorldMatrix, XMMatrixScaling(0.05f, 0.05f, 0.05f));

		mPointLight = std::unique_ptr<PointLight>(new PointLight(*mGame));
		mPointLight->SetRadius(500.0f);
		mPointLight->SetPosition(5.0f, 0.0f, 10.0f);
		
		mAnimationPlayer = std::make_unique<AnimationPlayer>(*mGame, *mSkinnedModel, false);
		mAnimationPlayer->StartClip(*(mSkinnedModel->Animations().at(0)));

		mProxyModel = std::unique_ptr<ProxyModel>(new ProxyModel(*mGame, *mCamera, "Content\\Models\\PointLightProxy.obj.bin", 0.5f));
		mProxyModel->Initialize();

		mSpriteBatch = std::unique_ptr<SpriteBatch>(new SpriteBatch(mGame->GetD3DDeviceContext()));
		mSpriteFont = std::unique_ptr<SpriteFont>(new SpriteFont(mGame->GetD3DDevice(), L"Content\\Fonts\\Arial_14_Regular.spritefont"));
	}

	void AnimationDemo::Update(const GameTime& gameTime)
	{
		UpdateOptions();
		UpdateAmbientLight(gameTime);
		UpdatePointLight(gameTime);
		UpdateSpecularLight(gameTime);

		if (mManualAdvanceMode == false)
		{
			mAnimationPlayer->Update(gameTime);
		}

		mProxyModel->Update(gameTime);
	}

	void AnimationDemo::Draw(const GameTime& gameTime)
	{
		ID3D11DeviceContext* direct3DDeviceContext = mGame->GetD3DDeviceContext();
		direct3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		direct3DDeviceContext->IASetInputLayout(mInputLayout.Get());

		XMMATRIX worldMatrix = XMLoadFloat4x4(&mWorldMatrix);
		XMMATRIX wvp = worldMatrix * mCamera->ViewMatrix() * mCamera->ProjectionMatrix();
		XMVECTOR ambientColor = XMLoadColor(&mAmbientColor);
		XMVECTOR specularColor = XMLoadColor(&mSpecularColor);

		UINT stride = sizeof(VertexSkinnedPositionTextureNormal);
		UINT offset = 0;

		for (UINT i = 0; i < mVertexBuffers.size(); i++)
		{
			ID3D11Buffer* vertexBuffer = mVertexBuffers[i].Get();
			ID3D11Buffer* indexBuffer = mIndexBuffers[i].Get();
			UINT indexCount = mIndexCounts[i];
			ID3D11ShaderResourceView* colorTexture = mColorTextures[i].Get();

			direct3DDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
			direct3DDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			direct3DDeviceContext->VSSetShader(mVertexShader.Get(), nullptr, 0);
			direct3DDeviceContext->PSSetShader(mPixelShader.Get(), nullptr, 0);

			XMStoreFloat4x4(&mVSCBufferPerObjectData.WorldViewProjection, XMMatrixTranspose(wvp));
			
			XMStoreFloat4x4(&mVSCBufferPerObjectData.World, XMMatrixTranspose(worldMatrix));
			
			XMStoreFloat3(&mPSCBufferPerObjectData.SpecularColor, specularColor);
			mPSCBufferPerObjectData.SpecularPower = mSpecularPower;

			XMStoreFloat4(&mPSCBufferPerFrameData.AmbientColor, ambientColor);
			XMStoreFloat4(&mPSCBufferPerFrameData.LightColor, mPointLight->ColorVector());

			XMStoreFloat3(&mPSCBufferPerFrameData.LightPosition, mPointLight->PositionVector());
			mPSCBufferPerFrameData.LightRadius = mPointLight->Radius();
			XMStoreFloat3(&mVSCBufferPerFrameData.LightPosition, mPointLight->PositionVector());
			mVSCBufferPerFrameData.LightRadius = mPointLight->Radius();

			XMStoreFloat3(&mPSCBufferPerFrameData.CameraPosition, mCamera->PositionVector());

			uint32_t limit = MAXBONES > static_cast<uint32_t>(mAnimationPlayer->BoneTransforms().size()) ? static_cast<uint32_t>(mAnimationPlayer->BoneTransforms().size()) : MAXBONES;
			
			for (uint32_t j = 0; j < limit; ++j)
			{
				XMMATRIX boneMatrix = XMLoadFloat4x4(&mAnimationPlayer->BoneTransforms()[j]);
				XMStoreFloat4x4(&mVSCBufferSkinningData.BoneTransforms[j], XMMatrixTranspose(boneMatrix));
			}
			
			direct3DDeviceContext->UpdateSubresource(mVSCBufferPerFrame.Get(), 0, nullptr, &mVSCBufferPerFrameData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mVSCBufferPerObject.Get(), 0, nullptr, &mVSCBufferPerObjectData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mVSCBufferSkinning.Get(), 0, nullptr, &mVSCBufferSkinningData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mPSCBufferPerFrame.Get(), 0, nullptr, &mPSCBufferPerFrameData, 0, 0);
			direct3DDeviceContext->UpdateSubresource(mPSCBufferPerObject.Get(), 0, nullptr, &mPSCBufferPerObjectData, 0, 0);

			ID3D11Buffer* VSConstantBuffers[] = { mVSCBufferPerFrame.Get(), mVSCBufferPerObject.Get(), mVSCBufferSkinning.Get() };
			direct3DDeviceContext->VSSetConstantBuffers(0, ARRAYSIZE(VSConstantBuffers), VSConstantBuffers);

			ID3D11Buffer* PSConstantBuffers[] = { mPSCBufferPerFrame.Get(), mPSCBufferPerObject.Get() };
			direct3DDeviceContext->PSSetConstantBuffers(0, ARRAYSIZE(PSConstantBuffers), PSConstantBuffers);

			ID3D11ShaderResourceView* PSShaderResources[] = { mColorTextures[i].Get() };
			direct3DDeviceContext->PSSetShaderResources(0, ARRAYSIZE(PSShaderResources), PSShaderResources);
			static ID3D11SamplerState* samplers[] = { mColorSampler.Get() };
			direct3DDeviceContext->PSSetSamplers(0, ARRAYSIZE(samplers), samplers);

			direct3DDeviceContext->DrawIndexed(indexCount, 0, 0);
		}

		mProxyModel->Draw(gameTime);

		mRenderStateHelper.SaveAll();
		mSpriteBatch->Begin();

		std::wostringstream helpLabel;
		helpLabel << std::setprecision(5);
		helpLabel << L"Ambient Intensity (+DPadRight/-DPadLeft):" << mAmbientColor.a << "\n";
		helpLabel << L"Point Light Intensity (Hold Trigger and (+A/-B)): " << mPointLight->Color().a << "\n";
		helpLabel << L"Specular Power (+DpadUp/-DpadDown): " << mSpecularPower << "\n";
		helpLabel << L"Move Projector/Light (XZ: Left Stick, XY: Right Stick)\n";
		helpLabel << "Frame Advance Mode (B): " << (mManualAdvanceMode ? "Manual" : "Auto") << "\nAnimation Time: " << mAnimationPlayer->CurrentTime()
			<< "\nFrame Interpolation (X): " << (mAnimationPlayer->InterpolationEnabled() ? "On" : "Off") << "\nGo to Bind Pose (Y)";

		if (mManualAdvanceMode)
		{
			helpLabel << "\nCurrent Keyframe (A): " << mAnimationPlayer->CurrentKeyframe();
		}
		else
		{
			helpLabel << "\nPause / Resume(Start)";
		}
		if (Camera::CameraControlFlag)
		{
			helpLabel << L"\n[Camera Control On]" << "\n";
		}


		mSpriteFont->DrawString(mSpriteBatch.get(), helpLabel.str().c_str(), mTextPosition);

		mSpriteBatch->End();
		mRenderStateHelper.RestoreAll();
	}

	void AnimationDemo::UpdateOptions()
	{
		GamePadComponent* gamepad = mGame->GetGamePad();

		if (gamepad && !((gamepad->CurrentState().IsRightTriggerPressed() || gamepad->CurrentState().IsLeftTriggerPressed())))
		{
			if (gamepad->WasButtonReleasedThisFrame(GamePadButton::Start))
			{
				if (mAnimationPlayer->IsPlayingClip())
				{
					mAnimationPlayer->PauseClip();
				}
				else
				{
					mAnimationPlayer->ResumeClip();
				}				
			}

			if (gamepad->WasButtonReleasedThisFrame(GamePadButton::B))
			{
				// Enable/disable manual advance mode
				mManualAdvanceMode = !mManualAdvanceMode;
				mAnimationPlayer->SetCurrentKeyFrame(0);
			}

			if (gamepad->WasButtonReleasedThisFrame(GamePadButton::X))
			{
				// Enable/disabled interpolation
				mAnimationPlayer->SetInterpolationEnabled(!mAnimationPlayer->InterpolationEnabled());
			}


			if (gamepad->WasButtonReleasedThisFrame(GamePadButton::Y))
			{
				// Reset the animation clip to the bind pose
				mAnimationPlayer->StartClip(*(mSkinnedModel->Animations().at(0)));
			}

			if (mManualAdvanceMode && gamepad->WasButtonReleasedThisFrame(GamePadButton::A))
			{
				// Advance the current keyframe
				UINT currentKeyFrame = mAnimationPlayer->CurrentKeyframe();
				currentKeyFrame++;
				if (currentKeyFrame >= mAnimationPlayer->CurrentClip()->KeyframeCount())
				{
					currentKeyFrame = 0;
				}

				mAnimationPlayer->SetCurrentKeyFrame(currentKeyFrame);
			}
		}
	}

	void AnimationDemo::UpdateAmbientLight(const GameTime& gameTime)
	{
		static float ambientIntensity = mAmbientColor.a; 
		float elapsedTime = (float)gameTime.ElapsedGameTime().count() / 1000.0f;

		GamePadComponent* gamepad = mGame->GetGamePad();
		// Update point light intensity		

		if (gamepad != nullptr)
		{
			if (gamepad->IsButtonDown(GamePadButton::DPadRight) && ambientIntensity < UCHAR_MAX)
			{
				ambientIntensity += LightModulationRate * elapsedTime;
				mAmbientColor.a = (UCHAR)XMMin<float>(ambientIntensity, UCHAR_MAX);
			}

			if (gamepad->IsButtonDown(GamePadButton::DPadLeft) && ambientIntensity > 0)
			{
				ambientIntensity -= LightModulationRate *elapsedTime;
				mAmbientColor.a = (UCHAR)XMMax<float>(ambientIntensity, 0);

			}
		}
	}

	void AnimationDemo::UpdateSpecularLight(const GameTime& gameTime)
	{
		static float specularIntensity = mSpecularPower;
		float elapsedTime = (float)gameTime.ElapsedGameTime().count() / 1000.0f;

		GamePadComponent* gamepad = mGame->GetGamePad();
		// Update point light intensity		
		if (gamepad != nullptr)
		{
			if (gamepad->IsButtonDown(GamePadButton::DPadUp) && specularIntensity < UCHAR_MAX)
			{
				specularIntensity += LightModulationRate * elapsedTime;
				specularIntensity = XMMin<float>(specularIntensity, UCHAR_MAX);

				mSpecularPower = specularIntensity;;
			}

			if (gamepad->IsButtonDown(GamePadButton::DPadDown) && specularIntensity > 0)
			{
				specularIntensity -= LightModulationRate * elapsedTime;
				specularIntensity = XMMax<float>(specularIntensity, 0);

				mSpecularPower = specularIntensity;
			}
		}
	}

	void AnimationDemo::UpdatePointLight(const GameTime& gameTime)
	{
		static float pointLightIntensity = mPointLight->Color().a;
		float elapsedTime = (float)gameTime.ElapsedGameTime().count() / 1000.0f;

		GamePadComponent* gamepad = mGame->GetGamePad();
		// Update point light intensity		
		if (gamepad && ((gamepad->CurrentState().IsRightTriggerPressed() || gamepad->CurrentState().IsLeftTriggerPressed())))
		{
			if (gamepad->IsButtonDown(GamePadButton::A) && pointLightIntensity < UCHAR_MAX)
			{
				pointLightIntensity += LightModulationRate * elapsedTime;

				XMCOLOR pointLightLightColor = mPointLight->Color();
				pointLightLightColor.a = (UCHAR)XMMin<float>(pointLightIntensity, UCHAR_MAX);
				mPointLight->SetColor(pointLightLightColor);
			}
			if (gamepad->IsButtonDown(GamePadButton::B) && pointLightIntensity > 0)
			{
				pointLightIntensity -= LightModulationRate * elapsedTime;

				XMCOLOR pointLightLightColor = mPointLight->Color();
				pointLightLightColor.a = (UCHAR)XMMax<float>(pointLightIntensity, 0.0f);
				mPointLight->SetColor(pointLightLightColor);
			}
		}

		// Move point light
		XMFLOAT3 movementAmount = Vector3Helper::Zero;

		
		GamePadComponent* gp = mGame->GetGamePad();
		if (gp)
		{
			if (!mCamera->CameraControlFlag)
			{
				movementAmount.x = abs(gp->CurrentState().thumbSticks.leftX) > abs(gp->CurrentState().thumbSticks.rightX) ? gp->CurrentState().thumbSticks.leftX : gp->CurrentState().thumbSticks.rightX;
				movementAmount.y = gp->CurrentState().thumbSticks.rightY;
				movementAmount.z = -gp->CurrentState().thumbSticks.leftY;
			}
		}

		XMVECTOR movement = XMLoadFloat3(&movementAmount) * LightMovementRate * elapsedTime;
		mPointLight->SetPosition(mPointLight->PositionVector() + movement);
		mProxyModel->SetPosition(mPointLight->Position());
	}

	void AnimationDemo::CreateVertexBuffer(ID3D11Device* device, const Mesh& mesh, ID3D11Buffer** vertexBuffer) const
	{
		const std::vector<XMFLOAT3>& sourceVertices = mesh.Vertices();
		std::vector<XMFLOAT3>* textureCoordinates = mesh.TextureCoordinates().at(0);
		assert(textureCoordinates->size() == sourceVertices.size());
		const std::vector<XMFLOAT3>& normals = mesh.Normals();
		assert(normals.size() == sourceVertices.size());
		const std::vector<BoneVertexWeights>& boneWeights = mesh.BoneWeights();
		assert(boneWeights.size() == sourceVertices.size());

		std::vector<VertexSkinnedPositionTextureNormal> vertices;
		vertices.reserve(sourceVertices.size());
		for (UINT i = 0; i < sourceVertices.size(); i++)
		{
			XMFLOAT3 position = sourceVertices.at(i);
			XMFLOAT3 uv = textureCoordinates->at(i);
			XMFLOAT3 normal = normals.at(i);
			BoneVertexWeights vertexWeights = boneWeights.at(i);

			float weights[BoneVertexWeights::MaxBoneWeightsPerVertex];
			UINT indices[BoneVertexWeights::MaxBoneWeightsPerVertex];
			ZeroMemory(weights, sizeof(float) * ARRAYSIZE(weights));
			ZeroMemory(indices, sizeof(UINT) * ARRAYSIZE(indices));
			for (UINT i = 0; i < vertexWeights.Weights().size(); i++)
			{
				BoneVertexWeights::VertexWeight vertexWeight = vertexWeights.Weights().at(i);
				weights[i] = vertexWeight.Weight;
				indices[i] = vertexWeight.BoneIndex;
			}

			vertices.push_back(VertexSkinnedPositionTextureNormal(XMFLOAT4(position.x, position.y, position.z, 1.0f), XMFLOAT2(uv.x, uv.y), normal, XMUINT4(indices), XMFLOAT4(weights)));
		}

		CreateVertexBuffer(device, &vertices[0], static_cast<UINT>(vertices.size()), vertexBuffer);
	}

	void AnimationDemo::CreateVertexBuffer(ID3D11Device* device, VertexSkinnedPositionTextureNormal* vertices, UINT vertexCount, ID3D11Buffer** vertexBuffer) const
	{
		D3D11_BUFFER_DESC vertexBufferDesc;
		ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
		vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(VertexSkinnedPositionTextureNormal) * vertexCount);
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexSubResourceData;
		ZeroMemory(&vertexSubResourceData, sizeof(vertexSubResourceData));
		vertexSubResourceData.pSysMem = vertices;
		Utility::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexSubResourceData, vertexBuffer));;
	}
}