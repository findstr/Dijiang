#include "include/engine_variables.inc.hlsl"
#include "include/engine_lighting.inc.hlsl"

struct VSInput {
	float3 inPosition : POSITION0;
	float3 inColor : COLOR0;
	float2 inTexCoord : TEXCOORD0;
};

struct VSOutput {
	float4 pos : SV_POSITION;
	float3 fragColor : COLOR0;
	float2 fragTexCoord : TEXCOORD0;
	float4 world_pos : TEXCOORD1;
};

VSOutput vert(VSInput input) {
	VSOutput output = (VSOutput)0;
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(input.inPosition, 1.0));
	output.world_pos = mul(engine_matrix_model, float4(input.inPosition, 1.0));
	output.fragColor = input.inColor;
	output.fragTexCoord = input.inTexCoord;
	return output;
}

Texture2D tex;
SamplerState tex_sampler;

float4 frag(VSOutput input) : SV_TARGET
{
	float shadow = engine_sample_shadowmap_depth(0, input.world_pos.xyz);
	float4 color = tex.Sample(tex_sampler, input.fragTexCoord);
	color.rgb *= shadow;
	return color;
}

