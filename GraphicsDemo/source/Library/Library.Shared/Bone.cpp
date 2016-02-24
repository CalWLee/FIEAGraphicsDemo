#include "pch.h"
#include "Bone.h"
#include "Model.h"
#include "MatrixHelper.h"

namespace Library
{
	const std::vector<BoneVertexWeights::VertexWeight>& BoneVertexWeights::Weights()
	{
		return mWeights;
	}

	void BoneVertexWeights::AddWeight(float weight, UINT boneIndex)
	{
		if (mWeights.size() == MaxBoneWeightsPerVertex)
		{
			throw std::exception("Maximum number of bone weights per vertex exceeded.");
		}

		mWeights.push_back(VertexWeight(weight, boneIndex));
	}

	RTTI_DEFINITIONS(Bone)
	
	Bone::Bone(Model& model)
		: SceneNode(model), mIndex(), mOffsetTransform()
	{

	}

	UINT Bone::Index() const
	{
		return mIndex;
	}

	void Bone::SetIndex(UINT index)
	{
		mIndex = index;
	}

	const XMFLOAT4X4& Bone::OffsetTransform() const
	{
		return mOffsetTransform;
	}

	XMMATRIX Bone::OffsetTransformMatrix() const
	{
		return XMLoadFloat4x4(&mOffsetTransform);
	}


	void Bone::Load(std::ifstream& file)
	{
		uint32_t nameSize=0;
		file.read((char*)&nameSize, sizeof(uint32_t));

		ReadString(file, nameSize, mName);

		//Bone index.
		file.read((char*)&mIndex, sizeof(uint32_t));

		for (uint32_t i = 0; i < 16; ++i)
		{
			file.read((char*)&mOffsetTransform.m[i / 4][i % 4], sizeof(float));
		}
	}

	void Bone::LoadHelper(std::ifstream& file, SceneNode* parent)
	{
		if (parent != nullptr)
		{
			SetParent(parent);
		}

		for (uint32_t i = 0; i < 16; ++i)
		{
			file.read((char*)&mTransform.m[i / 4][i % 4], sizeof(float));
		}

		uint32_t nChildren = 0;
		file.read((char*)&nChildren, sizeof(uint32_t));

		for (uint32_t i = 0; i < nChildren; ++i)
		{
			bool isBone = false;
			file.read((char*)&isBone, sizeof(bool));

			if (!isBone)
			{
				std::shared_ptr<SceneNode> childSceneNode = std::shared_ptr<SceneNode>(new SceneNode(*mModel));
				Children().push_back(childSceneNode);

				childSceneNode->LoadHelper(file, this);
			}
			else
			{
				std::string boneName;
				uint32_t nameSize = 0;
				file.read((char*)&nameSize, sizeof(uint32_t));
				ReadString(file, nameSize, boneName);

				auto boneMapping = mModel->BoneIndexMapping().find(boneName)->second;
				Children().push_back(mModel->Bones()[boneMapping]);

				mModel->Bones()[boneMapping]->LoadHelper(file, this);
			}
		}
	}
}
