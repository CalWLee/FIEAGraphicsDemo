#pragma once

namespace Library
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

    class ModelMaterial : public Loadable
    {
        friend class Model;

    public:
        ModelMaterial(Model& model);
        ~ModelMaterial();

        Model& GetModel();
        const std::string& Name() const;
		const std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>>& Textures() const;
		const std::vector<TextureType>& TextureTypes() const;

		virtual void Load(std::ifstream& file) override;

    private:		
		static void InitializeTextureTypeMappings();
		static std::map<TextureType, UINT> sTextureTypeMappings;

        ModelMaterial(const ModelMaterial& rhs);
        ModelMaterial& operator=(const ModelMaterial& rhs);

        Model& mModel;
        std::string mName;
        std::map<TextureType, std::shared_ptr<std::vector<std::wstring>>> mTextures;		
		std::vector<TextureType> mTypes;
    };
}