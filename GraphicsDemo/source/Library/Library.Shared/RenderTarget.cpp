#include "pch.h"
#include "RenderTarget.h"

namespace Library
{
    RTTI_DEFINITIONS(RenderTarget)

    std::stack<RenderTarget::RenderTargetData> RenderTarget::sRenderTargetStack;

    RenderTarget::RenderTarget()
    {
    }

    RenderTarget::~RenderTarget()
    {
    }

    void RenderTarget::Begin(ID3D11DeviceContext* deviceContext, UINT viewCount, ID3D11RenderTargetView** renderTargetViews, ID3D11DepthStencilView* depthStencilView, const D3D11_VIEWPORT& viewport)
    {
        sRenderTargetStack.push(RenderTargetData(viewCount, renderTargetViews, depthStencilView, viewport));
		
		RebindCurrentRenderTargets(deviceContext);
    }

    void RenderTarget::End(ID3D11DeviceContext* deviceContext)
    {
        sRenderTargetStack.pop();

		RebindCurrentRenderTargets(deviceContext);
    }

	void RenderTarget::RebindCurrentRenderTargets(ID3D11DeviceContext* deviceContext)
	{
		RenderTargetData renderTargetData = sRenderTargetStack.top();
		deviceContext->OMSetRenderTargets(renderTargetData.ViewCount, renderTargetData.RenderTargetViews, renderTargetData.DepthStencilView);
		deviceContext->RSSetViewports(1, &renderTargetData.Viewport);
	}

	void RenderTarget::Clear(ID3D11DeviceContext* deviceContext)
	{
		RenderTargetData renderTargetData = sRenderTargetStack.top();

		deviceContext->ClearRenderTargetView(*renderTargetData.RenderTargetViews, reinterpret_cast<const float*>(&ColorHelper::CornflowerBlue));
		deviceContext->ClearDepthStencilView(renderTargetData.DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	}
}