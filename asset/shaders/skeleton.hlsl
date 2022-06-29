#include "include/engine_skeleton.inc.hlsl"

struct vsin {
	float3 position : POSITION0;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
	engine_skeleton_vertex skeleton;
};

struct vsout {
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
	float2 uv : TEXCOORD0;
};

vsout vert(vsin input) {
	vsout output;
	float3 pos = engine_skeleton_vertex_blend(input.position, input.skeleton);
	float4x4 vp = mul(engine_matrix_proj,  engine_matrix_view);
	float4x4 mvp = mul(engine_matrix_proj, mul(engine_matrix_view, engine_matrix_model));
	output.pos = mul(mvp, float4(pos, 1.0));
	output.color = input.color;
	output.uv = input.uv;
	return output;
}

Texture2D tex;
Texture2D normal;
SamplerState tex_sampler;

float4 frag(vsout input) : SV_TARGET
{
	return tex.Sample(tex_sampler, input.uv);
}

