#include <iostream>
#include "vk_mesh.h"

namespace engine {
namespace vulkan {

void
vk_mesh::upload_vertex(std::unique_ptr<vk_buffer> &buf)
{
	vk_buffer *vk_buf;
	bool hasuv = uv.size() >= vertices.size();
	bool hascolor = colors.size() >= vertices.size();
	size_t vertex_size = sizeof(vertices[0]);
	if (hasuv)
		vertex_size += sizeof(float) * 2;
	if (hascolor)
		vertex_size += sizeof(float) * 3;
	VkDeviceSize size = vertex_size * vertices.size();
	vk_buffer staging(vk_buffer::STAGING, size);
	float *stage_buf =  (float *)staging.map();
	float *data = stage_buf;
	for (int i = 0; i < vertices.size(); i++) {
		*data++ = vertices[i].x();
		*data++ = vertices[i].y();
		*data++ = vertices[i].z();
		if (hasuv) {
			*data++ = uv[i].x();
			*data++ = uv[i].y();
		}
		if (hascolor) {
			*data++ = colors[i].x();
			*data++ = colors[i].y();
			*data++ = colors[i].z();
		}
	}
	staging.unmap();
	vk_buf = buf.get();
	if (vk_buf == nullptr || vk_buf->size < size) {
		buf.reset(nullptr); //release first, may trigger realloc
		vk_buf = new vk_buffer(vk_buffer::VERTEX, size);
		buf.reset(vk_buf);
	}
	vk_buf->copy_from(&staging);
	if (readonly == true)
		vertices.clear();
}

void
vk_mesh::upload_index(std::unique_ptr<vk_buffer> &buf)
{
	void *data;
	vk_buffer *vk_buf;
	VkDeviceSize size = sizeof(triangles[0]) * triangles.size();
	vk_buffer staging(vk_buffer::STAGING, size);
	staging.upload(triangles.data(), (size_t)size);
	vk_buf = buf.get();
	if (vk_buf == nullptr || vk_buf->size < size) {
		buf.reset(nullptr); //release first, may trigger realloc
		vk_buf = new vk_buffer(vk_buffer::INDEX, size);
		buf.reset(vk_buf);
	}
	vk_buf->copy_from(&staging);
	index_count = triangles.size();
	if (readonly == true)
		triangles.clear();
}

void
vk_mesh::flush()
{
	if (dirty == true) {
		upload_vertex(vertex);
		upload_index(index);
		dirty = false;
	}
}


}

namespace render {
mesh *
mesh::create()
{
	return new vulkan::vk_mesh();
}
}

}

