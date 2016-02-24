#pragma once
#include "GameComponent.h"
class TransformableComponent :
	public Library::GameComponent
{
	RTTI_DECLARATIONS(TransformableComponent, GameComponent)

public:
	TransformableComponent(Game::GameBase& game);
	~TransformableComponent() = default;

	const XMFLOAT3& Position() const;
	const XMFLOAT3& Direction() const;
	const XMFLOAT3& Up() const;
	const XMFLOAT3& Right() const;

	virtual void SetPosition(FLOAT x, FLOAT y, FLOAT z);
	virtual void SetPosition(FXMVECTOR position);
	virtual void SetPosition(const XMFLOAT3& position);

	void ApplyRotation(CXMMATRIX transform);
	void ApplyRotation(const XMFLOAT4X4& transform);
protected:
	XMFLOAT3 mPosition;
	XMFLOAT3 mDirection;
	XMFLOAT3 mUp;
	XMFLOAT3 mRight;

	XMFLOAT4 mQuaternionRotation;
	XMFLOAT4X4 mTransformMatrix;
};

