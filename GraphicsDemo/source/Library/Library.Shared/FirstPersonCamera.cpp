#include "pch.h"
#include "GamePadComponent.h"
#include "FirstPersonCamera.h"
#include "GameTime.h"
#include "VectorHelper.h"

namespace Library
{
    RTTI_DEFINITIONS(FirstPersonCamera)

    const float FirstPersonCamera::DefaultRotationRate = XMConvertToRadians(1.0f);
    const float FirstPersonCamera::DefaultMovementRate = 10.0f;
	const float FirstPersonCamera::DefaultSensitivity = 100.0f;

    FirstPersonCamera::FirstPersonCamera(Game::GameBase& game)
		: Camera(game), mSensitivity(DefaultSensitivity), mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate), mMoveInZSpace(false)
    {
    }

	FirstPersonCamera::FirstPersonCamera(Game::GameBase& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance)
		: Camera(game, fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance), mSensitivity(DefaultSensitivity),
		mRotationRate(DefaultRotationRate), mMovementRate(DefaultMovementRate), mMoveInZSpace(false)

	{
	}

    float&FirstPersonCamera::Sensitivity()
    {
        return mSensitivity;
    }


    float& FirstPersonCamera::RotationRate()
    {
        return mRotationRate;
    }

    float& FirstPersonCamera::MovementRate()
    {
        return mMovementRate;
    }

    void FirstPersonCamera::Initialize()
    {
        Camera::Initialize();
    }

	void FirstPersonCamera::Update(const GameTime& gameTime)
	{
		GamePadComponent* gamepad = mGame->GetGamePad();
		if (gamepad != nullptr)
		{
			if (gamepad->WasButtonPressedThisFrame(GamePadButton::LeftStick))
			{
				CameraControlFlag = !CameraControlFlag;
			}
			if (gamepad->WasButtonPressedThisFrame(GamePadButton::RightStick) && CameraControlFlag)
			{
				mMoveInZSpace = !mMoveInZSpace;
			}
			
		}

		XMFLOAT3 movementAmount = Vector3Helper::Zero;
		XMFLOAT2 rotationAmount = Vector2Helper::Zero;

		if (CameraControlFlag)
		{
			if (gamepad != nullptr)
			{
				movementAmount.x = gamepad->GamePad()->GetState(0).thumbSticks.leftX;
				movementAmount.y = gamepad->GamePad()->GetState(0).thumbSticks.leftY;

				if (mMoveInZSpace)
				{
					rotationAmount.x = -gamepad->GamePad()->GetState(0).thumbSticks.rightX * mSensitivity;
					movementAmount.z = gamepad->GamePad()->GetState(0).thumbSticks.rightY;
				}
				else
				{
					rotationAmount.x = -gamepad->GamePad()->GetState(0).thumbSticks.rightX * mSensitivity;
					rotationAmount.y = -gamepad->GamePad()->GetState(0).thumbSticks.rightY * mSensitivity;
				}
			}
		}
	

		float elapsedTime = static_cast<float>(gameTime.ElapsedGameTime().count()) / 1000.0f;
        XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount) * mRotationRate * elapsedTime;
        XMVECTOR right = XMLoadFloat3(&mRight);

        XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, XMVectorGetY(rotationVector));
        XMMATRIX yawMatrix = XMMatrixRotationY(XMVectorGetX(rotationVector));

        ApplyRotation(XMMatrixMultiply(pitchMatrix, yawMatrix));

        XMVECTOR position = XMLoadFloat3(&mPosition);
		XMVECTOR movement = XMLoadFloat3(&movementAmount) * mMovementRate * elapsedTime;

		XMVECTOR strafe = right * XMVectorGetX(movement);
        position += strafe;

        XMVECTOR forward = XMLoadFloat3(&mDirection) * XMVectorGetY(movement);
        position += forward;
        

		XMVECTOR up = XMLoadFloat3(&mUp) * XMVectorGetZ(movement);
		position += up;

        XMStoreFloat3(&mPosition, position);

        Camera::Update(gameTime);
    }
}
