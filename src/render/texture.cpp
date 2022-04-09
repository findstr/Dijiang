#include <assert.h>
#include "texture.h"
namespace engine {
namespace render {

int texture::width() const
{
	return width_;
}

int texture::height() const
{
	return height_;
}

void
texture::setpixel(std::vector<uint8_t> &pixel)
{
	this->pixel = std::move(pixel);
}

}}
