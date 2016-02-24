#include "stdafx.h"
#include "ModelExporter.h"
#include "Bone.h"
#include "Model.h"
#include "MatrixHelper.h"
#include <assimp/scene.h>

namespace Pipeline
{
	const std::vector<BoneVertexWeights::VertexWeight>& BoneVertexWeights::Weights()
	{
		return mWeights;
	}

	void BoneVertexWeights::AddWeight(float weight, UINT boneIndex, UINT vertexID)
	{
		if (mWeights.size() == MaxBoneWeightsPerVertex)
		{
			throw std::exception("Maximum number of bone weights per vertex exceeded.");
		}

		mWeights.push_back(VertexWeight(weight, boneIndex, vertexID));
	}

	RTTI_DEFINITIONS(Bone)

	Bone::Bone(const std::string& name, UINT index, const XMFLOAT4X4& offsetTransform)
	   : SceneNode(name), mIndex(index), mOffsetTransform(offsetTransform)
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

	void Bone::Save(std::ofstream& file)
	{
		ModelExporter::WriteInteger(Name().size(), file);

		if (Name().size() > 0)
		{
			file.write(Name().c_str(), Name().size()*sizeof(char));
		}

		//Bone index.
		ModelExporter::WriteInteger(Index(), file);

		float transformCell = 0;
		for (uint32_t i = 0; i < 16; ++i)
		{
			transformCell = mOffsetTransform.m[i / 4][i % 4];
			file.write((char*)&transformCell, sizeof(float));
		}
	}
}
