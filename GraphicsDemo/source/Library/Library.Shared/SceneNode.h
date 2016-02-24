#pragma once

#include "Loadable.h"
#include <RTTI.h>

namespace Library
{
	class SceneNode : public RTTI, public Loadable
	{
		RTTI_DECLARATIONS(SceneNode, RTTI);
		friend class Model;
	public:	
		const std::string& Name() const;
		SceneNode* GetParent();
		std::vector<std::shared_ptr<SceneNode>>& Children();
		const XMFLOAT4X4& Transform() const;
		XMMATRIX TransformMatrix() const;

		void SetParent(SceneNode* parent);

		void SetTransform(XMFLOAT4X4& transform);
		void SetTransform(CXMMATRIX transform);

		SceneNode(Model& model);

		virtual void Load(std::ifstream& file) override;
		virtual void LoadHelper(std::ifstream& file, SceneNode* parent);
	protected:
		std::string mName;
		SceneNode* mParent;
		std::vector<std::shared_ptr<SceneNode>> mChildren;
		XMFLOAT4X4 mTransform;
		Model* mModel;
	private:

		SceneNode() = default;
		SceneNode(const SceneNode& rhs);
		SceneNode& operator=(const SceneNode& rhs);
	};
}
