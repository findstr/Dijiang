#include <optional>
#include "vk_object.h"
#include "samplermgr.h"
#include "cmdbuf.h"
#include "vk_buffer.h"
#include "vk_texture2d.h"

namespace engine {
namespace vulkan {

void
vk_texture2d::apply()
{
	native.destroy();
	native.create(this);
	vk_buffer staging(vk_buffer::STAGING, pixel.size());
	staging.upload(pixel.data(), pixel.size());
	native.transition_layout(this, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	native.fill(this, staging);
	native.gen_mipmap(this);
}


}

namespace render {

texture2d *
texture2d::create(int width, int height,
	texture_format format,
	bool linear, int miplevels)
{
	texture2d *tex = new vulkan::vk_texture2d();
	tex->width_ = width;
	tex->height_ = height;
	tex->format = format;
	tex->linear = false;
	tex->miplevels = miplevels;
	return tex;
}

}

}

