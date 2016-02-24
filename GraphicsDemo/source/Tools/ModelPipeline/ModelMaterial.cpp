
#include "stdafx.h"
#include "ModelExporter.h"
#include "ModelMaterial.h"
#include "Utility.h"
#include <assimp/scene.h>


using namespace Game;
namespace Pipeline
{
    std::map<TextureType, UINT> ModelMaterial::sTextureTypeMappings;

    ModelMaterial::ModelMaterial(Model& model)
        : mModel(model), mTextures()
    {
        InitializeTextureTypeMappings();
    }

    ModelMaterial::ModelMaterial(Model& model, aiMaterial* material)
        : mModel(model), mTextures()
    {
        InitializeTextureTypeMappings();

        aiString name;
        material->Get(AI_MATKEY_NAME, name);
        mName = name.C_Str();

        for (TextureType textureType = (TextureType)0; textureType < TextureTypeEnd; textureType = (TextureType)(textureType + 1))
        {
            aiTextureType mappedTextureType = (aiTextureType)sTextureTypeMappings[textureType];

            UINT textureCount = material->GetTextureCount(mappedTextureType);
            if (textureCount > 0)
            {
				std::shared_ptr<std::vector<std::wstring>> textures = std::shared_ptr<std::vector<std::wstring>>(new std::vector<std::wstring>());
				mTextures.insert(std::pair<TextureType, std::shared_ptr<std::vector<std::wstring>>>(textureType, textures));

                textures->reserve(textureCount);
                for (UINT textureIndex = 0; textureIndex < textureCount; textureIndex++)
                {
                    aiString path;
                    if (material->GetTexture(mappedTextureType, textureIndex, &path) == AI_SUCCESS)
                    {
                        std::wstring wPath;
                        Library::Utility::ToWideString(path.C_Str(), wPath);

                        textures->push_back(wPath);
                    }
                }
            }
        }
    }

    ModelMaterial::~ModelMaterial()
    {

    }

	void ModelMaterial::Save(std::ofstream& file)
	{
		uint32_t nameSize = Name().size();
		ModelExporter::WriteInteger(nameSize, file);
		file.write(mName.c_str(), nameSize*sizeof(char));

		uint32_t nTextures = Textures().size();
		ModelExporter::WriteInteger(nTextures, file);
		for (auto& texture : Textures())
		{
			uint32_t textureType = texture.first;
			ModelExporter::WriteInteger(textureType, file);

			uint32_t textureCount = texture.second->size();
			ModelExporter::WriteInteger(textureCount, file);
			if (textureCount > 0)
			{
				for (auto& wideString : *texture.second)
				{
					uint32_t textureNameSize = wideString.size();
					ModelExporter::WriteInteger(textureNameSize, file);

					if (textureNameSize > 0)
					{
						for (uint32_t i = 0; i < textureNameSize; ++i)
						{
							file.write((char*)&wideString.at(i), sizeof(wchar_t));
						}

						//file.write(reinterpret_cast<char*>(&wideString), wideString.size()*sizeof(char));
					}
				}
			}
		}
	}

    Model& ModelMaterial::GetModel()
    {
        return mModel;
    }

    const std::string& ModelMaterial::Name() const
    {
        return mName;
    }

	const std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>> ModelMaterial::Textures() const
    {
        return mTextures;
    }

    void ModelMaterial::InitializeTextureTypeMappings()
    {
        if (sTextureTypeMappings.size() != TextureTypeEnd)
        {
            sTextureTypeMappings[TextureTypeDifffuse] = aiTextureType_DIFFUSE;
            sTextureTypeMappings[TextureTypeSpecularMap] = aiTextureType_SPECULAR;
            sTextureTypeMappings[TextureTypeAmbient] = aiTextureType_AMBIENT;
            sTextureTypeMappings[TextureTypeHeightmap] = aiTextureType_HEIGHT;
            sTextureTypeMappings[TextureTypeNormalMap] = aiTextureType_NORMALS;
            sTextureTypeMappings[TextureTypeSpecularPowerMap] = aiTextureType_SHININESS;
            sTextureTypeMappings[TextureTypeDisplacementMap] = aiTextureType_DISPLACEMENT;
            sTextureTypeMappings[TextureTypeLightMap] = aiTextureType_LIGHTMAP;
        }
    }
}