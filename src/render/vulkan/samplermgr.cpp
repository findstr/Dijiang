#include <algorithm>
#include <unordered_map>
#include "samplermgr.h"

namespace engine {
namespace vulkan {

VkSampler
samplermgr_newsampler(const renderctx *ctx, const texture_setting &args)
{
	VkSampler texsampler = VK_NULL_HANDLE;
	VkFilter filter;
	VkSamplerAddressMode addressmode;
	switch (args.filter_mode) {
	default:
	case TEX_FILTER_POINT:
		filter = VK_FILTER_NEAREST;
		break;
	case TEX_FILTER_BILINEAR:
		filter = VK_FILTER_LINEAR;
		break;
	case TEX_FILTER_TRILINEAR:
		filter = VK_FILTER_CUBIC_IMG;
		break;
	}
	switch (args.wrap_mode) {
	default:
	case TEX_WRAP_CLAMP:
		addressmode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		break;
	case TEX_WRAP_REPEAT:
		addressmode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		break;
	}
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = filter;
	samplerInfo.minFilter = filter;
	samplerInfo.addressModeU = addressmode;
	samplerInfo.addressModeV = addressmode;
	samplerInfo.addressModeW = addressmode;
	if (args.aniso_level > 0) {
		int max;
		VkPhysicalDeviceProperties prop{};
		vkGetPhysicalDeviceProperties(ctx->phydevice, &prop);
		max = prop.limits.maxSamplerAnisotropy;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = std::min(max, args.aniso_level);
	}
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(args.mipmap_levels);
	auto ret = vkCreateSampler(ctx->logicdevice, &samplerInfo, nullptr, &texsampler);
	if (ret != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return texsampler;
}

void
samplermgr_delsampler(const renderctx *ctx, VkSampler sampler)
{
	vkDestroySampler(ctx->logicdevice, sampler, nullptr);
}

}}


