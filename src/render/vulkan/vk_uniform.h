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
		vk_buffer buffer;
		size_t offset = 0;
		void *data = nullptr;
		void refresh(vk_buffer &b, int elem_size, int frame_index) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = b.handle;
			bufferInfo.offset = 0;
			bufferInfo.range = elem_size;
			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = VK_CTX.engine_desc_set[frame_index];
			descriptorWrite.dstBinding = BINDING;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(VK_CTX.device, 1,
				&descriptorWrite, 0, nullptr);
		}
		void destroy()
		{
			buffer.destroy();
		}
		void map() {
			if (data == nullptr)
				data = buffer.map();
		}
		void unmap(int elem_size) {
			if (data != nullptr) {
				buffer.unmap(offset, elem_size);
				offset += elem_size;
				data = nullptr;
			}
		}
		void reserve(int count, int elem_size, int frame_index) {
			int need = offset + count * elem_size;
			if (need > buffer.size) {
				buffer.destroy();
				buffer.create(vk_buffer::DYNAMIC, need);
				refresh(buffer, elem_size, frame_index);
			}
		}
		void *alloc(int elem_size, int frame_index) {
			void *ptr;
			int need = offset + elem_size;
			assert(need <= buffer.size);
			ptr = ((unsigned char*)data + offset);
			return ptr;
		}
	};
	size_t size_aligned = 0;
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
	void frame_begin(int need_count) {
		buffers[VK_CTX.frame_index].offset = 0;
		buffers[VK_CTX.frame_index].reserve(need_count, size_aligned, VK_CTX.frame_index);
	}
	void frame_end() {
	}
	T *alloc() {
		auto &b = buffers[VK_CTX.frame_index];
		b.map();
		return (T *)b.alloc(size_aligned, VK_CTX.frame_index);
	}
	void unmap() {
		buffers[VK_CTX.frame_index].unmap(size_aligned);
	}
	int offset() const {
		return buffers[VK_CTX.frame_index].offset;
	}
};



}}

