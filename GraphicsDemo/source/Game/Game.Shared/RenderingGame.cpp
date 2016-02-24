#include "pch.h"

#include "RenderTarget.h"
#include "FirstPersonCamera.h"
#include "Skybox.h"

#include "Demos\TexturedModelDemo.h"
#include "Demos\BlinnPhongDemo.h"
#include "Demos\PointLightDemo.h"
#include "Demos\EnvironmentMappingDemo.h"
#include "Demos\FogDemo.h"
#include "Demos\TransparencyDemo.h"
#include "Demos\ColorFilteringDemo.h"

#include "RenderingGame.h"

using namespace Library;
using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace Game
{
	RenderingGame::RenderingGame() :
		mGamePad(nullptr)
	{

	}


	RenderingGame::~RenderingGame()
	{

	}

	void RenderingGame::Initialize(void* window, std::function<void(Game::Size&)> getSizeCallBack)
	{
		GameBase::Initialize(window, getSizeCallBack);
		
		mGamePad = std::make_unique<GamePadComponent>(*this, 0);
		mSkybox = std::unique_ptr<Skybox>(new Skybox(*this, *mCamera, L"Content\\Textures\\Maskonaive2_1024.dds", 500.0f));
		mTerrain = std::unique_ptr<Terrain>(new Terrain(*this, *mCamera, L"Content\\Textures\\colorm01.bmp", L"Content\\Textures\\Heightmap.jpg"));

		mComponents.push_back(mGamePad.get());
		mComponents.push_back(mSkybox.get());
		mComponents.push_back(mTerrain.get());

		RasterizerStates::Initialize(GetD3DDevice());
		BlendStates::Initialize(GetD3DDevice());

		
		for (auto& component : mComponents)
		{
			component->Initialize();
		}
		mComponents.push_back(mCamera.get());
		mCamera->SetPosition(0.0f, 0.0f, 25.0f);

	}

	void RenderingGame::Run()
	{
		Update();

		if (Render())
		{
			HRESULT hr = m_swapChain->Present(0, 0);
			
			// Discard the contents of the render target.
			// This is a valid operation only when the existing contents will be entirely
			// overwritten. If dirty or scroll rects are used, this call should be removed.
			m_d3dContext->DiscardView(m_d3dRenderTargetView.Get());

			// Discard the contents of the depth stencil.
			m_d3dContext->DiscardView(m_d3dDepthStencilView.Get());

			// If the device was reset we must completely reinitialize the renderer.
			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				OnDeviceLost();
			}
			else
			{
				Utility::ThrowIfFailed(hr);
			}
		}
	}
	void RenderingGame::Update()
	{
		mGameTime.Update();
		mDemoSystem->Update(mGameTime);
		for (auto& component : mComponents)
		{
			if (component->Enabled())
			{
				component->Update(mGameTime);
			}
		}
				
	}

	GamePadComponent* RenderingGame::GetGamePad()
	{
		return mGamePad.get();
	}

	bool RenderingGame::Render()
	{
		mDemoSystem->Draw(mGameTime);

		return true;
	}

	void RenderingGame::CreateWindowSizeDependentResources()
	{
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
		// Windows Phone does not support resizing the swap chain, so clear it instead of resizing.
		m_swapChain = nullptr;
#endif
		ID3D11RenderTargetView* nullViews[] = { nullptr };
		m_d3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
		m_d3dRenderTargetView = nullptr;
		m_d2dContext->SetTarget(nullptr);
		m_d2dTargetBitmap = nullptr;
		m_d3dDepthStencilView = nullptr;
		m_d3dContext->Flush();

		f_getSize(m_d3dRenderTargetSize);


		if (m_swapChain != nullptr)
		{
			// If the swap chain already exists, resize it.
			HRESULT hr = m_swapChain->ResizeBuffers(
				2, // Double-buffered swap chain.
				lround(m_d3dRenderTargetSize.Width),
				lround(m_d3dRenderTargetSize.Height),
				DXGI_FORMAT_B8G8R8A8_UNORM,
				0
				);

			if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
			{
				// If the device was removed for any reason, a new device and swap chain will need to be created.
				OnDeviceLost();

				// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
				// and correctly set up the new device.
				return;
			}
			else
			{
				Utility::ThrowIfFailed(hr);
			}
		}
		else
		{
			// Otherwise, create a new one using the same adapter as the existing Direct3D device.
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

			swapChainDesc.Width = lround(m_d3dRenderTargetSize.Width); // Match the size of the window.
			swapChainDesc.Height = lround(m_d3dRenderTargetSize.Height);
			swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
			swapChainDesc.Stereo = false;
			swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = 2; // Use double-buffering to minimize latency.
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // All Windows Store apps must use this SwapEffect.
			swapChainDesc.Flags = 0;
			swapChainDesc.Scaling = DXGI_SCALING_NONE;
			swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

			// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
			ComPtr<IDXGIDevice3> dxgiDevice;
			Utility::ThrowIfFailed(
				m_d3dDevice.As(&dxgiDevice)
				);

			ComPtr<IDXGIAdapter> dxgiAdapter;
			Utility::ThrowIfFailed(
				dxgiDevice->GetAdapter(&dxgiAdapter)
				);

			ComPtr<IDXGIFactory2> dxgiFactory;
			Utility::ThrowIfFailed(
				dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))
				);

