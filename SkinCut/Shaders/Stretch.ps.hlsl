// stretch.ps.hlsl
// Pixel shader that computes texture stretching due to UV distortion.


float4 main(float4 position : SV_POSITION, float3 worldpos : TEXCOORD) : SV_TARGET
{
	float3 deltaU = ddx(worldpos.xyz);
	float3 deltaV = ddy(worldpos.xyz);
	float1 stretchU = 0.001 / length(deltaU);
	float1 stretchV = 0.001 / length(deltaV);
	
    return float4(stretchU, stretchV, 0.0, 1.0);
}
