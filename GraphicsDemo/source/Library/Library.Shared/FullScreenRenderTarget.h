#pragma once

namespace Library
{
    class FullScreenRenderTarget : public  RenderTarget
    {
    public:
        FullScreenRenderTarget(Game::GameBase& game);
        ~FullScreenRenderTarget() = default;

        ID3D11ShaderResourceView* OutputTexture() const;
        ID3D11RenderTargetView* RenderTargetView() const;
        ID3D11DepthStencilView* DepthStencilView() const;

        void Begin() override;
		void End() override;

    private:
        FullScreenRenderTarget();
        FullScreenRenderTarget(const FullScreenRenderTarget& rhs);
        FullScreenRenderTarget& operator=(const FullScreenRenderTarget& rhs);

        Game::GameBase* mGame;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView> mRenderTargetView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> mDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> mOutputTexture;
    };
}