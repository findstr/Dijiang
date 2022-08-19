#pragma once
#include <array>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "render/texture.h"
#include "vk_ctx.h"
#include "vk_filter.h"
#include "vk_sampler_address_mode.h"

namespace engine {
namespace vulkan {

class vk_sampler_pool {
public:
	const int MAX_ANISOTROPY = 16;
	static vk_sampler_pool &inst() {
		static vk_sampler_pool inst;
		return inst;
	}
public:
	VkSampler fetch(const render::texture *tex) {
		assert(texture_wrap::value::COUNT <= 4);
		assert(texture_filter::value::COUNT <= 4);
		assert(tex->anisolevels <= 32);
		uint32_t address_id =
			(tex->wrap_mode_u) << 4 | 
			(tex->wrap_mode_v) << 2 | 
			(tex->wrap_mode_w) << 0;		//6bit
		uint32_t filter_id = (int)tex->filter_mode;	//2bit
		uint32_t aniso_id = tex->anisolevels;		//5bit
		uint32_t mipmap_id = tex->miplevels;		//19bit
		uint32_t id = address_id | filter_id << 6 | aniso_id << 8 | mipmap_id << 11;
		auto &handle = sampler_pool[id];
		if (handle != VK_NULL_HANDLE)
			return handle;
		vk_filter vkfilter(tex->filter_mode);
		vk_sampler_address_mode vk_address_u(tex->wrap_mode_u);
		vk_sampler_address_mode vk_address_v(tex->wrap_mode_v);
		vk_sampler_address_mode vk_address_w(tex->wrap_mode_w);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = vkfilter;
		samplerInfo.minFilter = vkfilter;
		samplerInfo.addressModeU = vk_address_u;
		samplerInfo.addressModeV = vk_address_v;
		samplerInfo.addressModeW = vk_address_w;
		if (tex->anisolevels > 0) {
			int max = VK_CTX.properties.limits.maxSamplerAnisotropy;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = std::min(max, tex->anisolevels);
		}
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(tex->miplevels);
		auto ret = vkCreateSampler(VK_CTX.device, &samplerInfo, nullptr, &handle);
		if (ret != VK_SUCCESS)
			handle = VK_NULL_HANDLE;
		return handle;
	}
private:
	std::unordered_map<uint32_t, VkSampler> sampler_pool;
};

}}
