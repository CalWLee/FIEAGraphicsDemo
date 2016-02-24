#pragma once

struct aiAnimation;

namespace Pipeline
{
	class Bone;
	class BoneAnimation;

	class AnimationClip : public Library::Serializable
    {
		friend class Model;

    public:        
        ~AnimationClip();
		
		const std::string& Name() const;
		float Duration() const;
		float TicksPerSecond() const;
		const std::vector<BoneAnimation*>& BoneAnimations() const;
		const std::map<Bone*, BoneAnimation*>& BoneAnimationsByBone() const;
		const UINT KeyframeCount() const;

	protected:
		void Save(std::ofstream& file) override;
    private:
		AnimationClip(Model& model, aiAnimation& animation);

		AnimationClip();
        AnimationClip(const AnimationClip& rhs);
        AnimationClip& operator=(const AnimationClip& rhs);

		std::string mName;
		float mDuration;
		float mTicksPerSecond;
		std::vector<BoneAnimation*> mBoneAnimations;
		std::map<Bone*, BoneAnimation*> mBoneAnimationsByBone;
		UINT mKeyframeCount;
    };
}
