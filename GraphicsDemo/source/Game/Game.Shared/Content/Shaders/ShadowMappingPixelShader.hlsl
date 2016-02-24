
/************* Resources *************/
static const float4 ColorWhite = { 1, 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.005;

cbuffer CBufferPerFrame
{
	float4 AmbientColor = { 1.0f, 1.0f, 1.0f, 0.0f };
	float4 LightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	float3 LightPosition = { 0.0f, 0.0f, 0.0f };
	float LightRadius = 10.0f;
	float3 CameraPosition;
	float2 ShadowMapSize = { 1024.0f, 1024.0f };
}

cbuffer CBufferPerObject
{
	float4 SpecularColor : SPECULAR = { 1.0f, 1.0f, 1.0f, 1.0f };
	float SpecularPower : SPECULARPOWER = 25.0f;
}

Texture2D ColorTexture;
Texture2D ShadowMap;

SamplerState ColorSampler;
SamplerState ShadowMapSampler;
SamplerComparisonState PcfShadowMapSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float3 Normal : NORMAL;
	float2 TextureCoordinate : TEXCOORD0;
	float3 WorldPosition : TEXCOORD1;
	float Attenuation : TEXCOORD2;
	float4 ShadowTextureCoordinate : TEXCOORD3;
};

interface IShadowMapping
{
	float4 ComputeFinalColor(VS_OUTPUT IN);
};

class ShadowMapper : IShadowMapping
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
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

			float3 ambient = AmbientColor.a * AmbientColor.rgb * color.rgb;
			float3 diffuse = (LightColor.a * LightColor.rgb * lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular =(SpecularColor.a * SpecularColor.rgb * min(lightCoefficients.z, color.w)) * IN.Attenuation;

			if (IN.ShadowTextureCoordinate.w >= 0.0f)
			{
				IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;
				float pixelDepth = IN.ShadowTextureCoordinate.z;
				float sampledDepth = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy).x + DepthBias;

				// Shadow applied in a boolean fashion -- either in shadow or not
				float3 shadow = (pixelDepth > sampledDepth ? ColorBlack : ColorWhite.rgb);
					diffuse *= shadow;
				specular *= shadow;
			}

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

class ManualPCFShadowMapper : IShadowMapping
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
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

			float3 ambient = AmbientColor.a * AmbientColor.rgb * color.rgb;
			float3 diffuse = (LightColor.a * LightColor.rgb * lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular = (SpecularColor.a * SpecularColor.rgb * min(lightCoefficients.z, color.w)) * IN.Attenuation;

			if (IN.ShadowTextureCoordinate.w >= 0.0f)
			{
				IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;

				float2 texelSize = 1.0f / ShadowMapSize;
					float sampledDepth1 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy).x + DepthBias;
				float sampledDepth2 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(texelSize.x, 0)).x + DepthBias;
				float sampledDepth3 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(0, texelSize.y)).x + DepthBias;
				float sampledDepth4 = ShadowMap.Sample(ShadowMapSampler, IN.ShadowTextureCoordinate.xy + float2(texelSize.x, texelSize.y)).x + DepthBias;

				float pixelDepth = IN.ShadowTextureCoordinate.z;
				float shadowFactor1 = (pixelDepth > sampledDepth1 ? 0.0f : 1.0f);
				float shadowFactor2 = (pixelDepth > sampledDepth2 ? 0.0f : 1.0f);
				float shadowFactor3 = (pixelDepth > sampledDepth3 ? 0.0f : 1.0f);
				float shadowFactor4 = (pixelDepth > sampledDepth4 ? 0.0f : 1.0f);

				float2 lerpValues = frac(IN.ShadowTextureCoordinate.xy * ShadowMapSize);
					float shadow = lerp(lerp(shadowFactor1, shadowFactor2, lerpValues.x), lerp(shadowFactor3, shadowFactor4, lerpValues.x), lerpValues.y);
				diffuse *= shadow;
				specular *= shadow;
			}

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

class PCFShadowMapper : IShadowMapping
{
	float4 ComputeFinalColor(VS_OUTPUT IN)
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

			float3 ambient = AmbientColor.a * AmbientColor.rgb * color.rgb;
			float3 diffuse = (LightColor.a * LightColor.rgb * lightCoefficients.y * color.rgb) * IN.Attenuation;
			float3 specular = (SpecularColor.a * SpecularColor.rgb * min(lightCoefficients.z, color.w)) * IN.Attenuation;

			IN.ShadowTextureCoordinate.xyz /= IN.ShadowTextureCoordinate.w;
		float pixelDepth = IN.ShadowTextureCoordinate.z;

		float shadow = ShadowMap.SampleCmpLevelZero(PcfShadowMapSampler, IN.ShadowTextureCoordinate.xy, pixelDepth).x;
		diffuse *= shadow;
		specular *= shadow;

		OUT.rgb = ambient + diffuse + specular;
		OUT.a = 1.0f;

		return OUT;
	}
};

IShadowMapping ShadowMappingInstance;

float4 main(VS_OUTPUT IN) : SV_TARGET
{
	return ShadowMappingInstance.ComputeFinalColor(IN);
}