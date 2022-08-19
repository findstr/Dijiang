#include "include/engine_variables.inc.hlsl"
#include "include/engine_lighting.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	float3 inColor : COLOR0;
	float2 inTexCoord : TEXCOORD0;
	uint instanceId : SV_InstanceID;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float3 fragColor : COLOR0;
	float2 fragTexCoord : TEXCOORD0;
	float4 world_pos : TEXCOORD1;
	uint instanceId : SV_InstanceID;
};

struct material {
	uint albedo;
	uint roughness;
	uint normal;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 engine_matrix_model = engine_bindless_objects[input.instanceId].engine_matrix_model;
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(input.inPosition, 1.0));
	output.world_pos = mul(engine_matrix_model, float4(input.inPosition, 1.0));
	output.fragColor = input.inColor;
	output.fragTexCoord = input.inTexCoord;
	output.instanceId = input.instanceId;
	return output;
}

float4 frag(VSOutput input) : SV_TARGET
{
	material mat = engine_bindless_material.Load<material>(engine_bindless_objects[input.instanceId].material);
	float shadow = engine_sample_shadowmap_depth(0, input.world_pos.xyz);
	float4 color = engine_bindless_textures[mat.albedo].Sample(engine_bindless_samplers[mat.albedo], input.fragTexCoord);
	return color;
}

