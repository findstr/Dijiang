#pragma once
#include <assert.h>
#include <memory>
#include <vulkan/vulkan.hpp>
#include "conf.h"
#include "vk_shader_variables.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

template<typename T, int BINDING>
struct vk_uniform {
private:
	struct uniform_buffer {
		bool active = false;
		vk_buffer buffer;
		void create(int need_size, int size_one) {
			destroy();
			buffer.create(vk_buffer::DYNAMIC, need_size);
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = buffer.handle;
			bufferInfo.offset = 0;
			bufferInfo.range = size_one;
			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = VK_CTX.engine_desc_set;
			descriptorWrite.dstBinding = BINDING;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;
			vkUpdateDescriptorSets(VK_CTX.logicdevice, 1,
				&descriptorWrite, 0, nullptr);
			active = true;
		}
		void destroy() {
			if (active == false)
				return ;
			active = false;
			buffer.destroy();
		}
	};
	size_t size_aligned = 0;
	size_t offset = 0;
	int frame_index = 0;
	uniform_buffer *main = nullptr;
	std::array<uniform_buffer, conf::MAX_FRAMES_IN_FLIGHT> buffers;
public:
	~vk_uniform() {
		for (int i = 0; i < buffers.size(); i++)
			buffers[i].destroy();
	}
	vk_uniform() {
		size_t align = VK_CTX.properties.limits.minUniformBufferOffsetAlignment;
		size_aligned = sizeof(T);
		if (align > 0)
			size_aligned = (size_aligned + align - 1) / align * align;
	}
	void frame_begin(int frame_index, int need_count) {
		size_t need_size = need_count *
			size_aligned *
			conf::MAX_FRAMES_IN_FLIGHT;
		if (main == nullptr || main->buffer.size < need_size) {
			main = &buffers[frame_index];
			main->create(need_size, size_aligned);
			offset = 0;
		} else {
			auto cur = &buffers[frame_index];
			if (cur->active && cur != main)
				cur->destroy();
		}
	}
	void frame_end() {
		if (++frame_index == conf::MAX_FRAMES_IN_FLIGHT) {
			frame_index = 0;
			offset = 0;
		}
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

