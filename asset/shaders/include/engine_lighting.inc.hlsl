#ifndef ENGINE_LIGHTING_H
#define ENGINE_LIGHTING_H

#include "engine_constant.inc.hlsl"
#include "engine_variables.inc.hlsl"
 
float3 engine_world_view_dir(float3 vertex_world_position)
{
	return engine_camera_pos - vertex_world_position;
}

struct engine_light_param {
	float3 albedo;
	float metallic;
	float roughness;
	float3 world_light_dir;
	float3 world_view_dir;
	float3 world_normal;
	float3 light_radiance;
	float3 env_ambient;
	float3 env_reflection;
};

float3 engine_light_blinn_phong(engine_light_param args)
{
	float3 ldir = args.world_light_dir * -1; //convert light dir from fragment to light
	float diff = max(0, dot(args.world_normal, ldir));
	float3 h = normalize(args.world_view_dir + ldir);
	float nh = max(0, dot(args.world_normal, h));
	float3 spec = pow(nh, 128)  * (1.0 - args.roughness) * float3(1,1,1);

	return ((diff + spec) * args.light_radiance + args.env_ambient) * args.albedo;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = ENGINE_PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (float3(1.0, 1.0, 1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}  

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float smoothness = 1.0 - roughness;
	return F0 + (max(float3(smoothness, smoothness, smoothness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 engine_sample_skybox_irradiance(float3 N)
{
	return engine_skybox_irradiance.Sample(engine_skybox_irradiance_sampler, N).rgb;
}

float3 engine_sample_skybox_reflection(float3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	float3 a = engine_skybox_specular.SampleLevel(engine_skybox_specular_sampler, R, lodf).rgb;
	float3 b = engine_skybox_specular.SampleLevel(engine_skybox_specular_sampler, R, lodc).rgb;
	return lerp(a, b, lod - lodf);
}

float engine_sample_shadowmap_depth(int i, float3 worldpos)
{
	float shadow = 1.0;
	float4x4 vp = mul(engine_light_matrix_proj[i], engine_light_matrix_view[i]);
	float4 pos = mul(vp, float4(worldpos, 1.0));
	float3 projCoords = pos.xyz / pos.w;
	projCoords.xy = projCoords.xy * 0.5 + float2(0.5, 0.5);
	float dist = engine_shadowmap[i].Sample(engine_shadowmap_sampler, projCoords.xy).r;
	if (projCoords.z > 1.0)
		shadow = 1.0;
	else
		shadow = projCoords.z < (dist + 0.005) ? 1.0 : 0.0;
	return shadow;
}

float3 engine_light_pbs(engine_light_param args)
{
	float3 L = args.world_light_dir * -1; //convert light dir from fragment to light
	float3 V = args.world_view_dir;
	float3 H = normalize(L + V);
	float3 N = args.world_normal;
	
	float NdotV = clamp(dot(N, V), 0.0, 1.0); //TODO: replace to max for test
	float NdotL = clamp(dot(N, L), 0.0, 1.0);
	float LdotH = clamp(dot(L, H), 0.0, 1.0);
	float NdotH = clamp(dot(N, H), 0.0, 1.0);
	
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, args.albedo, args.metallic);
	
	float3 radiance = args.light_radiance;
	float roughness = max(0.05, args.roughness);
	
	float3 Lo, ambient;
	{
		float3 F = fresnelSchlick(NdotV, F0);
		float D = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);

		float3 nominator = D * G * F;
		float denominator = 4.0 * NdotV * NdotL + 0.001;
		float3 specular = nominator / denominator;

		float3 kD = (float3(1,1,1) - F) * (1.0 - args.metallic);
		Lo = (kD * args.albedo / ENGINE_PI + (float3(1,1,1) - kD) * specular) * radiance * NdotL;
	}
	{
		float2 brdf = engine_brdf_tex.Sample(engine_brdf_tex_sampler, float2(NdotV, roughness)).rg;
		float3 reflection = args.env_reflection;
		float3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
		float3 specular = reflection * (F * brdf.x + brdf.y);
		float3 diffuse = args.env_ambient * args.albedo;

		float3 kD = (float3(1,1,1) - F) * (1 - args.metallic);
 
		ambient = kD * diffuse + specular;
	}
	return Lo + ambient;

} 
 
#endif

