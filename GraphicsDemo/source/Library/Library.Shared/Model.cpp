#include "pch.h"
#include "Model.h"
#include "GameBase.h"
#include "AnimationClip.h"
#include "Bone.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "ModelMaterial.h"

using namespace Game;

namespace Library
{
	Model::Model(Game::GameBase& game, const std::string& filename, bool flipUVs)
        : mGame(game), mMeshes(), mMaterials()
    {

		std::ifstream file;
		file.open(filename, std::ios::binary);

		Load(file);

		file.close();
    }
	SceneNode* Model::RootNode()
	{
		return mRootNode.get();
	}

	void Model::Load(std::ifstream& file)
	{
		uint32_t nMaterials = 0;
		file.read((char*)&nMaterials, sizeof(uint32_t));
		for (UINT i = 0; i < nMaterials; i++)
		{
			ModelMaterial* newMaterial = new ModelMaterial(*this);
			mMaterials.push_back(newMaterial);

			newMaterial->Load(file);
		}

		uint32_t nBones = 0;
		file.read((char*)&nBones, sizeof(uint32_t));
		for (uint32_t i = 0; i < nBones; ++i)
		{
			mBones.push_back(std::shared_ptr<Bone>(new Bone(*this)));

			Bone& lastBone = *mBones[mBones.size() - 1].get();
			lastBone.Load(file);
			mBoneIndexMapping.insert(std::pair<std::string, UINT>(lastBone.Name(), lastBone.Index()));
		}


		uint32_t nMeshes = 0;
		file.read((char*)&nMeshes, sizeof(uint32_t));

		for (uint32_t i = 0; i < nMeshes; ++i)
		{
			Mesh* mesh = new Mesh(*this);
			mMeshes.push_back(mesh);

			mesh->Load(file);
		}
		
		bool hasRoot = false;
		file.read((char*)&hasRoot, sizeof(bool));
		if (hasRoot)
		{
			bool f = false;
			file.read((char*)&f, sizeof(bool));

			mRootNode = std::unique_ptr<SceneNode>(new SceneNode(*this));

			mRootNode->Load(file);
		}

		uint32_t nAnimations = 0;
		file.read((char*)&nAnimations, sizeof(uint32_t));

		for (uint32_t i = 0; i < nAnimations; ++i)
		{
			AnimationClip* animation = new AnimationClip(*this);

			animation->Load(file);
			mAnimations.push_back(animation);
		}

#if defined( DEBUG ) || defined( _DEBUG )
		ValidateModel();
#endif
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

		for (AnimationClip* animation : mAnimations)
		{
			delete animation;
		}
    }

    Game::GameBase& Model::GetGame()
    {
        return mGame;
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

	const std::vector<std::shared_ptr<Bone>> Model::Bones() const
	{
		return mBones;
	}

	const std::vector<AnimationClip*>& Model::Animations() const
	{
		return mAnimations;
	}

	const std::map<std::string, UINT> Model::BoneIndexMapping() const
	{
		return mBoneIndexMapping;
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
