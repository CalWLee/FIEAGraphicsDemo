
#include "pch.h"
#include "GameBase.h"
#include "RenderingGame.h"
#include <windows.h>
#include <agile.h>
#include <concrt.h>
#include <collection.h>
#include <ppltasks.h>
#include "App.h"

using namespace Game;
using namespace Library;
using namespace DirectX;
using namespace D2D1;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	//m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_main = std::unique_ptr<RenderingGame>(new RenderingGame());
}


// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	m_window = window;

	m_window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	m_window->Closed +=
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	m_displayInfo = DisplayInformation::GetForCurrentView();

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	m_window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	m_displayInfo->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	m_displayInfo->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	// Disable all pointer visual feedback for better performance when touching.
	// This is not supported on Windows Phone applications.
	auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
	pointerVisualizationSettings->IsContactFeedbackEnabled = false; 
	pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
#endif
	IUnknown* thing = reinterpret_cast<IUnknown*>(m_window.Get());
	m_main->Initialize(thing,
		[&](Game::Size& size){


		m_main->SetLogicalSize(Game::Size(m_window->Bounds.Width, m_window->Bounds.Height));

		// Calculate the necessary render target size in pixels.
		float Width = Library::Utility::ConvertDipsToPixels(m_window->Bounds.Width, m_displayInfo->LogicalDpi);
		float Height = Library::Utility::ConvertDipsToPixels(m_window->Bounds.Height, m_displayInfo->LogicalDpi);

		// Prevent zero size DirectX content from being created.
		Width = max(Width, 1);
		Height = max(Height, 1);

		m_main->SetOutputSize(Size(Width, Height));

		// The width and height of the swap chain must be based on the window's
		// natively-oriented width and height. If the window is not in the native
		// orientation, the dimensions must be reversed.
		DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;

		switch (m_displayInfo->NativeOrientation)
		{
		case DisplayOrientations::Landscape:
			switch (m_displayInfo->CurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;
			}
			break;

		case DisplayOrientations::Portrait:
			switch (m_displayInfo->CurrentOrientation)
			{
			case DisplayOrientations::Landscape:
				rotation = DXGI_MODE_ROTATION_ROTATE90;
				break;

			case DisplayOrientations::Portrait:
				rotation = DXGI_MODE_ROTATION_IDENTITY;
				break;

			case DisplayOrientations::LandscapeFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE270;
				break;

			case DisplayOrientations::PortraitFlipped:
				rotation = DXGI_MODE_ROTATION_ROTATE180;
				break;
			}
			break;
		}

		bool swapDimensions = rotation == DXGI_MODE_ROTATION_ROTATE90 || rotation == DXGI_MODE_ROTATION_ROTATE270;
		size.Width = swapDimensions ? Height : Width;
		size.Height = swapDimensions ? Width : Height;



		D2D1::Matrix3x2F m_orientationTransform2D = Matrix3x2F::Identity();
		DirectX::XMFLOAT4X4 m_orientationTransform3D = ScreenRotation::Rotation0;

		switch (rotation)
		{
		case DXGI_MODE_ROTATION_IDENTITY:
			m_orientationTransform2D = Matrix3x2F::Identity();
			m_orientationTransform3D = ScreenRotation::Rotation0;
			break;

		case DXGI_MODE_ROTATION_ROTATE90:
			m_orientationTransform2D =
				Matrix3x2F::Rotation(90.0f) *
				Matrix3x2F::Translation(Height, 0.0f);
			m_orientationTransform3D = ScreenRotation::Rotation270;
			break;

		case DXGI_MODE_ROTATION_ROTATE180:
			m_orientationTransform2D =
				Matrix3x2F::Rotation(180.0f) *
				Matrix3x2F::Translation(Width, Height);
			m_orientationTransform3D = ScreenRotation::Rotation180;
			break;

		case DXGI_MODE_ROTATION_ROTATE270:
			m_orientationTransform2D =
				Matrix3x2F::Rotation(270.0f) *
				Matrix3x2F::Translation(0.0f, Width);
			m_orientationTransform3D = ScreenRotation::Rotation90;
			break;

		default:
			throw std::exception("");
		}

		m_main->SetOrientationTransform2D(m_orientationTransform2D);
		m_main->SetOrientationTransform3D(m_orientationTransform3D);


		m_main->SetAspectRatio(Width / Height);
	});
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{

	/*
	if (m_main == nullptr)
	{

		m_main = std::unique_ptr<RenderingGameWindows>(new RenderingGameWindows());
		m_main->SetWindow(m_windowPointer.Get());
	}*/
}

// This method is called after the window becomes active.
void App::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			m_main->Run();
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
        //m_main->Trim();

		// Insert your code here.

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// Insert your code here.
}

// Window event handlers.

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	m_window = sender;
	//m_main->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	m_main->CreateWindowSizeDependentResources();
}
#endif

// DisplayInformation event handlers.

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	//m_deviceResources->ValidateDevice();
}

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	m_displayInfo = sender;
	//m_main->SetDpi(sender->LogicalDpi);
	//m_main->CreateWindowSizeDependentResources();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	m_displayInfo = sender;
	//m_main->SetCurrentOrientation(sender->CurrentOrientation);
	//m_main->CreateWindowSizeDependentResources();
}
#endif