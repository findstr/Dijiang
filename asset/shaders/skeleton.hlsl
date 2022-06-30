#include "include/engine_skeleton.inc.hlsl"
#include "include/engine_lighting.inc.hlsl"

struct vsin {
	float3 position : POSITION0;
	float3 color : COLOR0;
	float3 tangent : TANGENT0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0;
	engine_skeleton_vertex skeleton;
};

struct vsout {
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float3 normal : NORMAL0;
	float2 uv : TEXCOORD0; 
	float4 t_to_w0: TEXCOORD1;
	float4 t_to_w1: TEXCOORD2;
	float4 t_to_w2: TEXCOORD3;
	
};

vsout vert(vsin input) {
	vsout output;
	float3 pos = engine_skeleton_vertex_blend(input.position, input.skeleton);
	float4x4 vp = mul(engine_matrix_proj,  engine_matrix_view);
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(pos, 1.0));
	output.color = input.color;
	output.normal = mul(float4(input.normal, 0), engine_matrix_model).xyz;
	output.uv = input.uv;
	
	float3 world_pos = mul(engine_matrix_model, float4(pos, 1.0)).xyz;
	float3 world_normal = mul(engine_matrix_model, float4(input.normal, 1.0)).xyz;
	float3 world_tangent = mul(engine_matrix_model, float4(input.tangent, 1.0)).xyz;
	float3 world_binormal = cross(world_normal, world_tangent).xyz * -1;
	
	output.t_to_w0 = float4(world_tangent.x, world_binormal.x, world_normal.x, world_pos.x);
	output.t_to_w1 = float4(world_tangent.y, world_binormal.y, world_normal.y, world_pos.y);
	output.t_to_w2 = float4(world_tangent.z, world_binormal.z, world_normal.z, world_pos.z); 
	
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



float4 frag(vsout input) : SV_TARGET
{
	engine_light_param param;
	float4 metal_roughness = tex_roughness.Sample(tex_albedo_sampler, input.uv);
	float3 normal = tex_normal.Sample(tex_albedo_sampler, input.uv).xyz * 2.0 - 1.0;
	float3 world_pos = float3(input.t_to_w0.w, input.t_to_w1.w, input.t_to_w2.w);
	float4 albedo = tex_albedo.Sample(tex_albedo_sampler, input.uv);
	param.albedo = albedo.rgb;
	param.metallic = metal_roughness.z;
	param.roughness = metal_roughness.y;
 
	param.world_normal = normalize(float3(dot(input.t_to_w0.xyz, normal), dot(input.t_to_w1.xyz, normal), dot(input.t_to_w2.xyz, normal)));
	param.world_light_dir = normalize(engine_light_dir);
	param.world_view_dir = normalize(engine_world_view_dir(world_pos));
	param.light_radiance = engine_light_intensity;
		
	const float MAX_REFLECTION_LOD = 8.0;
	float3 R = reflect(-param.world_view_dir, param.world_normal);
	float3 ambient = tex_skybox_irradiance.Sample(tex_skybox_irradiance_sampler, param.world_normal).rgb;
	float3 prefilterColor = tex_skybox_specular.SampleLevel(tex_skybox_specular_sampler, R, param.roughness * MAX_REFLECTION_LOD).rgb;
	

	param.env_ambient = ambient;
	param.env_reflection = prefilterColor;
	
	return float4(engine_light_pbs(param), albedo.a);
}

