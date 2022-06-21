#pragma once
#include <assert.h>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "conf.h"
#include "vk_shader_variables.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

template<typename T>
struct vk_uniform {
private:
	struct uniform_buffer {
		bool active = false;
		vk_buffer buffer;
		VkDescriptorSet desc_set = VK_NULL_HANDLE;
		void create(VkDescriptorSetLayout layout, int need_size, int size_one) {
			destroy();
			buffer.create(vk_buffer::DYNAMIC, need_size);
			VkDescriptorSetAllocateInfo dsa{};
			dsa.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			dsa.descriptorPool = vk_ctx->descriptorpool;
			dsa.descriptorSetCount = 1;
			dsa.pSetLayouts = &layout;
			auto result = vkAllocateDescriptorSets(
				vk_ctx->logicdevice,
				&dsa, &desc_set);
			assert(result == VK_SUCCESS);
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer.handle;
			bufferInfo.offset = 0;
			bufferInfo.range = size_one;
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = desc_set;
			descriptorWrite.dstBinding = ENGINE_PER_DRAW_BINDING;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(vk_ctx->logicdevice, 1,
				&descriptorWrite, 0, nullptr);
			active = true;
		}
		void destroy() {
			if (active == false)
				return ;
			active = false;
			buffer.destroy();
			//TODO
			/*
			auto result = vkFreeDescriptorSets(
				vk_ctx->logicdevice,
				vk_ctx->descriptorpool,
				1, &desc_set);
			assert(result == VK_SUCCESS);
			*/
		}
	};
	size_t size_aligned = 0;
	size_t offset = 0;
	uniform_buffer *main = nullptr;
	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	std::array<uniform_buffer, conf::MAX_FRAMES_IN_FLIGHT> buffers;
public:
	~vk_uniform() {
		for (int i = 0; i < buffers.size(); i++)
			buffers[i].destroy();
		if (layout != VK_NULL_HANDLE)
			vkDestroyDescriptorSetLayout(vk_ctx->logicdevice, layout, nullptr);
	}
	vk_uniform() {
		size_t align = vk_ctx->properties.limits.minUniformBufferOffsetAlignment;
		size_aligned = sizeof(T);
		if (align > 0)
			size_aligned = (size_aligned + align - 1) / align * align;
		//TODO:move this peice code into renderctx_init
		VkDescriptorSetLayoutBinding bindings = {
			.binding = ENGINE_PER_DRAW_BINDING,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr,
		};
		VkDescriptorSetLayoutCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.bindingCount = 1;
		ci.pBindings = &bindings;
		auto result = vkCreateDescriptorSetLayout(vk_ctx->logicdevice, &ci, nullptr, &layout);
		assert(result == VK_SUCCESS);
	}
	VkDescriptorSet frame_begin(int frame_index, int need_count) {
		size_t need_size = need_count *
			size_aligned *
			conf::MAX_FRAMES_IN_FLIGHT;
		if (main == nullptr || main->buffer.size < need_size) {
			main = &buffers[frame_index];
			main->create(layout, need_size, size_aligned);
			offset = 0;
		} else {
			auto cur = &buffers[frame_index];
			if (cur->active && cur != main)
				cur->destroy();
		}
		return main->desc_set;
	}
	void frame_end() {
		if (offset == main->buffer.size)
			offset = 0;
	}
	std::tuple<T *, int> per_begin() {
		void *data = main->buffer.map();
		assert((offset + size_aligned) <= main->buffer.size);
		return std::make_tuple((T*)((uint8_t *)data + (int)offset), (int)offset);
	}
	void per_end() {
		main->buffer.unmap(offset, size_aligned);
		offset += size_aligned;
	}
};



}}

