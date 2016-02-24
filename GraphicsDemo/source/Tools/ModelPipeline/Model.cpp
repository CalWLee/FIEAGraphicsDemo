
#include "stdafx.h"
#include "ModelExporter.h"
#include "Model.h"
#include "Mesh.h"
#include "ModelMaterial.h"
#include "AnimationClip.h"
#include "Bone.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "SceneNode.h"

using namespace Game;

namespace Pipeline
{

	Model::Model(const std::string& filename, bool flipUVs)
        : mMeshes(), mMaterials()
    {
        Assimp::Importer importer;

        UINT flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType | aiProcess_FlipWindingOrder;
        if (flipUVs)
        {
            flags |= aiProcess_FlipUVs;
        }

        const aiScene* scene = importer.ReadFile(filename, flags);
        if (scene == nullptr)
        {
            throw std::exception(importer.GetErrorString());
        }

        if (scene->HasMaterials())
        {
            for (UINT i = 0; i < scene->mNumMaterials; i++)
            {
                mMaterials.push_back(new ModelMaterial(*this, scene->mMaterials[i]));
            }
        }

        if (scene->HasMeshes())
        {
            for (UINT i = 0; i < scene->mNumMeshes; i++)
            {
                ModelMaterial* material = (mMaterials.size() > i ? mMaterials.at(i) : nullptr);

                Mesh* mesh = new Mesh(*this, *(scene->mMeshes[i]));
                mMeshes.push_back(mesh);
            }
        }

		if (scene->HasAnimations())
		{
			assert(scene->mRootNode != nullptr);
			mRootNode = BuildSkeleton(*scene->mRootNode, nullptr);

			mAnimations.reserve(scene->mNumAnimations);
			for (UINT i = 0; i < scene->mNumAnimations; i++)
			{
				AnimationClip* animationClip = new AnimationClip(*this, *(scene->mAnimations[i]));
				mAnimations.push_back(animationClip);
				mAnimationsByName.insert(std::pair<std::string, AnimationClip*>(animationClip->Name(), animationClip));
			}
		}
		else
		{
			mRootNode = nullptr;
		}


		std::ofstream file;
		file.open(filename + ".bin", std::ios::binary);

		Save(file);
    }

	void Model::Save(std::ofstream& file)
	{
		uint32_t nMaterials = Materials().size();
		ModelExporter::WriteInteger(nMaterials, file);
		for (auto& material : Materials())
		{
			material->Save(file);
		}

		uint32_t nBones = mBones.size();
		ModelExporter::WriteInteger(nBones, file);
		for (auto& bone : mBones)
		{
			bone->Save(file);
		}

		uint32_t nMeshes = Meshes().size();
		ModelExporter::WriteInteger(nMeshes, file);

		for (auto& mesh : Meshes())
		{
			mesh->Save(file);
		}

		bool hasRoot = (mRootNode != nullptr);

		file.write((char*)&hasRoot, sizeof(bool));
		if (hasRoot)
		{
			bool f = false;
			file.write((char*)&f, sizeof(bool));
			mRootNode->Save(file);
		}
		
		uint32_t nAnimations = mAnimations.size();
		ModelExporter::WriteInteger(nAnimations, file);

		for (auto& animation : mAnimations)
		{
			animation->Save(file);
		}

		file.close();
	}

    Model::~Model()
    {
        for (Mesh* mesh : mMeshes)
        {
            delete mesh;
        }

        for (ModelMaterial* material : mMaterials)
        {
            delete material;
        }
    }

    bool Model::HasMeshes() const
    {
        return (mMeshes.size() > 0);
    }

    bool Model::HasMaterials() const
    {
        return (mMaterials.size() > 0);
    }

    const std::vector<Mesh*>& Model::Meshes() const
    {
        return mMeshes;
    }

    const std::vector<ModelMaterial*>& Model::Materials() const
    {
        return mMaterials;
    }

	const std::vector<Bone*> Model::Bones() const
	{
		return mBones;
	}
	
	const std::map<std::string, UINT> Model::BoneIndexMapping() const
	{
		return mBoneIndexMapping;
	}

	SceneNode* Model::BuildSkeleton(aiNode& node, SceneNode* parentSceneNode)
	{
		SceneNode* sceneNode = nullptr;

		auto boneMapping = mBoneIndexMapping.find(node.mName.C_Str());

		if (boneMapping == mBoneIndexMapping.end())
		{
			sceneNode = new SceneNode(node.mName.C_Str());
		}
		else
		{
			sceneNode = mBones[boneMapping->second];
		}

		XMMATRIX transform = XMLoadFloat4x4(&(XMFLOAT4X4(reinterpret_cast<const float*>(node.mTransformation[0]))));
		sceneNode->SetTransform(XMMatrixTranspose(transform));
		sceneNode->SetParent(parentSceneNode);

		for (UINT i = 0; i < node.mNumChildren; i++)
		{
			SceneNode* childSceneNode = BuildSkeleton(*(node.mChildren[i]), sceneNode);
			sceneNode->Children().push_back(childSceneNode);
		}

		return sceneNode;
	}

	void Model::ValidateModel()
	{
		// Validate bone weights
		for (Mesh* mesh : mMeshes)
		{
			for (BoneVertexWeights boneWeight : mesh->mBoneWeights)
			{
				float totalWeight = 0.0f;

				for (BoneVertexWeights::VertexWeight vertexWeight : boneWeight.Weights())
				{
					totalWeight += vertexWeight.Weight;
					assert(vertexWeight.BoneIndex >= 0);
					assert(vertexWeight.BoneIndex < mBones.size());
				}

				assert(totalWeight <= 1.05f);
				assert(totalWeight >= 0.95f);
			}
		}
	}
}
