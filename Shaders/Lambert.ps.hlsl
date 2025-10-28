// lambert.ps.hlsl
// Lambertian shader with ambient and diffuse components.


cbuffer cb0 : register(b0)
{
	float4 AmbientColor;
	float4 LightColor;
	float4 LightDirection;
};


float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0, float3 normal : TEXCOORD1) : SV_TARGET
{
	float4 ambient = 0.1 * AmbientColor;
	float4 diffuse = saturate(dot(normal, -LightDirection.xyz)) * LightColor;
	
	return ambient + diffuse;
}
