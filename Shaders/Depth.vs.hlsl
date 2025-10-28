// depth.vs.hlsl
// Vertex shader for writing linearized depth buffers.


#pragma pack_matrix(row_major)

cbuffer cb0 : register(b0)
{
	matrix WVP;
};

float4 main(float4 pos : POSITION) : SV_POSITION
{
	float4 position = mul(pos, WVP);
	position.z *= position.w; // linearize depth
	return position;
}
