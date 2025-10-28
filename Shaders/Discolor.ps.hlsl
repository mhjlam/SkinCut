// discolor.ps.hlsl
// Pixel shader for discoloration painting.


#include "noise.h.hlsl"
#include "distance.h.hlsl"

cbuffer cb_discolor : register(b0)
{
	float4 Discolor; // [0,2]
	float2 Point0;
	float2 Point1;
	float1 MaxDistance;
};


float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	// compute distance from cutline
	float1 t = 0.0;
    float1 dist = distance(Point0, Point1, texcoord, t);
	t = saturate(t) * 2.0 - 1.0; // first clamp to [0,1], then convert to [-1,1]

	// compute maximum distances for inner and outer layers
	float1 range_inner = (MaxDistance * 0.50) * (-(t*t/2)+1); // parabola: (y(-1)=0.5, y(0)=1, y(1)=0.5)
	float1 range_outer = 2.0 * range_inner;

	// pack color value ([0,2] to [0,1])
	float3 discolor = Discolor.rgb * float3(0.5,0.5,0.5);

	// compute alpha intensity for outer layer
    float1 range = 1.0 - (dist / range_outer);
	float1 noise = fbm(texcoord * 0.5, 4, 0.5, 64.0);
	float1 alpha = clamp(noise, 0.0, range);

	// increase alpha intensity for inner layer
    if (dist <= range_inner)
    {
        alpha += (1.1 - (dist / range_inner));
    }

	return float4(discolor, alpha);
}

