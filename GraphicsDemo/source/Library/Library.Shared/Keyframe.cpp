#include "pch.h"
#include "Keyframe.h"
#include "VectorHelper.h"

namespace Library
{
	float Keyframe::Time() const
	{
		return mTime;
	}

	const XMFLOAT3& Keyframe::Translation() const
	{	
		return mTranslation;
	}

	const XMFLOAT4& Keyframe::RotationQuaternion() const
	{
		return mRotationQuaternion;
	}

	const XMFLOAT3& Keyframe::Scale() const
	{
		return mScale;
	}

	XMVECTOR Keyframe::TranslationVector() const
	{
		return XMLoadFloat3(&mTranslation);
	}

	XMVECTOR Keyframe::RotationQuaternionVector() const
	{
		return XMLoadFloat4(&mRotationQuaternion);
	}

	XMVECTOR Keyframe::ScaleVector() const
	{
		return XMLoadFloat3(&mScale);
	}

	XMMATRIX Keyframe::Transform() const
	{
		static XMVECTOR rotationOrigin = XMLoadFloat4(&Vector4Helper::Zero);

		return XMMatrixAffineTransformation(ScaleVector(), rotationOrigin, RotationQuaternionVector(), TranslationVector());
	}

	void Keyframe::Load(std::ifstream& file)
	{
		file.read((char*)&mTime, sizeof(float));

		file.read((char*)&mScale.x, sizeof(float));
		file.read((char*)&mScale.y, sizeof(float));
		file.read((char*)&mScale.z, sizeof(float));

		file.read((char*)&mRotationQuaternion.x, sizeof(float));
		file.read((char*)&mRotationQuaternion.y, sizeof(float));
		file.read((char*)&mRotationQuaternion.z, sizeof(float));
		file.read((char*)&mRotationQuaternion.w, sizeof(float));

		file.read((char*)&mTranslation.x, sizeof(float));
		file.read((char*)&mTranslation.y, sizeof(float));
		file.read((char*)&mTranslation.z, sizeof(float));

	}
}
