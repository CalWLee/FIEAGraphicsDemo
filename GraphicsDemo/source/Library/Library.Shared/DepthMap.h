#pragma once

#include "RenderTarget.h"

namespace Library
{
    class DepthMap : public RenderTarget
    {
		RTTI_DECLARATIONS(DepthMap, RenderTarget)

    public:
        DepthMap(Game::GameBase& game, UINT width, UINT height);
        ~DepthMap() = default;

		ID3D11ShaderResourceView* OutputTexture() const;
		ID3D11DepthStencilView* DepthStencilView() const;
		
        virtual void Begin() override;
		virtual void End() override;

    private:
        DepthMap();
        DepthMap(const DepthMap& rhs);
        DepthMap& operator=(const DepthMap& rhs);

        Game::GameBase* mGame;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mOutputTexture;
		D3D11_VIEWPORT mViewport;
    };
}