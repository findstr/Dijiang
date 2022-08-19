#pragma once
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine {
namespace vulkan {

class vk_desc_set_write {
private:
	struct write_cmd {
		VkDescriptorType	desc_type;
		VkDescriptorSet		dst_set = VK_NULL_HANDLE;
		uint32_t		dst_binding;
		uint32_t		dst_array_element;
		int			info_index = -1;
	};
	std::vector<write_cmd> write_cmds;
	std::vector<VkDescriptorBufferInfo> buffer_info;
	std::vector<VkDescriptorImageInfo> image_info;
public:
	static vk_desc_set_write &inst() {
		static vk_desc_set_write inst;
		return inst;
	};
public:
	vk_desc_set_write &write(VkDescriptorSet desc_set, int dst_binding, int dst_array_element, VkDescriptorBufferInfo &info) {
		write_cmds.emplace_back();
		auto &cmd = write_cmds.back();
		cmd.desc_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		cmd.dst_set = desc_set;
		cmd.dst_binding = dst_binding;
		cmd.dst_array_element = dst_array_element;
		cmd.info_index = buffer_info.size();
		buffer_info.emplace_back(info);
		return *this;
	}
	vk_desc_set_write &write(VkDescriptorSet desc_set, int dst_binding, int dst_array_element, VkDescriptorImageInfo &info) {
		write_cmds.emplace_back();
		auto &cmd = write_cmds.back();
		if (info.sampler != VK_NULL_HANDLE && info.imageView != VK_NULL_HANDLE)
			cmd.desc_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		else if (info.imageView != VK_NULL_HANDLE)
			cmd.desc_type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		else if (info.sampler != VK_NULL_HANDLE)
			cmd.desc_type = VK_DESCRIPTOR_TYPE_SAMPLER;
		else
			assert(!"invalid image info");
		cmd.dst_set = desc_set;
		cmd.dst_binding = dst_binding;
		cmd.dst_array_element = dst_array_element;
		cmd.info_index = image_info.size();
		image_info.emplace_back(info);
		return *this;
	};
	void flush() {
		std::vector<VkWriteDescriptorSet> desc_writes;
		desc_writes.resize(write_cmds.size());
		for (int i = 0; i < write_cmds.size(); i++) {
			auto &cmd = write_cmds[i];
			auto &write = desc_writes[i];
			write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			write.descriptorCount = 1;
			write.dstArrayElement = cmd.dst_array_element;
			write.descriptorType = cmd.desc_type;
			write.dstSet = cmd.dst_set;
			write.dstBinding = cmd.dst_binding;
			switch (cmd.desc_type) {
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
			case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
			case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
				write.pBufferInfo = &buffer_info[cmd.info_index];
				break;
			case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLER:
			case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
				write.pImageInfo = &image_info[cmd.info_index];
				break;
			}
		}
		vkUpdateDescriptorSets(VK_CTX.device, desc_writes.size(), desc_writes.data(), 0, nullptr);
	}

};

}}
