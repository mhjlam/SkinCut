// Subsurface.ps.hlsl
// Post-process screen-space subsurface scattering pixel shader, modified to support local discoloration.


#define NUM_SAMPLES 9
#define ENABLE_DISCOLORATION

Texture2D ColorMap : register(t0);
Texture2D DepthMap : register(t1);

#ifdef ENABLE_DISCOLORATION
Texture2D DiscolorMap : register(t2);
#endif

sampler PointSampler : register(s0);
sampler LinearSampler : register(s1);


cbuffer cbGlobal : register(b0)
{
	float1 FOVy;				// vertical field of view of the camera
	float1 Width;				// subsurface scattering width in world space units
	float2 Direction;			// direction of the blur filter
	float4 Kernel[NUM_SAMPLES];	
};


float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET
{
	float4 color = ColorMap.Sample(PointSampler, texcoord);			// center sample color
	if (color.a == 0.0) discard;									// strength of blur
	float1 depth = DepthMap.Sample(PointSampler, texcoord).r;		// center sample linear depth

	// scale of the blur
	float1 distproj = 1.0 / tan(0.5 * radians(FOVy));				// distance from camera eye to projection window
	float1 scale = distproj / depth;								// scale of the blur = projection distance / depth

	// step to fetch surrounding samples
	float2 step = color.a * scale * Width * Direction * 0.5;		// divide by 2 due to kernel offsets range [-2,2]


#ifdef ENABLE_DISCOLORATION
	// discoloration of center sample
	float4 discolor = DiscolorMap.Sample(PointSampler, texcoord);	// center sample discoloration
	discolor.rgb = discolor.rgb * float3(2,2,2); // convert from [0,1] to [0,2]
	float3 center = lerp(Kernel[NUM_SAMPLES/2].xyz, Kernel[NUM_SAMPLES/2].xyz * discolor.rgb, discolor.a);

	// add contribution of center sample
	float4 final_color = float4(color.rgb * center, 1.0);
#else
	float4 final_color = float4(color.rgb * Kernel[NUM_SAMPLES/2].xyz, 1.0);
#endif

	// add contribution of other samples
	[unroll] for (int i = 0; i < NUM_SAMPLES; i++)
	{
		// skip center sample
		if (i == NUM_SAMPLES/2) continue;

		// sample color and depth for current sample
		float2 offset = texcoord + Kernel[i].w * step;
		float3 sample_color = ColorMap.Sample(LinearSampler, offset).rgb;
		float1 sample_depth = DepthMap.Sample(LinearSampler, offset).r;

		// let subsurface scattering follow the surface 
		// (if the difference in depth is very large, the color is lerped back to original color)
		float s = saturate(300.0 * distproj * Width * abs(depth - sample_depth));
		sample_color = lerp(sample_color, color.rgb, s);

		// accumulate contribution of sample, modulated by a kernel color
		final_color.rgb += sample_color.rgb * Kernel[i].xyz;
	}

	return final_color;
}

