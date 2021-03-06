//
// Main.cpp -
//

#include "pch.h"
#include "shlwapi.h"
#include "RenderingGame.h"
#include "GameTime.h"

#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif
using namespace DirectX;
using namespace Library;
using namespace Game;

namespace
{
    std::unique_ptr<RenderingGame> g_game;
};



LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Entry point
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    HRESULT hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
    if (FAILED(hr))
        return 1;

	g_game.reset(new RenderingGame());

    // Register class and create window
    {
        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInstance;
        wcex.hIcon = LoadIcon(hInstance, L"IDI_ICON");
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"Game_DesktopWindowClass";
        wcex.hIconSm = LoadIcon(wcex.hInstance, L"IDI_ICON");
        if (!RegisterClassEx(&wcex))
            return 1;

        // Create window
        size_t w = 800, h =600;
        //g_game->GetDefaultSize( w, h );

        RECT rc;
        rc.top = 0;
        rc.left = 0;
        rc.right = static_cast<LONG>( w ); 
        rc.bottom = static_cast<LONG>( h );
        HWND hwnd = CreateWindow(L"Game_DesktopWindowClass", L"Game.Desktop", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
            nullptr);
        if (!hwnd)
            return 1;

        ShowWindow(hwnd, nCmdShow);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(g_game.get()) );

		g_game->Initialize(hwnd, 
			[&hwnd](Game::Size& size)
			{
				RECT rc;
				GetWindowRect(hwnd, &rc);

				size.Width = static_cast<float>(rc.right - rc.left > 1 ? rc.right - rc.left : 1);
				size.Height = static_cast<float>(rc.bottom - rc.top > 1 ? rc.bottom - rc.top : 1);

				g_game->SetOutputSize(size);
				g_game->SetLogicalSize(size);


				D2D1::Matrix3x2F m_orientationTransform2D = D2D1::Matrix3x2F::Identity();
				DirectX::XMFLOAT4X4 m_orientationTransform3D = ScreenRotation::Rotation0;
				
				g_game->SetOrientationTransform2D(m_orientationTransform2D);
				g_game->SetOrientationTransform3D(m_orientationTransform3D);

				g_game->SetAspectRatio(4.0f / 3.0f);
			}
		);
    }

    // Main message loop
    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            g_game->Run();
        }
    }

    g_game.reset();

    CoUninitialize();

    return (int) msg.wParam;
}

// Windows procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

    static bool s_in_sizemove = false;
    static bool s_in_suspend = false;
    static bool s_minimized = false;

	auto game = reinterpret_cast<RenderingGame*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	
    switch (message)
    {
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
/*
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
        {
            if (!s_minimized)
            {
                s_minimized = true;
                if (!s_in_suspend && game)
                    game->OnSuspending();
                s_in_suspend = true;
            }
        }
        else if (s_minimized)
        {
            s_minimized = false;
            if (s_in_suspend && game)
                game->OnResuming();
            s_in_suspend = false;
        }
        else if ( !s_in_sizemove && game )
            game->OnWindowSizeChanged();
        break;

    case WM_ENTERSIZEMOVE:
        s_in_sizemove = true;
        break;

    case WM_EXITSIZEMOVE:
        s_in_sizemove = false;
        if (game)
            game->OnWindowSizeChanged();
        break;

    case WM_GETMINMAXINFO:
        {
            auto info = reinterpret_cast<MINMAXINFO*>(lParam);
            info->ptMinTrackSize.x = 320;
            info->ptMinTrackSize.y = 200;
        }
        break;

	
    case WM_ACTIVATEAPP:
        if (game)
        {
            if (wParam)
            {
                game->OnActivated();
            }
            else
            {
                game->OnDeactivated();
            }
        }
        break;

    case WM_POWERBROADCAST:
        switch (wParam)
        {
        case PBT_APMQUERYSUSPEND:
            if (!s_in_suspend && game)
                game->OnSuspending();
            s_in_suspend = true;
            return true;

        case PBT_APMRESUMESUSPEND:
            if (!s_minimized)
            {
                if (s_in_suspend && game)
                    game->OnResuming();
                s_in_suspend = false;
            }
            return true;
        }
        break;
*/
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}