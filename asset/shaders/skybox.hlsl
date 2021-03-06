#include "include/engine_variables.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	float3 inColor : COLOR0;
	float2 inTexCoord : TEXCOORD0;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float3 fragTexCoord : TEXCOORD0;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 v = engine_matrix_view;
	v[0][3] = 0;
	v[1][3] = 0;
	v[2][3] = 0;
	output.pos = mul(engine_matrix_proj, mul(v, mul(engine_matrix_model, float4(input.inPosition, 1.0))));
	output.fragTexCoord = input.inPosition;
	output.pos.z = output.pos.w * 0.999999;
	return output;
}

TextureCube tex;
SamplerState tex_sampler;

float4 frag(VSOutput input) : SV_TARGET
{
	return tex.Sample(tex_sampler, input.fragTexCoord);
}

