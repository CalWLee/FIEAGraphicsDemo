#include "pch.h"
#include "RenderStateHelper.h"

namespace Library
{
    RenderStateHelper::RenderStateHelper(Game::GameBase& game)
        : mGame(game), mRasterizerState(nullptr), mBlendState(nullptr), mBlendFactor(new FLOAT[4]), mSampleMask(UINT_MAX), mDepthStencilState(nullptr), mStencilRef(UINT_MAX)
    {
    }

    RenderStateHelper::~RenderStateHelper()
    {
		delete mBlendFactor;
    }

    void RenderStateHelper::ResetAll(ID3D11DeviceContext* deviceContext)
    {
        deviceContext->RSSetState(nullptr);
        deviceContext->OMSetBlendState(nullptr, nullptr, UINT_MAX);
        deviceContext->OMSetDepthStencilState(nullptr, UINT_MAX);
    }

	ID3D11RasterizerState* RenderStateHelper::RasterizerState()
	{
		return mRasterizerState.Get();
	}

    ID3D11BlendState* RenderStateHelper::BlendState()
	{
		return mBlendState.Get();
	}

    ID3D11DepthStencilState* RenderStateHelper::DepthStencilState()
	{
		return mDepthStencilState.Get();
	}

    void RenderStateHelper::SaveRasterizerState()
    {
        mGame.GetD3DDeviceContext()->RSGetState(mRasterizerState.ReleaseAndGetAddressOf());
    }

    void RenderStateHelper::RestoreRasterizerState() const
    {
        mGame.GetD3DDeviceContext()->RSSetState(mRasterizerState.Get());
    }

    void RenderStateHelper::SaveBlendState()
    {
		mGame.GetD3DDeviceContext()->OMGetBlendState(mBlendState.ReleaseAndGetAddressOf(), mBlendFactor, &mSampleMask);
    }

    void RenderStateHelper::RestoreBlendState() const
    {
		mGame.GetD3DDeviceContext()->OMSetBlendState(mBlendState.Get(), mBlendFactor, mSampleMask);
    }

    void RenderStateHelper::SaveDepthStencilState()
    {
		mGame.GetD3DDeviceContext()->OMGetDepthStencilState(mDepthStencilState.ReleaseAndGetAddressOf(), &mStencilRef);
    }

    void RenderStateHelper::RestoreDepthStencilState() const
    {
		mGame.GetD3DDeviceContext()->OMSetDepthStencilState(mDepthStencilState.Get(), mStencilRef);
    }

    void RenderStateHelper::SaveAll()
    {
        SaveRasterizerState();
        SaveBlendState();
        SaveDepthStencilState();
    }

    void RenderStateHelper::RestoreAll() const
    {
        RestoreRasterizerState();
        RestoreBlendState();
        RestoreDepthStencilState();
    }
}
