#include "pch.h"
#include "DepthMap.h"
#include "GameBase.h"

namespace Library
{
    RTTI_DEFINITIONS(DepthMap)

    DepthMap::DepthMap(Game::GameBase& game, UINT width, UINT height)
        : RenderTarget(), mGame(&game), mDepthStencilView(nullptr),
          mOutputTexture(nullptr), mViewport()
    {
        D3D11_TEXTURE2D_DESC textureDesc;
        ZeroMemory(&textureDesc, sizeof(textureDesc));
        textureDesc.Width = width;
        textureDesc.Height = height;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL| D3D11_BIND_SHADER_RESOURCE;

        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture = nullptr;
		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateTexture2D(&textureDesc, nullptr, texture.ReleaseAndGetAddressOf()));

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
        ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
        resourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
        resourceViewDesc.Texture2D.MipLevels = 1;

		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateShaderResourceView(texture.Get(), &resourceViewDesc, mOutputTexture.ReleaseAndGetAddressOf()));

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
        ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
        depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilViewDesc.Texture2D.MipSlice = 0;    

		Utility::ThrowIfFailed(game.GetD3DDevice()->CreateDepthStencilView(texture.Get(), &depthStencilViewDesc, mDepthStencilView.ReleaseAndGetAddressOf()));


        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = static_cast<float>(width);
        mViewport.Height = static_cast<float>(height);
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;
    }

    
    ID3D11ShaderResourceView* DepthMap::OutputTexture() const
    {
        return mOutputTexture.Get();
    }

    ID3D11DepthStencilView* DepthMap::DepthStencilView() const
    {
		return mDepthStencilView.Get();
    }

    void DepthMap::Begin()
    {
        static ID3D11RenderTargetView* nullRenderTargetView = nullptr;
        RenderTarget::Begin(mGame->GetD3DDeviceContext(), 1, &nullRenderTargetView, mDepthStencilView.Get(), mViewport);
    }

    void DepthMap::End()
    {
		RenderTarget::End(mGame->GetD3DDeviceContext());
    }
}