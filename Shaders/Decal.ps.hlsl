// decal.ps.hlsl
// Pixel shader for decal projection.


#pragma pack_matrix(row_major)

struct PSIN
{
	float4 PositionSS : SV_POSITION;	// position screen-space
	float4 PositionCS : TEXCOORD0;		// position in clip-space
	float4 PositionVS : TEXCOORD1;		// position in view-space
	float3 NormalVS   : TEXCOORD2;		// normal in view-space
};

cbuffer cb0 : register(b0)
{
	matrix WorldInverse;
	matrix ViewInverse;
    matrix ProjectInverse;
};


Texture2D DepthMap;
Texture2D DecalTexture;
SamplerState LinearSampler;


float4 main(PSIN input) : SV_TARGET
{
	float width, height;
	DepthMap.GetDimensions(width, height);

	float2 screenPosition = input.PositionCS.xy / input.PositionCS.w; // [-1,1]
	float2 depthUV = screenPosition * float2(0.5, -0.5) + float2(0.5, 0.5); // [0,1], [1,0]
	depthUV += float2(0.5 / width, 0.5 / height); // half-pixel offset
	float depth = DepthMap.Sample(LinearSampler, depthUV).x;

	float x = depthUV.x * 2.0 - 1.0; // [-1,1]
	float y = (1.0 - depthUV.y) * 2.0 - 1.0; // [-1,1]
	float4 positionCS = float4(x, y, depth, 1.0f);
    float4 positionVS = mul(positionCS, ProjectInverse);
	positionVS = float4(positionVS.xyz / positionVS.w, 1.0);

	float4 positionWS = mul(positionVS, ViewInverse); // world-space
    float4 positionOS = mul(positionWS, WorldInverse); // decal object-space
	clip(0.5 - abs(positionOS.xyz)); // remove pixels not located inside decal mesh

	float3 decalNormal = normalize(input.NormalVS);
	float3 ddxWp = ddx(positionWS.xyz);
	float3 ddyWp = ddy(positionWS.xyz);
	float3 geometryNormal = -normalize(cross(ddyWp, ddxWp));
	float angle = dot(geometryNormal, decalNormal);

	float2 decalUV = positionOS.xz + float2(0.5, 0.5);
	return DecalTexture.Sample(LinearSampler, decalUV);
}
