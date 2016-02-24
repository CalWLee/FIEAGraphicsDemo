#pragma once

#include "Serializable.h"

struct aiMaterial;

namespace Pipeline
{
    enum TextureType
    {
        TextureTypeDifffuse = 0,
        TextureTypeSpecularMap,
        TextureTypeAmbient,
        TextureTypeEmissive,
        TextureTypeHeightmap,
        TextureTypeNormalMap,
        TextureTypeSpecularPowerMap,
        TextureTypeDisplacementMap,
        TextureTypeLightMap,
        TextureTypeEnd
    };

    class ModelMaterial : public Library::Serializable
    {
        friend class Model;

    public:
        ModelMaterial(Model& model);
        ~ModelMaterial();

        Model& GetModel();
        const std::string& Name() const;
		const std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>> Textures() const;

	protected:
		void Save(std::ofstream& file) override;

    private:		
        static void InitializeTextureTypeMappings();
        static std::map<TextureType, UINT> sTextureTypeMappings;

        ModelMaterial(Model& model, aiMaterial* material);
        ModelMaterial(const ModelMaterial& rhs);
        ModelMaterial& operator=(const ModelMaterial& rhs);

        Model& mModel;
        std::string mName;
        std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>> mTextures;		
    };
}