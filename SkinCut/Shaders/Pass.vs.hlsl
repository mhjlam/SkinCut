// pass.vs.hlsl
// Vertex shader that takes position and texture coordinate and passes position to next pipeline stage.


float4 main(float4 position : POSITION, inout float2 texcoord : TEXCOORD) : SV_POSITION
{
	return position;
}
