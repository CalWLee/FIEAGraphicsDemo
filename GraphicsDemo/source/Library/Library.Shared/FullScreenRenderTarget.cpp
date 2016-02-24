#include "pch.h"
#include "FullScreenRenderTarget.h"

namespace Library
{
    FullScreenRenderTarget::FullScreenRenderTarget(Game::GameBase& game)
        : mGame(&game), mRenderTargetView(nullptr), mDepthStencilView(nullptr), mOutputTexture(nullptr)
    {
        D3D11_TEXTURE2D_DESC fullScreenTextureDesc;
        ZeroMemory(&fullScreenTextureDesc, sizeof(fullScreenTextureDesc));
		fullScreenTextureDesc.Width = static_cast<UINT>(game.GetOutputSize().Width);
		fullScreenTextureDesc.Height = static_cast<UINT>(game.GetOutputSize().Height);
        fullScreenTextureDesc.MipLevels = 1;
        fullScreenTextureDesc.ArraySize = 1;
        fullScreenTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        fullScreenTextureDesc.SampleDesc.Count = 1;
        fullScreenTextureDesc.SampleDesc.Quality = 0;
        fullScreenTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        fullScreenTextureDesc.Usage = D3D11_USAGE_DEFAULT;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateTexture2D(&fullScreenTextureDesc, nullptr, fullScreenTexture.ReleaseAndGetAddressOf()));
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateShaderResourceView(fullScreenTexture.Get(), nullptr, mOutputTexture.ReleaseAndGetAddressOf()));
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateRenderTargetView(fullScreenTexture.Get(), nullptr, mRenderTargetView.ReleaseAndGetAddressOf()));

        D3D11_TEXTURE2D_DESC depthStencilDesc;
        ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.Width = static_cast<UINT>(game.GetOutputSize().Width);
		depthStencilDesc.Height = static_cast<UINT>(game.GetOutputSize().Height);
        depthStencilDesc.MipLevels = 1;
        depthStencilDesc.ArraySize = 1;
        depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
        depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;            

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = nullptr;
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.ReleaseAndGetAddressOf()));
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, mDepthStencilView.ReleaseAndGetAddressOf()));
    }

    ID3D11ShaderResourceView* FullScreenRenderTarget::OutputTexture() const
    {
		return mOutputTexture.Get();
    }

    ID3D11RenderTargetView* FullScreenRenderTarget::RenderTargetView() const
    {
		return mRenderTargetView.Get();
    }

    ID3D11DepthStencilView* FullScreenRenderTarget::DepthStencilView() const
    {
		return mDepthStencilView.Get();
    }

    void FullScreenRenderTarget::Begin()
    {	
		
		RenderTarget::Begin(mGame->GetD3DDeviceContext(), 1, mRenderTargetView.GetAddressOf(), mDepthStencilView.Get(), mGame->GetScreenViewport());
    }

    void FullScreenRenderTarget::End()
    {
		RenderTarget::End(mGame->GetD3DDeviceContext());
    }
}