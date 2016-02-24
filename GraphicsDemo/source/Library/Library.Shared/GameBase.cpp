#include "pch.h"
#include "DrawableGameComponent.h"
#include "GameBase.h"

using namespace Library;
using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace Game
{
	GameBase::GameBase()
		: f_getSize(nullptr),
		m_screenViewport(),
		m_outputSize(),
		m_logicalSize(),
		m_d3dFeatureLevel(D3D_FEATURE_LEVEL_9_1),
		m_d3dRenderTargetSize(), mComponents(),
		mGameTime()
	{
		CreateDeviceIndependentResources();
		CreateDevice();
	}


	GameBase::~GameBase()
	{
	}

	void GameBase::Initialize(void* window, std::function<void(Game::Size&)> getSizeCallBack)
	{
		m_window = window;
		m_d2dContext->SetDpi(96, 96);
		f_getSize = getSizeCallBack;
		CreateWindowSizeDependentResources();
	}

	void GameBase::Draw(const Library::GameTime& gameTime)
	{
		for (auto& component : mComponents)
		{
			DrawableGameComponent* drawableGameComponent = component->As<DrawableGameComponent>();
			if (drawableGameComponent != nullptr && drawableGameComponent->Visible())
			{
				drawableGameComponent->Draw(gameTime);
			}
		}
	}

	void GameBase::CreateDevice()
	{
		// This flag adds support for surfaces with a different color channel ordering
		// than the API default. It is required for compatibility with Direct2D.
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		// This array defines the set of DirectX hardware feature levels this app will support.
		// Note the ordering should be preserved.
		// Don't forget to declare your application's minimum required feature level in its
		// description.  All applications are assumed to support 9.1 unless otherwise stated.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};

		// Create the Direct3D 11 API device object and a corresponding context.
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

		HRESULT hr = D3D11CreateDevice(
			nullptr,					// Specify nullptr to use the default adapter.
			D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
			0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
			creationFlags,				// Set debug and Direct2D compatibility flags.
			featureLevels,				// List of feature levels this app can support.
			ARRAYSIZE(featureLevels),	// Size of the list above.
			D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Windows Store apps.
			&device,					// Returns the Direct3D device created.
			&m_d3dFeatureLevel,			// Returns feature level of device created.
			&context					// Returns the device immediate context.
			);

		if (FAILED(hr))
		{
			// If the initialization fails, fall back to the WARP device.
			// For more information on WARP, see: 
			// http://go.microsoft.com/fwlink/?LinkId=286690
			Utility::ThrowIfFailed(
				D3D11CreateDevice(
				nullptr,
				D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
				0,
				creationFlags,
				featureLevels,
				ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION,
				&device,
				&m_d3dFeatureLevel,
				&context
				)
				);
		}

		// Store pointers to the Direct3D 11.1 API device and immediate context.
		Utility::ThrowIfFailed(
			device.As(&m_d3dDevice)
			);

		Utility::ThrowIfFailed(
			context.As(&m_d3dContext)
			);

		// Create the Direct2D device object and a corresponding context.
		ComPtr<IDXGIDevice3> dxgiDevice;
		Utility::ThrowIfFailed(
			m_d3dDevice.As(&dxgiDevice)
			);

		Utility::ThrowIfFailed(
			m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice)
			);

		Utility::ThrowIfFailed(
			m_d2dDevice->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&m_d2dContext
			)
			);
	
	}

	void GameBase::CreateDeviceIndependentResources()
	{	
		// Initialize Direct2D resources.
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
		// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

		// Initialize the Direct2D Factory.
		Utility::ThrowIfFailed(
			D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory2),
			&options,
			&m_d2dFactory
			)
			);

		// Initialize the DirectWrite Factory.
		Utility::ThrowIfFailed(
			DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory2),
			&m_dwriteFactory
			)
			);

		// Initialize the Windows Imaging Component (WIC) Factory.
		Utility::ThrowIfFailed(
			CoCreateInstance(
			CLSID_WICImagingFactory2,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&m_wicFactory)
			)
			);
	}

	void GameBase::Begin()
	{
		RenderTarget::Begin(m_d3dContext.Get(), 1, m_d3dRenderTargetView.GetAddressOf(), m_d3dDepthStencilView.Get(), m_screenViewport);
	}

	void GameBase::End()
	{
		RenderTarget::End(GetD3DDeviceContext());
	}

	void GameBase::Clear()
	{
		// Clear the views
		
		RenderTarget::Clear(m_d3dContext.Get());
		RenderTarget::RebindCurrentRenderTargets(m_d3dContext.Get());
	}

	void GameBase::OnDeviceLost()
	{

	}

	void GameBase::OnDeviceRestored()
	{

	}
}