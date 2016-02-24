#pragma once
#include <RTTI.h>
using namespace Library;
namespace Pipeline
{
	class SceneNode : public RTTI, public Serializable
	{
		friend class Model;
		RTTI_DECLARATIONS(SceneNode, RTTI);

	public:	
		const std::string& Name() const;
		SceneNode* GetParent();
		std::vector<SceneNode*>& Children();
		const XMFLOAT4X4& Transform() const;
		XMMATRIX TransformMatrix() const;

		void SetParent(SceneNode* parent);

		void SetTransform(XMFLOAT4X4& transform);
		void SetTransform(CXMMATRIX transform);

		SceneNode(const std::string& name);
		SceneNode(const std::string& name, const XMFLOAT4X4& transform);

	protected:
		std::string mName;
		SceneNode* mParent;
		std::vector<SceneNode*> mChildren;
		XMFLOAT4X4 mTransform;
		void Save(std::ofstream& file) override;

	private:
		SceneNode();
		SceneNode(const SceneNode& rhs);
		SceneNode& operator=(const SceneNode& rhs);
	};
}
