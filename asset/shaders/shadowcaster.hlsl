#include "include/engine_variables.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	uint instanceId : SV_InstanceID;
};

struct VSOutput {
	float4 pos : SV_POSITION;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 engine_matrix_model = engine_bindless_objects[input.instanceId].engine_matrix_model;
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(input.inPosition, 1.0));
	return output;
}
 
float frag(VSOutput input) : SV_TARGET
{
	return input.pos.z;
}

