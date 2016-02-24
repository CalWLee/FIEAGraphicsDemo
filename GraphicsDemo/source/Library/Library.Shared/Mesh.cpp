#include "pch.h"
#include "Mesh.h"
#include "Bone.h"
#include "Model.h"
#include "GameBase.h"
//#include "GameException.h"

using namespace Game;
namespace Library
{
    Mesh::Mesh(Model& model)
		: mModel(model), mMaterial(nullptr), mName(""), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices(), mBoneWeights()
    {
    }

	void Mesh::Load(std::ifstream& file)
	{
		uint32_t nameSize;
		file.read((char*)&nameSize, sizeof(uint32_t));

		char* name = const_cast<char*>(mName.c_str());
		if (nameSize > 0)
		{
			file.read(name, nameSize*sizeof(char));
		}
		//Material index.
		uint32_t matIndex;
		file.read((char*)&matIndex, sizeof(uint32_t));

		mMaterial = mModel.Materials().at(matIndex);

		//Vertices
		uint32_t nVertices;
		file.read((char*)&nVertices, sizeof(uint32_t));
		mVertices.reserve(nVertices);
		
		for (uint32_t i = 0; i < nVertices; ++i)
		{
			XMFLOAT3 vertex;

			file.read((char*)&vertex.x, sizeof(float));
			file.read((char*)&vertex.y, sizeof(float));
			file.read((char*)&vertex.z, sizeof(float));

			mVertices.push_back(vertex);
		}

		
		//Normals
		uint32_t nNormals;
		file.read((char*)&nNormals, sizeof(uint32_t));
		mNormals.reserve(nVertices);
		for (uint32_t i = 0; i < nNormals; ++i)
		{
			XMFLOAT3 normal;

			file.read((char*)&normal.x, sizeof(float));
			file.read((char*)&normal.y, sizeof(float));
			file.read((char*)&normal.z, sizeof(float));

			mNormals.push_back(normal);
		}

		// Tangents and Binormals
		uint32_t nTangents = 0;
		file.read((char*)&nTangents, sizeof(uint32_t));
		if (nTangents > 0)
		{
			mTangents.reserve(nVertices);
			mBiNormals.reserve(nVertices);
			for (uint32_t i = 0; i < nVertices; ++i)
			{
				XMFLOAT3 tangent, binormal;

				file.read((char*)&tangent.x, sizeof(float));
				file.read((char*)&tangent.y, sizeof(float));
				file.read((char*)&tangent.z, sizeof(float));

				file.read((char*)&binormal.x, sizeof(float));
				file.read((char*)&binormal.y, sizeof(float));
				file.read((char*)&binormal.z, sizeof(float));

				mTangents.push_back(tangent);
				mBiNormals.push_back(binormal);
			}
		}
		// Texture Coordinates
		uint32_t nCoordinates;
		file.read((char*)&nCoordinates, sizeof(uint32_t));
		for (uint32_t i = 0; i < nCoordinates; ++i)
		{
			std::vector<XMFLOAT3>* textureCoordinates = new std::vector<XMFLOAT3>();
			textureCoordinates->reserve(nVertices);
			mTextureCoordinates.push_back(textureCoordinates);

			for (uint32_t j = 0; j < nVertices; ++j)
			{
				XMFLOAT3 coordinate;

				file.read((char*)&coordinate.x, sizeof(float));
				file.read((char*)&coordinate.y, sizeof(float));
				file.read((char*)&coordinate.z, sizeof(float));

				textureCoordinates->push_back(coordinate);
			}
		}


		// Vertex Colors
		uint32_t nColors;
		file.read((char*)&nColors, sizeof(uint32_t));
		for (uint32_t i = 0; i < nColors; ++i)
		{
			std::vector<XMFLOAT4>* vertexColors = new std::vector<XMFLOAT4>();
			vertexColors->reserve(nVertices);
			mVertexColors.push_back(vertexColors);

			for (uint32_t i = 0; i < nVertices; ++i)
			{
				XMFLOAT4 color;
				file.read((char*)&color.x, sizeof(float));
				file.read((char*)&color.y, sizeof(float));
				file.read((char*)&color.z, sizeof(float));
				file.read((char*)&color.w, sizeof(float));

				vertexColors->push_back(color);
			}
		}

		// Faces (note: could pre-reserve if we limit primitive types)
		file.read((char*)&mFaceCount, sizeof(uint32_t));
		uint32_t nIndices;
		file.read((char*)&nIndices, sizeof(uint32_t));

		for (uint32_t j = 0; j < nIndices; ++j)
		{
			uint32_t index;
			file.read((char*)&index, sizeof(uint32_t));

			mIndices.push_back(index);
		}

		//Bones
		file.read((char*)&mBoneCount, sizeof(uint32_t));


		if (mBoneCount > 0)
		{
			mBoneWeights.resize(nVertices);

			uint32_t nWeights;
			for (uint32_t i = 0; i < mBoneCount; ++i)
			{
				nWeights = 0;
				file.read((char*)&nWeights, sizeof(uint32_t));

				float weight;
				uint32_t index, vid;
				for (uint32_t i = 0; i < nWeights; ++i)
				{
					weight=0;
					file.read((char*)&weight, sizeof(float));

					index=0;
					file.read((char*)&index, sizeof(uint32_t));

					vid = 0;
					file.read((char*)&vid, sizeof(uint32_t));

					mBoneWeights[vid].AddWeight(weight, index);
				}				
			}
		}
	}

