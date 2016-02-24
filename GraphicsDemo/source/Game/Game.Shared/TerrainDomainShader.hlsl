cbuffer CBufferPerObject
{
	float4x4 World : WORLD;
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
	float DisplacementScale;
}

cbuffer LightBuffer
{
	float3 LightDirection;
	float3 CameraPosition;
}

struct DS_OUTPUT
{
	float4 Position  : SV_POSITION;
	float2 TextureCoordinate: TEXCOORD;
	float3 Normal : NORMAL;
	float3 LightDirection : LIGHTDIR;
	float3 ViewDirection : VIEWDIR;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinate: TEXCOORD;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeFactors[4]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideFactors[2]			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

Texture2D Heightmap;
SamplerState TrilinearSampler;
#define NUM_CONTROL_POINTS 4

float GetHeight(float2 texCoord)
{
	return (2 * Heightmap.SampleLevel(TrilinearSampler, texCoord, 0).x - 1);
}

[domain("quad")]
DS_OUTPUT main(HS_CONSTANT_DATA_OUTPUT IN, float2 uv : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, 4> patch)
{
	DS_OUTPUT OUT;

	float4 v0 = lerp(patch[0].ObjectPosition, patch[1].ObjectPosition, uv.x);
		float4 v1 = lerp(patch[2].ObjectPosition, patch[3].ObjectPosition, uv.x);
		float4 objectPosition = lerp(v0, v1, uv.y);

		float2 texCoord0 = lerp(patch[0].TextureCoordinate, patch[1].TextureCoordinate, uv.x);
		float2 texCoord1 = lerp(patch[2].TextureCoordinate, patch[3].TextureCoordinate, uv.x);
		float2 textureCoordinate = lerp(texCoord0, texCoord1, uv.y);

		objectPosition.y = GetHeight(textureCoordinate) * DisplacementScale;

	//generating normals by getting the heights of the surrounding pixels.
	float normZ1 = GetHeight(textureCoordinate + float2(1, 0)) - GetHeight(textureCoordinate - float2(1, 0));
	float normZ2 = GetHeight(textureCoordinate + float2(0, 1)) - GetHeight(textureCoordinate - float2(0, 1));

	//get the cross products
	OUT.Normal = cross(float3(1, 0, normZ1), float3(0, 1, normZ2));
	
	OUT.Position = mul(float4(objectPosition.xyz, 1.0f), WorldViewProjection);
	OUT.TextureCoordinate = textureCoordinate;

	OUT.LightDirection = normalize(-LightDirection);
	
	float3 worldPosition = mul(objectPosition, World).xyz;
		OUT.ViewDirection = normalize(CameraPosition - worldPosition);

	return OUT;
}
