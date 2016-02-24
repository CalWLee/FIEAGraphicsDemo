struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float2 TextureCoordinate : TEXCOORD;
};

struct VS_CONTROL_POINT_OUTPUT
{
	float4 Position: SV_Position;
	float2 TextureCoordinate : TEXCOORD;
};

VS_CONTROL_POINT_OUTPUT main(VS_INPUT IN)
{
	VS_CONTROL_POINT_OUTPUT OUT = (VS_CONTROL_POINT_OUTPUT)0;

	OUT.Position = (IN.ObjectPosition);
	OUT.TextureCoordinate = IN.TextureCoordinate;

	return OUT;
}
