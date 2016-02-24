#pragma once

#include "Serializable.h"
using namespace DirectX;
struct aiMesh;

namespace Pipeline
{
    class Material;
	class ModelMaterial;
	class BoneVertexWeights;

	class Mesh : public Library::Serializable
    {
        friend class Model;

    public:
        Mesh(Model& model, ModelMaterial* material);
        ~Mesh();

        Model& GetModel();
        ModelMaterial* GetMaterial();
		UINT GetMaterialIndex() const;
        const std::string& Name() const;

        const std::vector<XMFLOAT3>& Vertices() const;
        const std::vector<XMFLOAT3>& Normals() const;
        const std::vector<XMFLOAT3>& Tangents() const;
        const std::vector<XMFLOAT3>& BiNormals() const;
        const std::vector<std::vector<XMFLOAT3>*>& TextureCoordinates() const;
        const std::vector<std::vector<XMFLOAT4>*>& VertexColors() const;
        UINT FaceCount() const;
        const std::vector<UINT>& Indices() const;
		const std::vector<BoneVertexWeights>& BoneWeights() const;

	protected:
		void Save(std::ofstream& file) override;
    private:
        Mesh(Model& model, aiMesh& mesh);
        Mesh(const Mesh& rhs);
        Mesh& operator=(const Mesh& rhs);

        Model& mModel;
        ModelMaterial* mMaterial;
		UINT mMaterialIndex;
        std::string mName;
        std::vector<XMFLOAT3> mVertices;
        std::vector<XMFLOAT3> mNormals;
        std::vector<XMFLOAT3> mTangents;
        std::vector<XMFLOAT3> mBiNormals;
        std::vector<std::vector<XMFLOAT3>*> mTextureCoordinates;
        std::vector<std::vector<XMFLOAT4>*> mVertexColors;
        UINT mFaceCount;
		UINT mBoneCount;
        std::vector<UINT> mIndices;
		std::vector<BoneVertexWeights> mBoneWeights;

	};
}