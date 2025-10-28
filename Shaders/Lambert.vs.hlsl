#pragma pack_matrix(row_major)

struct VSIN
{
	float4 position  : POSITION0;
	float2 texcoord  : TEXCOORD0;
	float3 normal    : NORMAL0;
	float4 tangent   : TANGENT0;
};

struct VSOUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal   : TEXCOORD1;
};

cbuffer cb0 : register(b0)
{
	matrix WorldInverseTranspose;
	matrix WorldViewProjection;
};


VSOUT main(VSIN input)
{
	VSOUT output;
	output.position = mul(input.position, WorldViewProjection);
	output.texcoord = input.texcoord;
    output.normal = mul(float4(input.normal, 0), WorldInverseTranspose).xyz;
	return output;
}
