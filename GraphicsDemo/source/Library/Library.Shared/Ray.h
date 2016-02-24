#pragma once
#include "DirectXMath.h"
namespace Library
{
    class Ray
    {
    public:
		Ray(DirectX::FXMVECTOR position, DirectX::FXMVECTOR direction);
		Ray(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction);
		
		const DirectX::XMFLOAT3& Position() const;
		const DirectX::XMFLOAT3& Direction() const;

		DirectX::XMVECTOR PositionVector() const;
		DirectX::XMVECTOR DirectionVector() const;

		virtual void SetPosition(float x, float y, float z);
		virtual void SetPosition(DirectX::FXMVECTOR position);
		virtual void SetPosition(const DirectX::XMFLOAT3& position);

		virtual void SetDirection(float x, float y, float z);
		virtual void SetDirection(DirectX::FXMVECTOR direction);
		virtual void SetDirection(const DirectX::XMFLOAT3& direction);
    
	private:
		Ray();

		DirectX::XMFLOAT3 mPosition;
		DirectX::XMFLOAT3 mDirection;
    };
}

