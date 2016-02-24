
#include "stdafx.h"
#include "ModelExporter.h"
#include "AnimationClip.h"
#include "BoneAnimation.h"
#include "Bone.h"
#include "MatrixHelper.h"
#include <assimp/scene.h>

namespace Pipeline
{
	AnimationClip::AnimationClip(Model& model, aiAnimation& animation)
		: mName(animation.mName.C_Str()), mDuration(static_cast<float>(animation.mDuration)), mTicksPerSecond(static_cast<float>(animation.mTicksPerSecond)),
		  mBoneAnimations(), mBoneAnimationsByBone(), mKeyframeCount(0)
    {
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

	void AnimationClip::Save(std::ofstream& file)
	{
		ModelExporter::WriteInteger(mName.size(), file);
		if (mName.size() > 0)
		{
			file.write(mName.c_str(), mName.size()*sizeof(char));
		}

		ModelExporter::WriteFloat(mTicksPerSecond, file);
		ModelExporter::WriteInteger(mKeyframeCount, file);
		ModelExporter::WriteFloat(mDuration, file);

		//Bone index.
		ModelExporter::WriteInteger(mBoneAnimations.size(), file);
		for (BoneAnimation* boneAnimation : mBoneAnimations)
		{
			boneAnimation->Save(file);
		}
	}
}
