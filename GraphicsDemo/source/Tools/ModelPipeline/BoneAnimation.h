#pragma once
#include "Serializable.h"
struct aiNodeAnim;
using namespace Library;
namespace Pipeline
{
	class Model;
	class Bone;
	class Keyframe;

    class BoneAnimation : public Serializable
    {
		friend class AnimationClip;

    public:        
        ~BoneAnimation();
		
		Bone& GetBone();
		const std::vector<Keyframe*> Keyframes() const;	

	protected:
		void Save(std::ofstream& file) override;
    private:
		BoneAnimation(Model& model, aiNodeAnim& nodeAnim);

		BoneAnimation();
        BoneAnimation(const BoneAnimation& rhs);
        BoneAnimation& operator=(const BoneAnimation& rhs);

		UINT FindKeyframeIndex(float time) const;

		Model* mModel;
		Bone* mBone;
		std::vector<Keyframe*> mKeyframes;
    };
}
