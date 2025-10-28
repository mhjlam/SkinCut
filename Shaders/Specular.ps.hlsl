// specular.ps.hlsl
// Pixel shader that samples input texture and returns its value.


Texture2D Texture: register(t0);
sampler Sampler : register(s0);


float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD0) : SV_TARGET
{
	return Texture.Sample(Sampler, texcoord);
}

