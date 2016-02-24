
#include "stdafx.h"
#include "Mesh.h"
#include "ModelExporter.h"
#include "Model.h"
#include "Bone.h"
#include <assimp/scene.h>

using namespace Game;
namespace Pipeline
{
    Mesh::Mesh(Model& model, aiMesh& mesh)
		: mModel(model), mMaterial(nullptr), mName(mesh.mName.C_Str()), mVertices(), mNormals(), mTangents(), mBiNormals(), mTextureCoordinates(), mVertexColors(), mFaceCount(0), mIndices(), mBoneWeights()
    {
		mMaterial = mModel.Materials().at(mesh.mMaterialIndex);
		mMaterialIndex = mesh.mMaterialIndex;

        // Vertices
        mVertices.reserve(mesh.mNumVertices);
        for (UINT i = 0; i < mesh.mNumVertices; i++)
        {
            mVertices.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mVertices[i])));			
        }

        // Normals
        if (mesh.HasNormals())
        {
            mNormals.reserve(mesh.mNumVertices);
            for (UINT i = 0; i < mesh.mNumVertices; i++)
            {
                mNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mNormals[i])));
            }
        }

        // Tangents and Binormals
        if (mesh.HasTangentsAndBitangents())
        {
            mTangents.reserve(mesh.mNumVertices);
            mBiNormals.reserve(mesh.mNumVertices);
            for (UINT i = 0; i < mesh.mNumVertices; i++)
            {
                mTangents.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mTangents[i])));
                mBiNormals.push_back(XMFLOAT3(reinterpret_cast<const float*>(&mesh.mBitangents[i])));
            }
        }

        // Texture Coordinates
        UINT uvChannelCount = mesh.GetNumUVChannels();
        for (UINT i = 0; i < uvChannelCount; i++)
        {
            std::vector<XMFLOAT3>* textureCoordinates = new std::vector<XMFLOAT3>();
            textureCoordinates->reserve(mesh.mNumVertices);
            mTextureCoordinates.push_back(textureCoordinates);

            aiVector3D* aiTextureCoordinates = mesh.mTextureCoords[i];
            for (UINT j = 0; j < mesh.mNumVertices; j++)
            {
                textureCoordinates->push_back(XMFLOAT3(reinterpret_cast<const float*>(&aiTextureCoordinates[j])));
            }
        }

        // Vertex Colors
        UINT colorChannelCount = mesh.GetNumColorChannels();
        for (UINT i = 0; i < colorChannelCount; i++)
        {
            std::vector<XMFLOAT4>* vertexColors = new std::vector<XMFLOAT4>();
            vertexColors->reserve(mesh.mNumVertices);
            mVertexColors.push_back(vertexColors);

            aiColor4D* aiVertexColors = mesh.mColors[i];
            for (UINT j = 0; j < mesh.mNumVertices; j++)
            {
                vertexColors->push_back(XMFLOAT4(reinterpret_cast<const float*>(&aiVertexColors[j])));
            }
        }

        // Faces (note: could pre-reserve if we limit primitive types)
        if (mesh.HasFaces())
        {
            mFaceCount = mesh.mNumFaces;
            for (UINT i = 0; i < mFaceCount; i++)
            {
                aiFace* face = &mesh.mFaces[i];
            
                for (UINT j = 0; j < face->mNumIndices; j++)
                {		
                    mIndices.push_back(face->mIndices[j]);
                }
            }
        }

		// Bones
		if (mesh.HasBones())
		{
			mBoneWeights.resize(mesh.mNumVertices);

			mBoneCount = mesh.mNumBones;
			for (UINT i = 0; i < mesh.mNumBones; i++)
			{
				aiBone* meshBone = mesh.mBones[i];

				// Look up the bone in the model's hierarchy, or add it if not found.
				UINT boneIndex = 0U;
				std::string boneName = meshBone->mName.C_Str();
				auto boneMappingIterator = mModel.mBoneIndexMapping.find(boneName);
				if (boneMappingIterator != mModel.mBoneIndexMapping.end())
				{
					boneIndex = boneMappingIterator->second;
				}
				else
				{
					boneIndex = mModel.mBones.size();
					XMMATRIX offsetMatrix = XMLoadFloat4x4(&(XMFLOAT4X4(reinterpret_cast<const float*>(meshBone->mOffsetMatrix[0]))));
					XMFLOAT4X4 offset;
					XMStoreFloat4x4(&offset, XMMatrixTranspose(offsetMatrix));

					Bone* modelBone = new Bone(boneName, boneIndex, offset);
					mModel.mBones.push_back(modelBone);
					mModel.mBoneIndexMapping[boneName] = boneIndex;
				}

				for (UINT i = 0; i < meshBone->mNumWeights; i++)
				{
					aiVertexWeight vertexWeight = meshBone->mWeights[i];
					mBoneWeights[vertexWeight.mVertexId].AddWeight(vertexWeight.mWeight, boneIndex, vertexWeight.mVertexId);
				}
			}
		}
    }
	void Mesh::Save(std::ofstream& file)
	{
		uint32_t nameSize = Name().size();
		ModelExporter::WriteInteger(nameSize, file);

		if (nameSize > 0)
		{
			file.write(Name().c_str(), Name().size()*sizeof(char));
		}

		//Material index.
		uint32_t matIndex = GetMaterialIndex();
		ModelExporter::WriteInteger(matIndex, file);

		//Vertices
		uint32_t nVertices = Vertices().size();
		ModelExporter::WriteInteger(nVertices, file);

		for (auto& vertex : Vertices())
		{
			file.write((char*)&vertex.x, sizeof(float));
			file.write((char*)&vertex.y, sizeof(float));
			file.write((char*)&vertex.z, sizeof(float));
		}

		//Normals
		uint32_t nNormals = Normals().size();
		file.write((char*)&nNormals, sizeof(uint32_t));
		for (auto& normal : Normals())
		{
			file.write((char*)&normal.x, sizeof(float));
			file.write((char*)&normal.y, sizeof(float));
			file.write((char*)&normal.z, sizeof(float));
		}
		

		if (Tangents().size() > 0 && BiNormals().size() > 0)
		{
			// Tangents and Binormals
			ModelExporter::WriteInteger(nVertices, file);
			for (uint32_t i = 0; i < nVertices; ++i)
			{
				file.write((char*)&Tangents()[i].x, sizeof(float));
				file.write((char*)&Tangents()[i].y, sizeof(float));
				file.write((char*)&Tangents()[i].z, sizeof(float));

				file.write((char*)&BiNormals()[i].x, sizeof(float));
				file.write((char*)&BiNormals()[i].y, sizeof(float));
				file.write((char*)&BiNormals()[i].z, sizeof(float));
			}
		}

		else
		{
			uint32_t zero = 0;
			file.write((char*)&zero, sizeof(uint32_t));
		}

		// Texture Coordinates
		uint32_t nCoordinates = TextureCoordinates().size();
		file.write((char*)&nCoordinates, sizeof(uint32_t));
		for (auto& coordinate : TextureCoordinates())
		{
			for (auto& coordinateVector : *coordinate)
			{
				file.write((char*)&coordinateVector.x, sizeof(float));
				file.write((char*)&coordinateVector.y, sizeof(float));
				file.write((char*)&coordinateVector.z, sizeof(float));
			}
		}


		// Vertex Colors
		uint32_t nColors = VertexColors().size();
		ModelExporter::WriteInteger(nColors, file);
		for (auto& color : VertexColors())
		{
			for (auto& c : *color)
			{
				file.write((char*)&c.x, sizeof(float));
				file.write((char*)&c.y, sizeof(float));
				file.write((char*)&c.z, sizeof(float));
				file.write((char*)&c.w, sizeof(float));
			}
		}

		// Faces (note: could pre-reserve if we limit primitive types)
		uint32_t faces = FaceCount();
		ModelExporter::WriteInteger(faces, file);
		for (uint32_t i = 0; i < 1; ++i)
		{
			uint32_t nIndices = Indices().size();
			file.write((char*)&nIndices, sizeof(uint32_t));
			for (auto& index : Indices())
			{
				file.write((char*)&index, sizeof(UINT));
			}
		}

		//Bones
		uint32_t bones = mBoneWeights.size();
		ModelExporter::WriteInteger(bones, file);

		if (mBoneWeights.size() > 0)
		{
			uint32_t nWeights;
			for (auto& boneWeight : mBoneWeights)
			{
				nWeights = boneWeight.Weights().size();
				ModelExporter::WriteInteger(nWeights, file);

				if (nWeights > 0)
				{
					float weight;
					uint32_t index, vid;
					for (uint32_t i = 0; i < nWeights; ++i)
					{
						weight = boneWeight.Weights()[i].Weight;
						file.write((char*)&weight, sizeof(float));

						index = boneWeight.Weights()[i].BoneIndex;
						ModelExporter::WriteInteger(index, file);

						vid = boneWeight.Weights()[i].VertexID;
						ModelExporter::WriteInteger(vid, file);
					}
				}
			}
		}
	}

	UINT Mesh::GetMaterialIndex() const
	{
		return mMaterialIndex;
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
}