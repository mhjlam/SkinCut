// main.ps.hlsl
// Main skin rendering pixel shader.



#pragma pack_matrix(row_major)

#define NUM_LIGHTS 5
#define SHADOW_MAP_SIZE	2048


struct PSIN
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD0;
	float3 WorldPos : TEXCOORD1;	// world position
	float3 ViewDir  : TEXCOORD2;	// view direction
	float3 Normal   : TEXCOORD3;
	float4 Tangent  : TEXCOORD4;
};

struct PSOUT
{
	float4 Color    : SV_TARGET0;
	float4 Depth    : SV_TARGET1;
	float4 Specular : SV_TARGET2;
	float4 Discolor : SV_TARGET3;
};

struct Light
{
	float1 FarPlane;		// 10.0
	float1 FalloffDist;		// 0.924
	float1 FalloffWidth;	// 0.050
	float1 Attenuation;		// 0.008

	float4 Color;
	float4 Position;
	float4 Direction;

	matrix ViewProj;		// float4x4
};

cbuffer cbGlobals : register(b0)
{
	bool EnableColor;
	bool EnableBumps;
	bool EnableShadows;
	bool EnableSpeculars;
	bool EnableOcclusion;
	bool EnableIrradiance;

	float1 Ambient;
	float1 Fresnel;
	float1 Specular;
	float1 Bumpiness;
	float1 Roughness;
	float1 ScatterWidth;
	float1 Translucency;
};

cbuffer cbLights : register(b1)
{
	Light Lights[5];
};

Texture2D ColorMap : register(t0);
Texture2D NormalMap	: register(t1);
Texture2D SpecularMap : register(t2);
Texture2D OcclusionMap : register(t3);
Texture2D DiscolorMap : register(t4);
Texture2D BeckmannMap : register(t5);
TextureCube IrradianceMap : register(t6);
Texture2D ShadowMaps[5] : register(t7);

SamplerState SamplerLinear : register(s0);
SamplerState SamplerAnisotropic : register(s1);
SamplerComparisonState SamplerShadowComparison : register(s2);


// Kelemen/Szirmay-Kalos Specular BRDF.
float1 SpecularKSK(float3 normal, float3 light, float3 view, float1 intensity, float1 roughness)
{
	// half-way vector
	float3 halfway = light + view;
	float3 halfN = normalize(halfway);

	float1 NdotL = saturate(dot(normal, light));
	float1 NdotH = saturate(dot(normal, halfN));

	// fraction of facets oriented towards h
	float1 ph = pow(abs(2.0 * BeckmannMap.Sample(SamplerLinear, float2(NdotH, 1.0 - roughness)).r), 10.0);

	float f0 = 0.028;											// reflectance of skin at normal incidence
	float f = (1.0 - f0) * pow(1.0 - dot(view, halfN), 5.0);	// Schlick's Fresnel approximation
	f = lerp(0.25, f, Fresnel);									// modulate by given Fresnel value

	float1 fr = max(0.0, ph * f / dot(halfway, halfway));		// dot(halfway, halfway) = length(halfway)^2
	return intensity * NdotL * fr;
}


// Percentage-closer filtering (anti-aliased shadows).
float1 ShadowPCF(float3 worldPos, matrix viewProj, float1 farPlane, Texture2D shadowMap, int samples)
{
    float4 lightPos = mul(float4(worldPos, 1.0), viewProj);		// transform position into light space
    lightPos.xy = lightPos.xy / lightPos.w;						// perspective transform
    lightPos.z = lightPos.z - 0.01;								// shadow bias

	float1 shadow = 0.0;
	float1 offset = (samples - 1.0) / 2.0;
    float1 depth = lightPos.z / farPlane;						// depth of position, remapped to [0,1]

	[unroll]
	for (float1 x = -offset; x <= offset; x += 1.0) {
		[unroll]
		for (float1 y = -offset; y <= offset; y += 1.0) {
            float2 pos = lightPos.xy + (float2(x, y) / SHADOW_MAP_SIZE);
			shadow += saturate(shadowMap.SampleCmp(SamplerShadowComparison, pos, depth).r);
		}
	}

	return shadow / (samples*samples);
}


// Screen-space translucency.
float3 Transmittance(float3 worldPos, float3 normal, float3 lightDir, Texture2D shadowMap, matrix viewProj, float1 farPlane, float3 spotlight, float3 albedo)
{
	float scale = 8.25 * (1.0 - Translucency) / ScatterWidth;

	// used to prevent aliasing artifacts around edges
    float4 pos = float4(worldPos - 0.005 * normal, 1.0);
	
	// light-space position of pixel
    float4 lightPos = mul(pos, viewProj);
	
	// incoming and outgoing points
    float1 d1 = shadowMap.Sample(SamplerLinear, lightPos.xy / lightPos.w).r * farPlane;
    float1 d2 = lightPos.z;
	
	// distance between incoming and outgoing points
    float1 dist = abs(d1 - d2);

	float1 s = scale * dist;
	float1 ss = -s*s;

	// diffusion profile for skin
	float3 profile = float3(0, 0, 0);
	profile += float3(0.233, 0.455, 0.649) * exp(ss / 0.0064);
	profile += float3(0.100, 0.336, 0.344) * exp(ss / 0.0484);
	profile += float3(0.118, 0.198, 0.000) * exp(ss / 0.1870);
	profile += float3(0.113, 0.007, 0.007) * exp(ss / 0.5670);
	profile += float3(0.358, 0.004, 0.000) * exp(ss / 1.9900);
	profile += float3(0.078, 0.000, 0.000) * exp(ss / 7.4100);

	// transmittance = profile * (spotlight color * attenuation * falloff) * albedo * E;
	float3 E = saturate(0.3 + dot(-normal, lightDir)); // wrap lighting
	return profile * spotlight * albedo * E;
}


