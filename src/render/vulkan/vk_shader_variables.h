#pragma once
#include <string>
namespace engine {
namespace vulkan {

constexpr const char *ENGINE_PER_FRAME_NAME = "engine_per_frame";
constexpr const char *ENGINE_PER_CAMERA_NAME = "engine_per_camera";
constexpr const char *ENGINE_PER_OBJECT_NAME = "engine_per_object";
constexpr const char *ENGINE_BRDF_TEX_NAME = "engine_brdf_tex";
constexpr const char *ENGINE_BRDF_TEX_SAMPLER_NAME = "engine_brdf_tex_sampler";
constexpr const char *ENGINE_SKYBOX_SPECULAR_NAME = "engine_skybox_specular";
constexpr const char *ENGINE_SKYBOX_SPECULAR_SAMPLER_NAME = "engine_skybox_specular_sampler";
constexpr const char *ENGINE_SKYBOX_IRRADIANCE_NAME = "engine_skybox_irradiance";
constexpr const char *ENGINE_SKYBOX_IRRADIANCE_SAMPLER_NAME = "engine_skybox_irradiance_sampler";
constexpr const char *ENGINE_SHADOWMAP_NAME = "engine_shadowmap";
constexpr const char *ENGINE_SHADOWMAP_SAMPLER_NAME = "engine_shadowmap_sampler";

constexpr const int ENGINE_DESC_SET	= 1;
constexpr const int ENGINE_PER_FRAME_BINDING	= 0;
constexpr const int ENGINE_PER_CAMERA_BINDING	= 1;
constexpr const int ENGINE_PER_OBJECT_BINDING	= 2;

constexpr const int ENGINE_BRDF_TEX_BINDING = 256;
constexpr const int ENGINE_SKYBOX_SPECULAR_BINDING = 257;
constexpr const int ENGINE_SKYBOX_IRRADIANCE_BINDING = 258;
constexpr const int ENGINE_SHADOWMAP_BINDING = 259;

constexpr const int ENGINE_BINDLESS_TEXTURE_BINDING = 0;
constexpr const int ENGINE_BINDLESS_OBJECT_BINDING = 0;
constexpr const int ENGINE_BINDLESS_MATERIAL_BINDING = 0;

constexpr const int ENGINE_BRDF_TEX_SAMPLER_BINDING = 384;
constexpr const int ENGINE_SKYBOX_SPECULAR_SAMPLER_BINDING = 385;
constexpr const int ENGINE_SKYBOX_IRRADIANCE_SAMPLER_BINDING = 386;
constexpr const int ENGINE_SHADOWMAP_SAMPLER_BINDING = 387;

}}

