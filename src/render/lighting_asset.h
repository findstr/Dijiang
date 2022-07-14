#pragma once
#include <memory>
#include "render/texture2d.h"
#include "render/cubemap.h"

namespace engine {
namespace render {

class lighting_asset {
public:
	static lighting_asset &inst() {
		static lighting_asset *_inst = new lighting_asset();
		return *_inst;
	}
public:
	std::shared_ptr<render::texture> brdf_texture;
	std::shared_ptr<render::texture> skybox_specular;
	std::shared_ptr<render::texture> skybox_irradiance;
};

#define LIGHTING_ASSET ::engine::render::lighting_asset::inst()

}}

