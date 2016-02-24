
cbuffer CBufferPerFrame
{
	float4 TessellationEdgeFactors;
	float2 TessellationInsideFactors;
}

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
	float4 ObjectPosition : SV_Position;
	float2 TextureCoordinate: TEXCOORD;
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

#define NUM_CONTROL_POINTS 4

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, 4> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT OUT = (HS_CONSTANT_DATA_OUTPUT)0;

	for (int i = 0; i < 4; i++)
	{
		OUT.EdgeFactors[i] = TessellationEdgeFactors[i];
	}

	OUT.InsideFactors[0] = TessellationInsideFactors[0];
	OUT.InsideFactors[1] = TessellationInsideFactors[1];

	return OUT;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch, 
	uint controlPointID: SV_OutputControlPointID)
{
	HS_CONTROL_POINT_OUTPUT OUT = (HS_CONTROL_POINT_OUTPUT)0;

	OUT.ObjectPosition = patch[controlPointID].ObjectPosition;
	OUT.TextureCoordinate = patch[controlPointID].TextureCoordinate;

	return OUT;
}
