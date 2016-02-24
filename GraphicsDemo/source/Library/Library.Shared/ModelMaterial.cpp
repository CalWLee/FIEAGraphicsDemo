
#include "pch.h"
#include "ModelMaterial.h"
#include "Utility.h"

using namespace Game;
namespace Library
{
	std::map<TextureType, UINT> ModelMaterial::sTextureTypeMappings;

	ModelMaterial::ModelMaterial(Model& model)
		: mModel(model), mTextures(), mName("")
    {
		InitializeTextureTypeMappings();
    }

      ModelMaterial::~ModelMaterial()
    {

    }



	void ModelMaterial::Load(std::ifstream& file)
	{
		wchar_t c;
		uint32_t nameSize;
		file.read((char*)&nameSize, sizeof(uint32_t));

		if (nameSize > 0)
		{
			char* temp = const_cast<char*>(mName.c_str());
			file.read(temp, (nameSize)*sizeof(char));
		}

		uint32_t nTextures;
		file.read((char*)&nTextures, sizeof(uint32_t));

		for (uint32_t i = 0; i < nTextures; ++i)
		{
			uint32_t textureType;
			file.read((char*)&textureType, sizeof(uint32_t));

			mTypes.push_back((TextureType)textureType);

			uint32_t textureCount;
			file.read((char*)&textureCount, sizeof(uint32_t));

			if (textureCount > 0)
			{
				std::shared_ptr<std::vector<std::wstring>> textures = std::shared_ptr<std::vector<std::wstring>>(new std::vector<std::wstring>());
				mTextures.insert(std::pair<TextureType, std::shared_ptr<std::vector<std::wstring>>>((TextureType)textureType, textures));


				for (uint32_t j = 0; j < textureCount; ++j)
				{
					uint32_t textureNameSize = 0;

					file.read((char*)&textureNameSize, sizeof(uint32_t));

					if (textureNameSize > 0)
					{
						std::vector<wchar_t> pathVect;
						for (uint32_t i = 0; i < textureNameSize; ++i)
						{
							file.read((char*)&c, 2);
							pathVect.push_back(c);
						}		
						std::reverse(pathVect.begin(), pathVect.end());
						std::wstring path(pathVect.begin(), pathVect.end());
						textures->push_back(path);
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

	const std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>>& ModelMaterial::Textures() const
    {
        return mTextures;
	}
	const std::vector<TextureType>& ModelMaterial::TextureTypes() const
	{
		return mTypes;
	}

	void ModelMaterial::InitializeTextureTypeMappings()
	{
		if (sTextureTypeMappings.size() != TextureTypeEnd)
		{
			sTextureTypeMappings[TextureTypeDifffuse] = TextureTypeDifffuse + 1;
			sTextureTypeMappings[TextureTypeSpecularMap] = TextureTypeSpecularMap + 1;
			sTextureTypeMappings[TextureTypeAmbient] = TextureTypeAmbient + 1;
			sTextureTypeMappings[TextureTypeHeightmap] = TextureTypeHeightmap + 1;
			sTextureTypeMappings[TextureTypeNormalMap] = TextureTypeNormalMap + 1;
			sTextureTypeMappings[TextureTypeSpecularPowerMap] = TextureTypeSpecularPowerMap +1;
			sTextureTypeMappings[TextureTypeDisplacementMap] = TextureTypeDisplacementMap + 1;
			sTextureTypeMappings[TextureTypeLightMap] = TextureTypeLightMap + 1;
		}
	}
}