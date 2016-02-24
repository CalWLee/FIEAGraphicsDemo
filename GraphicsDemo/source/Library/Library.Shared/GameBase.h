#pragma once
#include "GameTime.h"
#include <functional>
#include "RenderTarget.h"
namespace Library
{
	class GamePadComponent;
	class GameComponent;
}
namespace Game
{
	// Provides an interface for an application that owns DeviceResources to be notified of the device being lost or created.
	interface IDeviceNotify
	{
		virtual void OnDeviceLost() = 0;
		virtual void OnDeviceRestored() = 0;
	};

	struct Size
	{
		float Width;
		float Height;

		Size()
			: Width(0), Height(0)
		{

		}

		Size(float width, float height)
			: Width(width), Height(height)
		{
		}

		bool operator==(const Size& rhs)
		{
			return rhs.Height == Height && rhs.Width == Width;
		}

		bool operator!=(const Size& rhs)
		{
			return !operator==(rhs);
		}

		Size& operator=(const Size& rhs)
		{
			if ((*this) != rhs)
			{
				Width = rhs.Width;
				Height = rhs.Height;
			}
			return *this;
		}
	};

	class GameBase : IDeviceNotify, Library::RenderTarget
	{
	public:
		GameBase();
		virtual ~GameBase();

		virtual void Initialize(void* window, std::function<void(Game::Size&)> getSizeCallBack);

		void CreateDevice();
		void CreateDeviceIndependentResources();
		virtual void CreateWindowSizeDependentResources() =0;

		virtual void Run() = 0;
		virtual void Update() = 0;
		virtual bool Render() = 0;

		virtual void Begin() override;
		virtual void End() override;

		void Draw(const Library::GameTime& gameTime);

		void Clear();

		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

		virtual  Library::GamePadComponent* GetGamePad() = 0;

		// Device Accessors.
		void SetOutputSize(const Size& sz)			{ m_outputSize = sz; }
		void SetLogicalSize(const Size& sz)			{ m_logicalSize = sz; }
		Size GetOutputSize() const					{ return m_outputSize; }
		Size GetLogicalSize() const					{ return m_logicalSize; }

		// D3D Accessors.
		ID3D11Device2*			GetD3DDevice() const					{ return m_d3dDevice.Get(); }
		ID3D11DeviceContext2*	GetD3DDeviceContext() const				{ return m_d3dContext.Get(); }
		IDXGISwapChain1*		GetSwapChain() const					{ return m_swapChain.Get(); }
		D3D_FEATURE_LEVEL		GetDeviceFeatureLevel() const			{ return m_d3dFeatureLevel; }
		ID3D11RenderTargetView*	GetBackBufferRenderTargetView() const	{ return m_d3dRenderTargetView.Get(); }
		ID3D11DepthStencilView* GetDepthStencilView() const				{ return m_d3dDepthStencilView.Get(); }
		D3D11_VIEWPORT			GetScreenViewport() const				{ return m_screenViewport; }

		void					SetOrientationTransform3D(const DirectX::XMFLOAT4X4& rotation)	{ m_orientationTransform3D = rotation; }
		DirectX::XMFLOAT4X4		GetOrientationTransform3D() const		{ return m_orientationTransform3D; }

		// D2D Accessors.
		ID2D1Factory2*			GetD2DFactory() const					{ return m_d2dFactory.Get(); }
		ID2D1Device1*			GetD2DDevice() const					{ return m_d2dDevice.Get(); }
		ID2D1DeviceContext1*	GetD2DDeviceContext() const				{ return m_d2dContext.Get(); }
		ID2D1Bitmap1*			GetD2DTargetBitmap() const				{ return m_d2dTargetBitmap.Get(); }
		IDWriteFactory2*		GetDWriteFactory() const				{ return m_dwriteFactory.Get(); }
		IWICImagingFactory2*	GetWicImagingFactory() const			{ return m_wicFactory.Get(); }
		
		void					SetOrientationTransform2D(const D2D1::Matrix3x2F& rotation)	{ m_orientationTransform2D = rotation; }
		D2D1::Matrix3x2F		GetOrientationTransform2D() const		{ return m_orientationTransform2D; }

		float					AspectRatio() const				{ return mAspectRatio; }
		void					SetAspectRatio(float newRatio)	{ mAspectRatio = newRatio; }

	protected:
		// Direct3D objects.
		Microsoft::WRL::ComPtr<ID3D11Device2>			m_d3dDevice;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext2>	m_d3dContext;
		Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_swapChain;

		// Direct3D rendering objects. Required for 3D.
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	m_d3dRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dDepthStencilView;
		D3D11_VIEWPORT									m_screenViewport;

		// Direct2D drawing components.
		Microsoft::WRL::ComPtr<ID2D1Factory2>		m_d2dFactory;
		Microsoft::WRL::ComPtr<ID2D1Device1>		m_d2dDevice;
		Microsoft::WRL::ComPtr<ID2D1DeviceContext1>	m_d2dContext;
		Microsoft::WRL::ComPtr<ID2D1Bitmap1>		m_d2dTargetBitmap;

		// DirectWrite drawing components.
		Microsoft::WRL::ComPtr<IDWriteFactory2>		m_dwriteFactory;
		Microsoft::WRL::ComPtr<IWICImagingFactory2>	m_wicFactory;

		// Cached device properties.
		D3D_FEATURE_LEVEL								m_d3dFeatureLevel;
		Size											m_d3dRenderTargetSize;
		Size											m_outputSize;
		Size											m_logicalSize;

		D2D1::Matrix3x2F	m_orientationTransform2D;
		DirectX::XMFLOAT4X4	m_orientationTransform3D;

		void*	m_window;
		std::function<void(Size&)>						f_getSize;
		Library::GameTime								mGameTime;

		float											mAspectRatio;

		std::vector<Library::GameComponent*> mComponents;
	};

}