#if (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
			IUnknown* thing = static_cast<IUnknown*>(m_window);

			Utility::ThrowIfFailed(
				dxgiFactory->CreateSwapChainForCoreWindow(
				m_d3dDevice.Get(),
				thing,
				&swapChainDesc,
				nullptr,
				&m_swapChain
				)
				);
#else

			DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = { 0 };
			fsSwapChainDesc.Windowed = TRUE;

			Utility::ThrowIfFailed(dxgiFactory->CreateSwapChainForHwnd(
				m_d3dDevice.Get(), reinterpret_cast<HWND> (m_window), &swapChainDesc,
				&fsSwapChainDesc,
				nullptr, m_swapChain.ReleaseAndGetAddressOf()));
#endif

			// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
			// ensures that the application will only render after each VSync, minimizing power consumption.
			Utility::ThrowIfFailed(
				dxgiDevice->SetMaximumFrameLatency(1)
				);
		}

		// Create a render target view of the swap chain back buffer.
		ComPtr<ID3D11Texture2D> backBuffer;
		Utility::ThrowIfFailed(
			m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer))
			);

		Utility::ThrowIfFailed(
			m_d3dDevice->CreateRenderTargetView(
			backBuffer.Get(),
			nullptr,
			&m_d3dRenderTargetView
			)
			);

		// Create a depth stencil view for use with 3D rendering if needed.
		CD3D11_TEXTURE2D_DESC depthStencilDesc(
			DXGI_FORMAT_D24_UNORM_S8_UINT,
			lround(m_d3dRenderTargetSize.Width),
			lround(m_d3dRenderTargetSize.Height),
			1, // This depth stencil view has only one texture.
			1, // Use a single mipmap level.
			D3D11_BIND_DEPTH_STENCIL
			);

		ComPtr<ID3D11Texture2D> depthStencil;
		Utility::ThrowIfFailed(
			m_d3dDevice->CreateTexture2D(
			&depthStencilDesc,
			nullptr,
			depthStencil.GetAddressOf()
			)
			);

		CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
		Utility::ThrowIfFailed(
			m_d3dDevice->CreateDepthStencilView(
			depthStencil.Get(),
			&depthStencilViewDesc,
			&m_d3dDepthStencilView
			)
			);

		// Set the 3D rendering viewport to target the entire window.
		m_screenViewport = CD3D11_VIEWPORT(
			0.0f,
			0.0f,
			m_d3dRenderTargetSize.Width,
			m_d3dRenderTargetSize.Height
			);

		m_d3dContext->RSSetViewports(1, &m_screenViewport);

		mCamera = std::unique_ptr<Camera>(new FirstPersonCamera(*this));
		mCamera->Initialize();
		
		mDemoSystem = std::unique_ptr<DemoSystem>(new DemoSystem(*this, *mCamera));
		mDemoSystem->Initialize();


		Begin();
	}
}
