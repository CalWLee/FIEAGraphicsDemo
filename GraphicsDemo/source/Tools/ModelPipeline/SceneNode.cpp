#include "stdafx.h"
#include "ModelExporter.h"
#include "SceneNode.h"
#include "Bone.h"
#include "Serializable.h"
#include "MatrixHelper.h"

using namespace Library;
namespace Pipeline
{
	RTTI_DEFINITIONS(SceneNode)

	SceneNode::SceneNode(const std::string& name)
		: mName(name), mParent(nullptr), mChildren(), mTransform(MatrixHelper::Identity)
	{
	}

	SceneNode::SceneNode(const std::string& name, const XMFLOAT4X4& transform)
		: mName(name), mParent(nullptr), mChildren(), mTransform(transform)
    {
    }

	const std::string& SceneNode::Name() const
	{
		return mName;
	}

	SceneNode* SceneNode::GetParent()
	{
		return mParent;
	}

	std::vector<SceneNode*>& SceneNode::Children()
	{
		return mChildren;
	}

	const XMFLOAT4X4& SceneNode::Transform() const
	{
		return mTransform;
	}

	XMMATRIX SceneNode::TransformMatrix() const
	{
		return XMLoadFloat4x4(&mTransform);
	}

	void SceneNode::SetParent(SceneNode* parent)
	{
		mParent = parent;
	}

	void SceneNode::SetTransform(XMFLOAT4X4& transform)
	{
		mTransform = transform;
	}

	void SceneNode::SetTransform(CXMMATRIX transform)
	{
		XMFLOAT4X4 t;
		XMStoreFloat4x4(&t, transform);

		SetTransform(t);
	}

	void SceneNode::Save(std::ofstream& file)
	{
		uint32_t nameSize;

		nameSize = Name().size();
		if (nameSize > 0)
		{
			file.write((char*)&nameSize, sizeof(uint32_t));
			file.write(Name().c_str(), nameSize*sizeof(char));
		}

		float transformCell = 0;
		for (uint32_t i = 0; i < 16; ++i)
		{
			transformCell = mTransform.m[i / 4][i % 4];
			file.write((char*)&transformCell, sizeof(float));
		}

		uint32_t nChildren = Children().size();
		ModelExporter::WriteInteger(nChildren, file);

		if (nChildren > 0)
		{
			for (auto& child : Children())
			{
				bool isBone = child->As<Bone>() != nullptr;
				file.write((char*)&isBone, sizeof(bool));
				child->SceneNode::Save(file);
			}
		}
	}
}
