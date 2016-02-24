#pragma once

namespace Library
{
	class Game::GameBase;

    class RenderStateHelper
    {
    public:
		RenderStateHelper(Game::GameBase& game);
        ~RenderStateHelper();

		RenderStateHelper(const RenderStateHelper& rhs) = delete;
		RenderStateHelper& operator=(const RenderStateHelper& rhs) = delete;

        static void ResetAll(ID3D11DeviceContext* deviceContext);

        ID3D11RasterizerState* RasterizerState();
        ID3D11BlendState* BlendState();
        ID3D11DepthStencilState* DepthStencilState();

        void SaveRasterizerState();
        void RestoreRasterizerState() const;

        void SaveBlendState();
        void RestoreBlendState() const;

        void SaveDepthStencilState();
        void RestoreDepthStencilState() const;

        void SaveAll();
        void RestoreAll() const;

    private:
		Game::GameBase& mGame;

        Microsoft::WRL::ComPtr<ID3D11RasterizerState> mRasterizerState;
		Microsoft::WRL::ComPtr<ID3D11BlendState> mBlendState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> mDepthStencilState;
        FLOAT* mBlendFactor;
        UINT mSampleMask;
        UINT mStencilRef;
    };
}
