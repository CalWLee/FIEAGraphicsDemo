#pragma once

namespace Library
{
	class GameBase;
    class Mesh;
	class ModelMaterial;
	class AnimationClip;
	class SceneNode;
	class Bone;

    class Model : public Loadable
    {
    public:
		Model(Game::GameBase& game, const std::string& filename, bool flipUVs = false);
        ~Model();

		Game::GameBase& GetGame();
        bool HasMeshes() const;
        bool HasMaterials() const;

        const std::vector<Mesh*>& Meshes() const;
		const std::vector<ModelMaterial*>& Materials() const;
		const std::vector<AnimationClip*>& Animations() const;
		const std::map<std::string, AnimationClip*>& AnimationsbyName() const;
		const std::vector<std::shared_ptr<Bone>> Bones() const;
		const std::map<std::string, UINT> BoneIndexMapping() const;
		SceneNode* RootNode();

		virtual void Load(std::ifstream& file) override;

    private:
        Model(const Model& rhs);
		Model& operator=(const Model& rhs); 
		void ValidateModel();

		Game::GameBase& mGame;
        std::vector<Mesh*> mMeshes;
		std::vector<ModelMaterial*> mMaterials;
		std::vector<AnimationClip*> mAnimations;
		std::map<std::string, AnimationClip*> mAnimationsByName;
		std::vector<std::shared_ptr<Bone>> mBones;
		std::map<std::string, UINT> mBoneIndexMapping;
		std::unique_ptr<SceneNode> mRootNode;
    };
}
