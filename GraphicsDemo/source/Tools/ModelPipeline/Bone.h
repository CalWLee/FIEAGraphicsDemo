#pragma once

#include "stdafx.h"
#include "RTTI.h"
#include "SceneNode.h"
using namespace Library;
namespace Pipeline
{
	class BoneVertexWeights
	{
	public:
		typedef struct _VertexWeight
		{
			float Weight;
			UINT BoneIndex;
			UINT VertexID;

			_VertexWeight(float weight, UINT boneIndex, UINT vertexID)
				: Weight(weight), BoneIndex(boneIndex), VertexID(vertexID) { }
		} VertexWeight;
		
		const std::vector<VertexWeight>& Weights();

		void AddWeight(float weight, UINT boneIndex, UINT vertexID);

		static const UINT MaxBoneWeightsPerVertex = 4U;

	private:
		std::vector<VertexWeight> mWeights;
	};

    class Bone : public SceneNode
    {
		friend class Model;
		RTTI_DECLARATIONS(Bone, SceneNode)

    public:	
		UINT Index() const;
		void SetIndex(UINT index);

		const XMFLOAT4X4& OffsetTransform() const;
		XMMATRIX OffsetTransformMatrix() const;

		Bone(const std::string& name, UINT index, const XMFLOAT4X4& offsetTransform);

	protected:
		void Save(std::ofstream& file) override;
    private:
		Bone();
        Bone(const Bone& rhs);
        Bone& operator=(const Bone& rhs);

		UINT mIndex;					// Index into the model's bone container
		XMFLOAT4X4 mOffsetTransform;	// Transforms from mesh space to bone space
    };
}
