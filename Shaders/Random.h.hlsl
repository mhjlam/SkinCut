// Random.h.hlsl
// Random number generator seeded by input texture coordinate.


float random(float2 p)
{
	float2 r = float2(23.1406926327792690,  // e^pi (Gelfond's constant)
		               2.6651441426902251); // 2^sqrt(2) (Gelfond–Schneider constant)
	return frac(cos(fmod(123456789.0, 1e-7 + 256.0 * dot(p, r))));
}

