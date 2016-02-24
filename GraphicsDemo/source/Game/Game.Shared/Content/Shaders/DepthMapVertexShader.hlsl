cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
}

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float2 Depth : TEXCOORD;
};

VS_OUTPUT main(float4 ObjectPosition : POSITION)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(ObjectPosition, WorldViewProjection);
	OUT.Depth = OUT.Position.zw;

	return OUT;
}