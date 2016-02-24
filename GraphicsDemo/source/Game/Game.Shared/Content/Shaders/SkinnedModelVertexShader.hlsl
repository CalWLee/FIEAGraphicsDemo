#define MaxBones 60

cbuffer CBufferPerFrame
{
	float3 LightPosition = { 0.0f, 0.0f, 0.0f };
	float LightRadius = 10.0f;
}

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
	float4x4 World : WORLD;
}

cbuffer CBufferSkinning
{
	float4x4 BoneTransforms[MaxBones];
}

struct VS_INPUT
{
	float4 ObjectPosition : POSITION;
	float2 TextureCoordinate : TEXCOORD;
	float3 Normal : NORMAL;
	uint4 BoneIndices : BONEINDICES;
	float4 BoneWeights : WEIGHTS;
};

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float2 TextureCoordinate : TEXCOORD0;
	float3 WorldPosition : TEXCOORD1;
	float Attenuation : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	float4x4 skinTransform = (float4x4)0;
	skinTransform += BoneTransforms[IN.BoneIndices.x] * IN.BoneWeights.x;
	skinTransform += BoneTransforms[IN.BoneIndices.y] * IN.BoneWeights.y;
	skinTransform += BoneTransforms[IN.BoneIndices.z] * IN.BoneWeights.z;
	skinTransform += BoneTransforms[IN.BoneIndices.w] * IN.BoneWeights.w;

	float4 position = mul(IN.ObjectPosition, skinTransform);
		OUT.Position = mul(position, WorldViewProjection);
	OUT.WorldPosition = mul(position, World).xyz;

	float4 normal = mul(float4(IN.Normal, 0), skinTransform);
		OUT.Normal = normalize(mul(normal, World).xyz);

	OUT.TextureCoordinate = IN.TextureCoordinate;

	float3 lightDirection = LightPosition - OUT.WorldPosition;
		OUT.Attenuation = saturate(1.0f - (length(lightDirection) / LightRadius));

	return OUT;
}