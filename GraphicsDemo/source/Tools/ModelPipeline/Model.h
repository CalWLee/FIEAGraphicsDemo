#pragma once
#include "Serializable.h"
struct aiNode;
namespace Pipeline
{
    class Mesh;
	class ModelMaterial;
	class AnimationClip;
	class SceneNode;
	class Bone;

    class Model final : public Library::Serializable
	{
		friend class Mesh;
    public:
		Model(const std::string& filename, bool flipUVs = false);
        ~Model();

        bool HasMeshes() const;
        bool HasMaterials() const;

        const std::vector<Mesh*>& Meshes() const;
		const std::vector<ModelMaterial*>& Materials() const;
		const std::vector<AnimationClip*>& Animations() const;
		const std::map<std::string, AnimationClip*>& AnimationsbyName() const;
		const std::vector<Bone*> Bones() const;
		const std::map<std::string, UINT> BoneIndexMapping() const;
		SceneNode* RootNode();
	protected:
		void Save(std::ofstream& file) override;
	private:
        Model(const Model& rhs);
		Model& operator=(const Model& rhs);

		SceneNode* BuildSkeleton(aiNode& node, SceneNode* parentSceneNode);
		void ValidateModel();
		void DeleteSceneNode(SceneNode* sceneNode);

        std::vector<Mesh*> mMeshes;
		std::vector<ModelMaterial*> mMaterials;
		std::vector<AnimationClip*> mAnimations;
		std::map<std::string, AnimationClip*> mAnimationsByName;
		std::vector<Bone*> mBones;
		std::map<std::string, UINT> mBoneIndexMapping;
		SceneNode* mRootNode;
    };
}
