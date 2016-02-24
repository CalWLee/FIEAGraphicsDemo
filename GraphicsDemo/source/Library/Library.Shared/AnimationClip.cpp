#include "pch.h"
#include "AnimationClip.h"
#include "BoneAnimation.h"
#include "Bone.h"
#include "MatrixHelper.h"

namespace Library
{
	AnimationClip::AnimationClip(Model& model)
		: mName(""), mDuration(0), mTicksPerSecond(0), mModel(&model),
		  mBoneAnimations(), mBoneAnimationsByBone(), mKeyframeCount(0)
    {
		/*
		assert(animation.mNumChannels > 0);

		if (mTicksPerSecond <= 0.0f)
		{
			mTicksPerSecond = 1.0f;
		}

		for (UINT i = 0; i < animation.mNumChannels; i++)
		{
			BoneAnimation* boneAnimation = new BoneAnimation(model, *(animation.mChannels[i]));
			mBoneAnimations.push_back(boneAnimation);

			assert(mBoneAnimationsByBone.find(&(boneAnimation->GetBone())) == mBoneAnimationsByBone.end());
			mBoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation;
		}

		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			if (boneAnimation->Keyframes().size() > mKeyframeCount)
			{
				mKeyframeCount = boneAnimation->Keyframes().size();
			}
		}
		*/
    }

    AnimationClip::~AnimationClip()
    {
		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			delete boneAnimation;
		}
    }

	const std::string& AnimationClip::Name() const
	{
		return mName;
	}

	float AnimationClip::Duration() const
	{
		return mDuration;
	}

	float AnimationClip::TicksPerSecond() const
	{
		return mTicksPerSecond;
	}

	const std::vector<BoneAnimation*>& AnimationClip::BoneAnimations() const
	{
		return mBoneAnimations;
	}

	const std::map<Bone*, BoneAnimation*>& AnimationClip::BoneAnimationsByBone() const
	{
		return mBoneAnimationsByBone;
	}

	const UINT AnimationClip::KeyframeCount() const
	{
		return mKeyframeCount;
	}

	UINT AnimationClip::GetTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			return foundBoneAnimation->second->GetTransform(time, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
			return UINT_MAX;
		}
	}

	void AnimationClip::GetTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetTransformAtKeyframe(UINT keyframe, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetTransformAtKeyframe(keyframe, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
		}
	}

	void AnimationClip::GetTransformsAtKeyframe(UINT keyframe, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetTransformAtKeyframe(keyframe, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::GetInteropolatedTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
	{
		auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
		if (foundBoneAnimation != mBoneAnimationsByBone.end())
		{
			foundBoneAnimation->second->GetInteropolatedTransform(time, transform);
		}
		else
		{
			transform = MatrixHelper::Identity;
		}
	}

	void AnimationClip::GetInteropolatedTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
	{
		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			boneAnimation->GetInteropolatedTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
		}
	}

	void AnimationClip::Load(std::ifstream& file)
	{
		uint32_t nameLength = 0;
		file.read((char*)&nameLength, sizeof(uint32_t));
		ReadString(file, nameLength, mName);

		file.read((char*)&mTicksPerSecond, sizeof(float));
		file.read((char*)&mKeyframeCount, sizeof(uint32_t));
		file.read((char*)&mDuration, sizeof(float));

		//Bone index.
		uint32_t nBoneAnimations = static_cast<uint32_t>(mBoneAnimations.size());
		file.read((char*)&nBoneAnimations, sizeof(uint32_t));
		for (uint32_t i = 0; i < nBoneAnimations; ++i)
		{
			BoneAnimation* boneAnimation = new BoneAnimation(*mModel);
			mBoneAnimations.push_back(boneAnimation);

			boneAnimation->Load(file);

			assert(mBoneAnimationsByBone.find(&(boneAnimation->GetBone())) == mBoneAnimationsByBone.end());
			mBoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation;
		}
	}

}
