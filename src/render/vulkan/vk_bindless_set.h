#pragma once
#include <vulkan/vulkan.hpp>

namespace engine {
namespace vulkan {

template<int MAX_BINDLESS_RESOURCE>
class vk_bindless_set {
public:
	vk_bindless_set &bind(const VkDescriptorSetLayoutBinding &binding) {
		bindings.emplace_back(binding);
		return *this;
	}
	void create() {
		destroy();
		VkDescriptorBindingFlags bindless_flags = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT | 
			VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT | \
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info { 
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT, nullptr 
		};
		extended_info.bindingCount = 1;
		extended_info.pBindingFlags = &bindless_flags;
		VkDescriptorSetLayoutCreateInfo layout_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		layout_info.bindingCount = bindings.size();
		layout_info.pBindings = bindings.data();
		layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;	
		layout_info.pNext = &extended_info;

		auto ret = vkCreateDescriptorSetLayout(VK_CTX.device, &layout_info, nullptr, &layout_);
		assert(ret == VK_SUCCESS);

		VkDescriptorSetVariableDescriptorCountAllocateInfoEXT count_info { 
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT 
		};
		uint32_t max_binding_id = MAX_BINDLESS_RESOURCE - 1;
		count_info.descriptorSetCount = 1;
		count_info.pDescriptorCounts = &max_binding_id;
	
		VkDescriptorSetAllocateInfo alloc_info { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		alloc_info.descriptorPool = VK_CTX.descriptorpool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &layout_;	
		alloc_info.pNext = &count_info;
		ret = vkAllocateDescriptorSets(VK_CTX.device, &alloc_info, &handle_);
		assert(ret == VK_SUCCESS);
		bindings.clear();
	}

	void destroy() {
		if (layout_ != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(VK_CTX.device, layout_, nullptr);
			layout_ = VK_NULL_HANDLE;
		}
	}
	~vk_bindless_set() {
		destroy();
	}
public:
	constexpr VkDescriptorSetLayout layout() const { return layout_; }
	constexpr VkDescriptorSet handle() const { return handle_; }
private:
	VkDescriptorSetLayout layout_ = VK_NULL_HANDLE;
	VkDescriptorSet handle_ = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayoutBinding> bindings;
};

}}