// Main skin rendering shader.
PSOUT main(PSIN input)
{
	PSOUT output;
	output.Color = float4(0,0,0,0);
	output.Depth = float4(0,0,0,0);
	output.Specular = float4(0,0,0,0);
	output.Discolor = float4(0,0,0,0);

	// Obtain albedo, occlusion, and irradiance for ambient lighting.
	float4 albedo = (EnableColor) ? ColorMap.Sample(SamplerAnisotropic, input.TexCoord) : float4(0.5, 0.5, 0.5, 1.0);
	float3 irradiance = (EnableIrradiance) ? IrradianceMap.Sample(SamplerLinear, input.Normal).rgb : float3(1.0, 1.0, 1.0);
	float1 occlusion = (EnableOcclusion) ? OcclusionMap.Sample(SamplerLinear, input.TexCoord).r : 1.0;

	// Obtain intensity and roughness for specular lighting.
	float1 intensity = (EnableSpeculars) ? SpecularMap.Sample(SamplerLinear, input.TexCoord).r * Specular : Specular;
	float1 roughness = (EnableSpeculars) ? SpecularMap.Sample(SamplerLinear, input.TexCoord).g * 3.3333 * Roughness : Roughness;
	
	// Normal mapping.
	float3 normal = input.Normal;

	if (EnableBumps) {
		// Construct tangent-space basis (TBN).
		float3 tangent = input.Tangent.xyz;
		float3 bitangent = input.Tangent.w * cross(normal, tangent); // compute bitangent from normal, tangent, and its handedness
		float3x3 tbn = transpose(float3x3(tangent, bitangent, normal)); // transforms from tangent space to world space (due to transpose)

		// Determine bump vector that perturbs normal.
		float3 bump = NormalMap.Sample(SamplerAnisotropic, input.TexCoord).rgb * 2.0 - 1.0; // remap from [0,1] to [-1,1]
		//bump.z = sqrt(1.0 - (bump.x * bump.x) - (bump.y * bump.y)); // z can be computed from xy (since normal map is normalized)

		// Perturb original normal by bump map in tangent space and transform to world space.
		float3 tangentNormal = lerp(float3(0.0, 0.0, 1.0), bump, Bumpiness.x); // lerp between original normal and bumped normal
		normal = mul(tbn, tangentNormal); // transform normal in tangent space to world space
	}

	
	// Compute and accumulate ambient lighting.
	float3 ambient = Ambient * occlusion * irradiance * albedo.rgb; // ambient light and occlusion
	output.Color.rgb += ambient;
	

	// Diffuse and specular lighting.
	[unroll] for (int i = 0; i < NUM_LIGHTS; i++) {
		Light light = Lights[i];
		Texture2D shadowMap = ShadowMaps[i];

		// Compute vector from surface position to light position.
		float3 lightDir = light.Position.xyz - input.WorldPos;
		float1 lightDist = length(lightDir); // distance to light
		lightDir /= lightDist; // normalize surface-to-light vector

		// Cosine of angle between light direction and surface-to-light vector.
		float1 spotAngle = dot(light.Direction.xyz, -lightDir);

		// If pixel is inside spotlight's circle of influence...
		if (spotAngle > light.FalloffDist) {
			// Compute spotlight attenuation and falloff
			float1 curve = min(pow(lightDist / light.FarPlane, 6.0), 1.0);
			float1 attenuation = (1.0 - curve) * (1.0 / (1.0 + light.Attenuation * lightDist*lightDist)); // if curve=1 => att=0
            float1 falloff = saturate((spotAngle - light.FalloffDist) / light.FalloffWidth);

			// Compute wavelength-independent diffuse and specular lighting.
			float1 lambert = saturate(dot(normal, lightDir)); // Lambertian reflectance
			float1 kelemen = SpecularKSK(normal, lightDir, input.ViewDir, intensity, roughness); // Kelemen/Szirmay-Kalos

			// Compute shadows (percentage-closer filtering).
			float1 shadow = (EnableShadows) ? ShadowPCF(input.WorldPos, light.ViewProj, light.FarPlane, shadowMap, 3) : 1.0;

			// Compute per-light diffuse and specular components.
			float3 spotLight = light.Color.xyz * attenuation * falloff;
			float3 diffuse = spotLight * albedo.rgb * lambert * shadow;
            float3 specular = spotLight * kelemen * shadow;

			// Accumulate diffuse and specular reflectance.
			if (EnableSpeculars) { // separate speculars
				output.Color.rgb += diffuse;
				output.Specular.rgb += specular;
			}
			else { // speculars included in output color
				output.Color.rgb += (diffuse + specular);
			}

			// Compute and accumulate transmittance (translucent shadow maps).
			if (EnableShadows) {
				output.Color.rgb += Transmittance(input.WorldPos, input.Normal, 
					lightDir, shadowMap, light.ViewProj, light.FarPlane, spotLight, albedo.rgb);
			}
		}
	}

	// render information to screen-space textures for later
	output.Color.a = albedo.a;
	output.Depth = input.Position.w;
	output.Discolor = DiscolorMap.Sample(SamplerAnisotropic, input.TexCoord);
	
	return output;
}
