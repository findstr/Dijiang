#include <iostream>
#include "vertex.h"
#include "vk_mesh.h"

namespace engine {
namespace vulkan {

void
vk_mesh::upload_vertex(std::unique_ptr<vk_buffer> &buf)
{
	vk_buffer *vk_buf;
	bool hasuv = uv.size() >= vertices.size();
	bool hascolor = colors.size() >= vertices.size();
	size_t vertex_size = render::vertex_type::size();
	VkDeviceSize size = vertex_size * sizeof(float) * vertices.size();
	vk_buffer staging(vk_buffer::STAGING, size);
	float *stage_buf =  (float *)staging.map();
	float *data = stage_buf;
	render::vertex_type t_pos = render::vertex_type::POSITION;
	render::vertex_type t_uv = render::vertex_type::TEXCOORD;
	render::vertex_type t_color = render::vertex_type::COLOR;

	for (int i = 0; i < vertices.size(); i++) {
		auto *p = &data[t_pos.offset()];
		p[0] = vertices[i].x();
		p[1] = vertices[i].y();
		p[2] = vertices[i].z();
		if (hasuv) {
			auto *p = &data[t_uv.offset()];
			p[0] = uv[i].x();
			p[1] = uv[i].y();
		}
		if (hascolor) {
			auto *p = &data[t_color.offset()];
			p[0] = colors[i].x();
			p[1] = colors[i].y();
			p[2] = colors[i].z();
		}
		data += vertex_size;
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


}}


