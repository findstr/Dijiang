#include "include/engine_variables.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	float3 inColor : COLOR0;
	float2 inTexCoord : TEXCOORD0;
	uint instanceId : SV_InstanceID;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float3 fragTexCoord : TEXCOORD0;
	uint instanceId : SV_InstanceID;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 v = engine_matrix_view;
	v[0][3] = 0;
	v[1][3] = 0;
	v[2][3] = 0;
	float4x4 engine_matrix_model = engine_bindless_objects[input.instanceId].engine_matrix_model;
	output.pos = mul(engine_matrix_proj, mul(v, mul(engine_matrix_model, float4(input.inPosition, 1.0))));
	output.fragTexCoord = input.inPosition;
	output.pos.z = output.pos.w * 0.9999999;
	output.instanceId = input.instanceId;
	return output;
}

struct material {
	uint tex;
};

float4 frag(VSOutput input) : SV_TARGET
{
	material mat = engine_bindless_material.Load<material>(engine_bindless_objects[input.instanceId].material);
	float3 col =  engine_bindless_cubemap[mat.tex].Sample(engine_bindless_samplers[mat.tex], input.fragTexCoord).rgb;
	return float4(col, 1);
}

