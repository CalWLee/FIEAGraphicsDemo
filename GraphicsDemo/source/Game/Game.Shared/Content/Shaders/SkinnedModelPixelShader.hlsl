cbuffer CBufferPerFrame
{
	float4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	float4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float3 LightPosition = { 0.0f, 0.0f, 0.0f };
	float LightRadius;
	float3 CameraPosition;
}

cbuffer CBufferPerObject
{
	float3 SpecularColor : SPECULAR;
	float SpecularPower : SPECULARPOWER;
}

Texture2D ColorTexture;
SamplerState ColorSampler;
struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float2 TextureCoordinate : TEXCOORD0;
	float3 WorldPosition : TEXCOORD1;
	float Attenuation : TEXCOORD2;
};

float4 main(VS_OUTPUT IN) : SV_Target
{
	float4 OUT = (float4)0;

	float3 lightDirection = LightPosition - IN.WorldPosition;
	lightDirection = normalize(lightDirection);

	float3 viewDirection = normalize(CameraPosition - IN.WorldPosition);

		float3 normal = normalize(IN.Normal);
		float n_dot_l = dot(normal, lightDirection);
	float3 halfVector = normalize(lightDirection + viewDirection);
		float n_dot_h = dot(normal, halfVector);

	float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
		float4 lightCoefficients = lit(n_dot_l, n_dot_h, SpecularPower);

		float3 ambient = AmbientColor.rgb * AmbientColor.a* color.rgb;
		float3 diffuse = LightColor.rgb *LightColor.a * lightCoefficients.y * color.rgb * IN.Attenuation;
		float3 specular = SpecularColor * min(lightCoefficients.z, color.w) * IN.Attenuation;

		OUT.rgb = ambient + diffuse + specular;
	OUT.a = 1.0f;

	return OUT;
}