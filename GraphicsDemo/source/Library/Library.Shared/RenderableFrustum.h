#pragma once

#include "DrawableGameComponent.h"
#include "Frustum.h"

namespace Library
{	
    class RenderableFrustum : public DrawableGameComponent
    {
        RTTI_DECLARATIONS(RenderableFrustum, DrawableGameComponent)

    public:
        RenderableFrustum(Game::GameBase& game, Camera& camera);
		RenderableFrustum(Game::GameBase& game, Camera& camera, const XMFLOAT4& color);
        ~RenderableFrustum() = default;

		const XMFLOAT3& Position() const;
        const XMFLOAT3& Direction() const;
        const XMFLOAT3& Up() const;
        const XMFLOAT3& Right() const;

		XMVECTOR PositionVector() const;
        XMVECTOR DirectionVector() const;
        XMVECTOR UpVector() const;
        XMVECTOR RightVector() const;

		void SetPosition(FLOAT x, FLOAT y, FLOAT z);
        void SetPosition(FXMVECTOR position);
        void SetPosition(const XMFLOAT3& position);

		void ApplyRotation(CXMMATRIX transform);
        void ApplyRotation(const XMFLOAT4X4& transform);

		void InitializeGeometry(const Frustum& frustum);

        virtual void Initialize() override;
        virtual void Update(const GameTime& gameTime) override;
		virtual void Draw(const GameTime& gameTime) override;

    private:
        RenderableFrustum();
        RenderableFrustum(const RenderableFrustum& rhs);
        RenderableFrustum& operator=(const RenderableFrustum& rhs);

		void InitializeVertexBuffer(const Frustum& frustum);
		void InitializeIndexBuffer();

		struct VSCBufferPerObject
		{
			XMFLOAT4X4 WorldViewProjection;
			XMFLOAT4X4 World;

			VSCBufferPerObject() : WorldViewProjection() { }

			VSCBufferPerObject(const XMFLOAT4X4& wvp, const XMFLOAT4X4& world) : WorldViewProjection(wvp), World(world) { }
		};


		VSCBufferPerObject mVertexCBufferPerObjectData;

		static const XMVECTORF32 DefaultColor;
		static const UINT FrustumVertexCount;		
		static const UINT FrustumPrimitiveCount;
		static const UINT FrustumIndicesPerPrimitive;
		static const UINT FrustumIndexCount;
		static const USHORT FrustumIndices[];

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexCBufferPerObject;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> mIndexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
		
		XMFLOAT4 mColor;
		XMFLOAT3 mPosition;
		XMFLOAT3 mDirection;
        XMFLOAT3 mUp;
        XMFLOAT3 mRight;

		XMFLOAT4X4 mWorldMatrix;
    };
}