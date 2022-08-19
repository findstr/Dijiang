#ifndef ENGINE_VARIABLES_H
#define ENGINE_VARIABLES_H

#include "engine_constant.inc.hlsl"

cbuffer engine_per_frame : register(b0, space1) {
	float3 engine_light_ambient;
	float3 engine_light_pos;
	float3 engine_light_dir;
	float3 engine_light_intensity;
	float4x4 engine_light_matrix_view[ENGINE_MAX_DIRECTIONAL_LIGHT];
	float4x4 engine_light_matrix_proj[ENGINE_MAX_DIRECTIONAL_LIGHT];
}

cbuffer engine_per_camera : register(b1, space1) {
	float4x4 engine_matrix_view;
	float4x4 engine_matrix_proj;
	float4x4 engine_matrix_view_proj;
	float3	 engine_camera_pos;
}

struct engine_per_object {
	int material;
	int dummy[3];
	float4x4 engine_matrix_model;
	float4x4 engine_matrix_model_view_proj;
	float4x4 engine_skeleton_matrix[ENGINE_MAX_BONE_PER_SKELETON];
};


Texture2D		engine_brdf_tex  : register(t0, space1);
SamplerState 	engine_brdf_tex_sampler : register(s0, space1);

TextureCube		engine_skybox_specular : register(t1, space1);
SamplerState	engine_skybox_specular_sampler : register(s1, space1);

TextureCube		engine_skybox_irradiance : register(t2, space1);
SamplerState	engine_skybox_irradiance_sampler : register(s2, space1);

Texture2D		engine_shadowmap[ENGINE_MAX_DIRECTIONAL_LIGHT] : register(t3, space1);
SamplerState    engine_shadowmap_sampler : register(s3, space1);


Texture2D							engine_bindless_textures[] : register(t0, space2);
SamplerState						engine_bindless_samplers[] : register(t0, space2);
StructuredBuffer<engine_per_object>	engine_bindless_objects : register(t0, space3);
ByteAddressBuffer					engine_bindless_material : register(t0, space4);


#endif