    Mesh::~Mesh()
    {
        for (std::vector<XMFLOAT3>* textureCoordinates : mTextureCoordinates)
        {
            delete textureCoordinates;
        }

        for (std::vector<XMFLOAT4>* vertexColors : mVertexColors)
        {
            delete vertexColors;
        }
    }

    Model& Mesh::GetModel()
    {
        return mModel;
    }

    ModelMaterial* Mesh::GetMaterial()
    {
        return mMaterial;
    }

    const std::string& Mesh::Name() const
    {
        return mName;
    }

    const std::vector<XMFLOAT3>& Mesh::Vertices() const
    {
        return mVertices;
    }

    const std::vector<XMFLOAT3>& Mesh::Normals() const
    {
        return mNormals;
    }

    const std::vector<XMFLOAT3>& Mesh::Tangents() const
    {
        return mTangents;
    }

    const std::vector<XMFLOAT3>& Mesh::BiNormals() const
    {
        return mBiNormals;
    }

    const std::vector<std::vector<XMFLOAT3>*>& Mesh::TextureCoordinates() const
    {
        return mTextureCoordinates;
    }

    const std::vector<std::vector<XMFLOAT4>*>& Mesh::VertexColors() const
    {
        return mVertexColors;
    }

    UINT Mesh::FaceCount() const
    {
        return mFaceCount;
    }

    const std::vector<UINT>& Mesh::Indices() const
    {
        return mIndices;
	}

	const std::vector<BoneVertexWeights>& Mesh::BoneWeights() const
	{
		return mBoneWeights;
	}

    void Mesh::CreateIndexBuffer(ID3D11Buffer** indexBuffer)
    {
        assert(indexBuffer != nullptr);

        D3D11_BUFFER_DESC indexBufferDesc;
        ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));
		indexBufferDesc.ByteWidth = sizeof(UINT) * static_cast<UINT>(mIndices.size());
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;		
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA indexSubResourceData;
        ZeroMemory(&indexSubResourceData, sizeof(indexSubResourceData));
        indexSubResourceData.pSysMem = &mIndices[0];
		Utility::ThrowIfFailed(mModel.GetGame().GetD3DDevice()->CreateBuffer(&indexBufferDesc, &indexSubResourceData, indexBuffer), "ID3D11Device::CreateBuffer() failed.");
    }
}