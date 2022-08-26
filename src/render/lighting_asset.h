#pragma once
#include <memory>
#include "render/texture2d.h"
#include "render/cubemap.h"
#include "resource/resource.h"
#include "yaml-cpp/yaml.h"
//TODO:
#include "vk_ctx.h"

namespace engine {
namespace render {

class lighting_asset {
public:
	static lighting_asset &inst() {
		static lighting_asset *_inst = new lighting_asset();
		return *_inst;
	}
	void init() {
		YAML::Node root = YAML::LoadFile("asset/lighting.asset");
		auto brdf = root["brdf_texture"];
		auto skybox = root["skybox"];
		auto skybox_specular = skybox["specular"];
		auto skybox_irradiance = skybox["irradiance"];
		this->brdf_texture = resource::load_texture2d(brdf["file"].as<std::string>());
		std::array<std::string, render::cubemap::FACE_COUNT> pathes;
		pathes[0] = skybox_specular["x+"].as<std::string>();
		pathes[1] = skybox_specular["x-"].as<std::string>();
		pathes[2] = skybox_specular["y+"].as<std::string>();
		pathes[3] = skybox_specular["y-"].as<std::string>();
		pathes[4] = skybox_specular["z+"].as<std::string>();
		pathes[5] = skybox_specular["z-"].as<std::string>();
		this->skybox_specular = resource::load_cubemap(pathes);
		pathes[0] = skybox_irradiance["x+"].as<std::string>();
		pathes[1] = skybox_irradiance["x-"].as<std::string>();
		pathes[2] = skybox_irradiance["y+"].as<std::string>();
		pathes[3] = skybox_irradiance["y-"].as<std::string>();
		pathes[4] = skybox_irradiance["z+"].as<std::string>();
		pathes[5] = skybox_irradiance["z-"].as<std::string>();
		this->skybox_irradiance = resource::load_cubemap(pathes);
		vulkan::vk_ctx_init_lighting();
	}
public:
	std::shared_ptr<render::texture> brdf_texture;
	std::shared_ptr<render::texture> skybox_specular;
	std::shared_ptr<render::texture> skybox_irradiance;
};

#define LIGHTING_ASSET ::engine::render::lighting_asset::inst()

}}

