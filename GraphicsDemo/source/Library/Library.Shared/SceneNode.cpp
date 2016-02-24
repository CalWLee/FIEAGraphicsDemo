#include "pch.h"
#include "SceneNode.h"
#include "MatrixHelper.h"
#include "Bone.h"

namespace Library
{
	RTTI_DEFINITIONS(SceneNode);
	
	SceneNode::SceneNode(Model& model)
		: mName(), mParent(nullptr), mChildren(), mTransform(MatrixHelper::Identity), mModel(&model)
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

	std::vector<std::shared_ptr<SceneNode>>& SceneNode::Children()
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

	void SceneNode::Load(std::ifstream& file)
	{
		LoadHelper(file, nullptr);
	}


	void SceneNode::LoadHelper(std::ifstream& file, SceneNode* parent)
	{
		int nameSize = 0;

		/*
		std::string parentName;
		file.read((char*)&nameSize, sizeof(uint32_t));
		file.read(const_cast<char*>(parentName.c_str()), nameSize*sizeof(char));
		*/

		if (parent != nullptr)
		{
			SetParent(parent);
		}

		file.read((char*)&nameSize, sizeof(uint32_t));
		ReadString(file, nameSize, mName);

		float transformCell = 0;
		for (uint32_t i = 0; i < 16; ++i)
		{
			file.read((char*)&mTransform.m[i / 4][i % 4], sizeof(float));
		}

		uint32_t nChildren = 0 ;
		file.read((char*)&nChildren, sizeof(uint32_t));


		for (uint32_t i = 0; i < nChildren; ++i)
		{
			bool isBone = false;
			file.read((char*)&isBone, sizeof(bool));

			SceneNode *childSceneNode;
			if (!isBone)
			{
				std::shared_ptr<SceneNode> node = std::shared_ptr<SceneNode>(new SceneNode(*mModel));
				Children().push_back(node);

				childSceneNode = node.get();
			}
			else
			{
				
				nameSize = 0;
				file.read((char*)&nameSize, sizeof(uint32_t));

				std::string boneName;

				ReadString(file, nameSize, boneName);
				auto boneMapping = mModel->BoneIndexMapping().find(boneName)->second;
				auto bone = mModel->Bones()[boneMapping];
				Children().push_back(bone);

				childSceneNode = mModel->Bones()[boneMapping].get();
			}

			childSceneNode->LoadHelper(file, this);
		}
		
	}
}
