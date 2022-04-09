#pragma once
#include <memory>
#include <array>
#include <vulkan/vulkan.hpp>
#include "conf.h"
#include "render/mesh.h"
#include "vk_buffer.h"

namespace engine {
namespace vulkan {

class vk_mesh : public render::mesh {
public:
	void flush();
public:
	std::unique_ptr<vk_buffer> vertex;
	std::unique_ptr<vk_buffer> index;
	int index_count = 0;
private:
	void upload_vertex(std::unique_ptr<vk_buffer> &buf);
	void upload_index(std::unique_ptr<vk_buffer> &buf);
};

}}

