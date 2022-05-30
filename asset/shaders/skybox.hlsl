struct UBO {
	float4x4 model;
	float4x4 view;
	float4x4 proj;
};

cbuffer ubo { UBO ubo; }

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
	float4x4 view = ubo.view;
	view[0][3] = 0;
	view[1][3] = 0;
	view[2][3] = 0;
	output.pos = mul(ubo.proj, mul(view, mul(ubo.model, float4(input.inPosition, 1.0))));
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

