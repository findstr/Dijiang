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
	float3 fragColor : COLOR0;
	float2 fragTexCoord : TEXCOORD0;
};

VSOutput vert(VSInput input) {t st
	VSOutput output = (VSOutput)0;
	output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
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

