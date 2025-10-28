// passpos.vs.hlsl
// Vertex shader that takes and passes position to next pipeline stage.


float4 main(float4 position : POSITION) : SV_POSITION
{
	return position;
}
