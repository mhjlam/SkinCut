// Noise.h.hlsl
// Various functions for generating Perlin noise, simplex noise, and fractal Brownian motion.


float2 mod289(float2 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 mod289(float3 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x)
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 permute(float3 x)
{
	return mod289(((x * 34.0) + 1.0) * x);
}

float4 permute(float4 x)
{
	return mod289(((x * 34.0) + 1.0) * x);
}

float4 taylorInvSqrt(float4 r)
{
	return 1.79284291400159 - 0.85373472095314 * r;
}

float fade(float t)
{
	return t*t*t * (t * (t * 6.0 - 15.0) + 10.0);
}

float2 fade(float2 t)
{
	return t*t*t * (t * (t * 6.0 - 15.0) + 10.0);
}


// textureless classic Perlin noise by Ian McEwan
float pnoise(float2 P)
{
	float4 Pi = floor(P.xyxy) + float4(0.0, 0.0, 1.0, 1.0);
	float4 Pf = frac(P.xyxy) - float4(0.0, 0.0, 1.0, 1.0);

	Pi = mod289(Pi);

	float4 ix = Pi.xzxz;
	float4 iy = Pi.yyww;
	float4 fx = Pf.xzxz;
	float4 fy = Pf.yyww;

	float4 i = permute(permute(ix) + iy);
	float4 gx = frac(i * (1.0 / 41.0)) * 2.0 - 1.0;
	float4 gy = abs(gx) - 0.5 ;
	float4 tx = floor(gx + 0.5);

	gx = gx - tx;

	float2 g00 = float2(gx.x, gy.x);
	float2 g10 = float2(gx.y, gy.y);
	float2 g01 = float2(gx.z, gy.z);
	float2 g11 = float2(gx.w, gy.w);
	float4 norm = taylorInvSqrt(float4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));

	g00 *= norm.x;
	g01 *= norm.y;
	g10 *= norm.z;
	g11 *= norm.w;

	float1 n00 = dot(g00, float2(fx.x, fy.x));
	float1 n10 = dot(g10, float2(fx.y, fy.y));
	float1 n01 = dot(g01, float2(fx.z, fy.z));
	float1 n11 = dot(g11, float2(fx.w, fy.w));

	float2 fade_xy = fade(Pf.xy);
	float2 n_x = lerp(float2(n00, n01), float2(n10, n11), fade_xy.x);

	return lerp(n_x.x, n_x.y, fade_xy.y) * 2.3;
}


// textureless simplex noise by Ian McEwan
float snoise(float2 v)
{
	const float4 C = float4(0.211324865405187,	// (3.0 - sqrt(3.0)) / 6.0 
							0.366025403784439,	// (sqrt(3.0) - 1.0) / 2.0
						   -0.577350269189626,  // C.x * 2.0 - 1.0
							0.024390243902439); // 1.0 / 41.0

	// first corner
	float2 i = floor(v + dot(v, C.yy));
	float2 x0 = v - i + dot(i, C.xx);

	// other corners
	float2 i1;
	i1.x = step(x0.y, x0.x);
	i1.y = 1.0 - i1.x;
	//float2 i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);

	// x1 = x0 - i1 + 1.0 * C.xx;
	// x2 = x0 - 1.0 + 2.0 * C.xx;
	float1 c = C.x * 2.0 - 1.0;
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;
	
	// permutations
	//i = mod(i, 289.0); // hlsl's mod works differently than glsl's
	i = mod289(i); // avoid truncation in polynomial evaluation
	float3 p = permute(permute(i.y + float3(0.0, i1.y, 1.0)) + i.x + float3(0.0, i1.x, 1.0));

	// circularly symmetric blending kernel
	float3 m = max(0.5 - float3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
	m = m*m*m*m;
	
	// gradients from 41 points on a line, mapped onto a diamond
	float3 x = frac(p * C.www) * 2.0 - 1.0;
	float3 gy = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 gx = x - ox;

	// normalize gradients implicitly by scaling m
	m *= 1.79284291400159 - 0.85373472095314 * (gx*gx + gy*gy);
	
	// compute final noise value at v
	float3 g;
	g.x  = gx.x  * x0.x   + gy.x  * x0.y;
	g.yz = gx.yz * x12.xz + gy.yz * x12.yw;
	
	// scale output to span range [-1, 1]
	return 130.0 * dot(m, g); // (this scale factor was determined empirically)
}

// fractal Brownian motion
float fbm(float2 texcoord, int octaves, float amplitude, float frequency)
{
	float s = 0.0;
	float a = amplitude;
	float f = frequency;

	[loop]
	for (int i = 0; i < octaves; i++)
	{
		s += snoise(texcoord * f) * a;
		f *= 2.0;
		a *= 0.5;
	}
	return s;
}

// fractal Brownian motion
float fbm(float2 texcoord, int octaves, float amplitude, float frequency, float lacunarity, float persistance)
{
	float1 s = 0.0;
	float1 a = amplitude;
	float1 f = frequency;

	[loop] for (int i = 0; i < octaves; i++)
	{
		s += snoise(texcoord * f) * a;
		f *= lacunarity;
		a *= persistance;
	}
	return s;
}

