#pragma once

#include "Loadable.h"

namespace Library
{
    class Keyframe : public Loadable
    {
		friend class BoneAnimation;

    public:
		float Time() const;
		const XMFLOAT3& Translation() const;
		const XMFLOAT4& RotationQuaternion() const;
		const XMFLOAT3& Scale() const;

		XMVECTOR TranslationVector() const;
		XMVECTOR RotationQuaternionVector() const;
		XMVECTOR ScaleVector() const;

		XMMATRIX Transform() const;

	protected:
		void Load(std::ifstream& file) override;
    private:
		//Keyframe(float time, const XMFLOAT3& translation, const XMFLOAT4& rotationQuaternion, const XMFLOAT3& scale);

		Keyframe() = default;
        Keyframe(const Keyframe& rhs);
        Keyframe& operator=(const Keyframe& rhs);

		float mTime;
		XMFLOAT3 mTranslation;
		XMFLOAT4 mRotationQuaternion;
		XMFLOAT3 mScale;
    };
}
