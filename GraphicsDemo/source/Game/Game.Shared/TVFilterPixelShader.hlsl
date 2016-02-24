//used for offsets
static const float4 rgbScalesHalo = float4 (0.989f, 1.00f, 1.011f, 1);
static const float BlurDistance = 0.003f;
static const float4 tint = float4(0.08, 0.06, .11, .55);

cbuffer CBufferPerFrame
{
	float TotalTime;
	float ElapsedTime;
	float RandomStatic;
	float RandomStatic2;
};

cbuffer CBufferPerObject
{
	float2 DistortionPanningSpeed;
	float2 ScanlinePanningSpeed;
	float2 ScanlinePanningSpeed2;
	float2 NoisePanningSpeed1;
	float2 NoisePanningSpeed2;
};


Texture2D ColorTexture;


Texture2D DistortionTexture;
Texture2D ScanlineTexture;
Texture2D NoiseTexture;

SamplerState TrilinearSampler;

struct VS_OUTPUT
{
	float4 Position : SV_Position;
	float2 TextureCoordinate : TEXCOORD;
};

float4 Abberation(float2 tex)
{
	//uses a cubic distortion lens algorithm
	//Source: http://www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
	//Source: https://www.ssontech.com/content/lensalg.html

	//distortion coefficient
	float k = -0.15f;

	// cubic distortion value 
	float kcube = 0.15f;

	//float u = (tex.x - 0.5f);
	//float v = (tex.y - 0.5f)

	//image aspect ratio
	float r2 = (tex.x - 0.5f) * (tex.x - 0.5f) + (tex.y - 0.5f) * (tex.y - 0.5f);

	float f = 1.0f + r2 * (k + kcube * sqrt(r2));

	float4 distortColor = float4(0,0,0,1);


	// get the right pixel for the current position
	float x = (f)*(tex.x - 0.5f) + .5f + (0.001*sin(TotalTime*RandomStatic)*RandomStatic2);
	float y = (f)*(tex.y - 0.5f) + .5f + (0.001*sin(TotalTime*RandomStatic)*RandomStatic2);

	//iterates through all the color components
	for (int i = 0; i < 4; ++i)
	{

		//offsets the distorted colors.
		//Halo ideas taken from: http://www.polygonpi.com/?cat=3
		float u2 = x*(rgbScalesHalo[i]);
		float v2 = y*(rgbScalesHalo[i]);

		//get neighboring pixels
		for (int a = -1; a <= 1; ++a)
		{
			for (int b = -1; b <= 1; ++b)
			{
				distortColor[i] += ColorTexture.Sample(TrilinearSampler, float2(u2 + BlurDistance*a, v2 + BlurDistance*b))[i];
			}
		}

		//get the mean of the value to blur them.
		distortColor[i] /= 9;
	}

	// We need to devide the color with the amount of times we added
	// a color to it, in this case 4, to get the avg. color
	
	//clamps color then adds a bluish tint.
	return saturate(distortColor)*.8f + tint;
}



float4 main(VS_OUTPUT IN) : SV_TARGET
{
	//samples a texture used to distort the screen during post-processing. The float2 added is an offset
	float2 distortionOffset = float2(0, 0.1*sin(TotalTime)*RandomStatic2);
	float distortMag =  .5f*pow(RandomStatic2, 3);							//brightness of the distortion.
	float4 distortion = DistortionTexture.Sample(TrilinearSampler, IN.TextureCoordinate + distortionOffset) * distortMag;

	//samples a Scanline texture with varying UVs.
	float4 scanline1 = ScanlineTexture.Sample(TrilinearSampler, (IN.TextureCoordinate *1.28) + (ScanlinePanningSpeed * TotalTime));
	float4 scanline2 = clamp(ScanlineTexture.Sample(TrilinearSampler, (IN.TextureCoordinate * 128) + (ScanlinePanningSpeed2 * TotalTime)), 0.1, 0.5);

	//an offset used to change where to sample from: The original scene + any color that existed from the distortion texture sample
	float2 distortUV = IN.TextureCoordinate + distortion.rg* sin(TotalTime)*2*RandomStatic;

	float4 sceneColor = ColorTexture.Sample(TrilinearSampler, distortUV);

	//used for a flickering effect
	float sinWave = (0.1 * sin(600*TotalTime)) + 0.3;

	//multiples the sampled scanline textures to create a scrolling/pulsing scanline effect
	float4 multScan = scanline1 * scanline2;
		float3 scannedScene = multScan.rgb*sceneColor.rgb;

		//lerps between the original scene color and the scanned scene using the sin wave to quickly flicker
		float4 color = lerp(float4(sceneColor.rgb, 1), float4(scannedScene.rgb, 1), sinWave);

		//Sample a noise texture
		//Could alternatively use Perlin noise to achieve the static effect with fading brightness in and out.
		float4 noise1 = NoiseTexture.Sample(TrilinearSampler, IN.TextureCoordinate + (NoisePanningSpeed1* TotalTime));
		float4 noise2 = NoiseTexture.Sample(TrilinearSampler, IN.TextureCoordinate + (NoisePanningSpeed2* TotalTime));

		//average the noise apply it to the color.
		float4 compressNoise = ((((noise1 + noise2) / 2)  * 0.4) + 0.6) * color;

		//creates a static (of the TV-kind) distortion line
		float4 multNoise = ((noise1 * noise2) * distortion.r);
		float4 multNoise2 = multNoise * 40;

		//make it super bright so that it can be subtracted from.
		float4 invNoise = ((float4(1, 1, 1, 1) - multNoise) * compressNoise) + multNoise2;

		float4 eColor = invNoise - (40 * pow(distortion.rrrr, 4));

		//add chromatic Abberation to the end.
		return (eColor + Abberation(IN.TextureCoordinate)) * .8f;
}