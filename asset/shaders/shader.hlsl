#include "include/engine_variables.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	float3 inColor : COLOR0;
	float2 inTexCoord : TEXCOORD0;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float3 fragColor : COLOR0;
	float2 fragTexCoord : TEXCOORD0;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(input.inPosition, 1.0));
	output.fragColor = input.inColor;
	output.fragTexCoord = input.inTexCoord;
	return output;
}

Texture2D tex;
SamplerState tex_sampler;

float4 frag(VSOutput input) : SV_TARGET
{
	return tex.Sample(tex_sampler, input.fragTexCoord);
}

