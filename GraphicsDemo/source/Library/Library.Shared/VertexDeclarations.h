#pragma once

#include "pch.h"

namespace Library
{
	struct VertexPosition
    {
        XMFLOAT4 Position;

        VertexPosition() { }

		VertexPosition(const XMFLOAT4& position)
            : Position(position) { }
	};

	struct VertexPositionTexture
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;

		VertexPositionTexture() { }

		VertexPositionTexture(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates)
			: Position(position), TextureCoordinates(textureCoordinates) { }
	};

	struct VertexPositionSize
	{
		XMFLOAT4 Position;
		XMFLOAT2 Size;

		VertexPositionSize() { }

		VertexPositionSize(const XMFLOAT4& position, const XMFLOAT2& size)
			: Position(position), Size(size) { }
	};

	struct VertexPositionNormal
    {
        XMFLOAT4 Position;
        XMFLOAT3 Normal;

        VertexPositionNormal() { }

		VertexPositionNormal(const XMFLOAT4& position, const XMFLOAT3& normal)
            : Position(position), Normal(normal) { }
    };

	struct VertexPositionTextureNormal
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;

		VertexPositionTextureNormal() { }

		VertexPositionTextureNormal(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates, const XMFLOAT3& normal)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal) { }
	};

	struct VertexPositionTextureNormalTangent
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;
		XMFLOAT3 Tangent;

		VertexPositionTextureNormalTangent() { }

		VertexPositionTextureNormalTangent(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates, const XMFLOAT3& normal, const XMFLOAT3& tangent)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal), Tangent(tangent) { }
	}; 
	
	struct VertexSkinnedPositionTextureNormal
	{
		XMFLOAT4 Position;
		XMFLOAT2 TextureCoordinates;
		XMFLOAT3 Normal;
		XMUINT4 BoneIndices;
		XMFLOAT4 BoneWeights;

		VertexSkinnedPositionTextureNormal() { }

		VertexSkinnedPositionTextureNormal(const XMFLOAT4& position, const XMFLOAT2& textureCoordinates, const XMFLOAT3& normal, const XMUINT4& boneIndices, const XMFLOAT4& boneWeights)
			: Position(position), TextureCoordinates(textureCoordinates), Normal(normal), BoneIndices(boneIndices), BoneWeights(boneWeights) { }
	};
}
