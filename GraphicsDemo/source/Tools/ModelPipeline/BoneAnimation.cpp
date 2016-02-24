#include "stdafx.h"
#include "ModelExporter.h"
#include "BoneAnimation.h"
#include "Bone.h"
#include "Keyframe.h"
#include "Model.h"
#include "VectorHelper.h"
#include <assimp/scene.h>

namespace Pipeline
{
	BoneAnimation::BoneAnimation(Model& model, aiNodeAnim& nodeAnim)		
		: mModel(&model), mBone(nullptr), mKeyframes()
    {
		UINT boneIndex = model.BoneIndexMapping().at(nodeAnim.mNodeName.C_Str());
		mBone = model.Bones().at(boneIndex);

		assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumRotationKeys);
		assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumScalingKeys);

		for (UINT i = 0; i < nodeAnim.mNumPositionKeys; i++)
		{
			aiVectorKey positionKey = nodeAnim.mPositionKeys[i];
			aiQuatKey rotationKey = nodeAnim.mRotationKeys[i];
			aiVectorKey scaleKey = nodeAnim.mScalingKeys[i];

			assert(positionKey.mTime == rotationKey.mTime);
			assert(positionKey.mTime == scaleKey.mTime);
			
			Keyframe* keyframe = new Keyframe(static_cast<float>(positionKey.mTime), XMFLOAT3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z),
				XMFLOAT4(rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z, rotationKey.mValue.w), XMFLOAT3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z));
			mKeyframes.push_back(keyframe);
		}
    }

    BoneAnimation::~BoneAnimation()
    {
		for (Keyframe* keyframe : mKeyframes)
		{
			delete keyframe;
		}
    }

	Bone& BoneAnimation::GetBone()
	{
		return *mBone;
	}
	
	const std::vector<Keyframe*> BoneAnimation::Keyframes() const
	{
		return mKeyframes;
	}

	void BoneAnimation::Save(std::ofstream& file)
	{
		ModelExporter::WriteInteger(mBone->Index(), file);
		ModelExporter::WriteInteger(mKeyframes.size(), file);

		for (Keyframe* keyframe : mKeyframes)
		{
			keyframe->Save(file);
		}
	}
}
