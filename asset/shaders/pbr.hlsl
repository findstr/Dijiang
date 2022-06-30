#include "include/engine_skeleton.inc.hlsl"
#include "include/engine_lighting.inc.hlsl"

struct vsin {
	float3 position : POSITION0;
	float3 color : COLOR0;
	float3 tangent : TANGENT0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
};

struct vsout {
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0; 
	float4 world_normal: NORMAL1;
	float4 world_tangent: TEXCOORD2;
	float4 world_binormal: TEXCOORD3;
	
};


vsout vert(vsin input) {
	vsout output;
	float3 pos =  input.position;
	float4x4 vp = mul(engine_matrix_proj,  engine_matrix_view);
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(pos, 1.0));
	output.color = input.color;
	output.normal = mul(engine_matrix_model, float4(input.normal, 0)).xyz;
	output.uv = input.uv;
	
	float3 world_pos = mul(engine_matrix_model, float4(pos, 1.0)).xyz;
	float3 world_normal = normalize(mul(engine_matrix_model, float4(input.normal, 0.0)).xyz);
	float3 world_tangent = normalize(mul(engine_matrix_model, float4(input.tangent, 0.0)).xyz);
	float3 world_binormal = normalize(cross(world_normal, world_tangent));
	
	output.world_normal = float4(world_normal, world_pos.x);
	output.world_tangent = float4(world_tangent, world_pos.y);
	output.world_binormal = float4(world_binormal, world_pos.z);
 
	return output;
}

Texture2D tex_albedo;
Texture2D tex_normal;
Texture2D tex_roughness;
SamplerState tex_albedo_sampler;

TextureCube tex_skybox_specular;
SamplerState tex_skybox_specular_sampler;
TextureCube tex_skybox_irradiance;
SamplerState tex_skybox_irradiance_sampler;

float3 prefilteredReflection(float3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0; // todo: param/const
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	float3 a = tex_skybox_specular.SampleLevel(tex_skybox_specular_sampler, R, lodf).rgb;
	float3 b = tex_skybox_specular.SampleLevel(tex_skybox_specular_sampler, R, lodc).rgb;
	return lerp(a, b, lod - lodf);
}

float4 frag(vsout input) : SV_TARGET
{
	engine_light_param param;
	float4 metal_roughness = tex_roughness.Sample(tex_albedo_sampler, input.uv);
	float3 normal = tex_normal.Sample(tex_albedo_sampler, input.uv).xyz * 2.0 - 1.0;
	float3 world_pos = float3(input.world_normal.w, input.world_tangent.w, input.world_binormal.w);
	float4 albedo = tex_albedo.Sample(tex_albedo_sampler, input.uv);
	
	param.albedo = albedo.rgb;
	param.metallic = metal_roughness.z;
	param.roughness = metal_roughness.y;
 
	float3 N = normalize(input.world_normal.xyz);
	float3 T = normalize(input.world_tangent.xyz);
	float3 B = normalize(cross(N, T));
	float3x3 TBN = transpose(float3x3(T, B, N));
 
	param.world_normal = normalize(mul(TBN, normal));
	param.world_light_dir = normalize(engine_light_dir);
	param.world_view_dir = normalize(engine_world_view_dir(world_pos));
	param.light_radiance = engine_light_intensity;

	/*	
	param.metallic = 0.0;
	param.roughness = 0.09106;

	param.albedo = float3(1.0, 0.765557, 0.336057) ;
	

	param.world_normal = normalize(input.normal);
	*/
	
	float3 R = reflect(-param.world_view_dir, N);
	 
	
	param.env_ambient = tex_skybox_irradiance.Sample(tex_skybox_irradiance_sampler, N).rgb;
	param.env_reflection = prefilteredReflection(R, param.roughness);
	
	float3 color = engine_light_pbs(param);
	return float4(color, 1);
}

