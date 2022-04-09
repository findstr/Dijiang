#include <assert.h>
#include <string>
#include <memory>
#include <iostream>
#include <unordered_map>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "yaml-cpp/yaml.h"
#include "utils/file.h"
#include "stb_image.h"
#include "resource.h"
#include "render/vulkan/vk_pass.h"


namespace engine {
namespace resource {

void
init()
{

}

void
cleanup()
{

}

std::shared_ptr<render::texture>
load_texture(const std::string &file)
{
	int width, height, channels;
	std::vector<uint8_t> pixels_data;
	stbi_uc* pixels = stbi_load(file.c_str(),
		&width, &height, &channels, STBI_rgb_alpha);
	if (pixels == nullptr)
		return nullptr;
	size_t image_size = width * height * 4;
	int max_length = std::max(width, height);
	auto miplevels = (uint32_t)(std::floor(std::log2(max_length))) + 1;
	pixels_data.resize(image_size);
	memcpy(pixels_data.data(), pixels, (uint32_t)image_size);
	stbi_image_free(pixels);
	auto tex = render::texture::create(width, height);
	tex->setpixel(pixels_data);
	tex->miplevels = miplevels;
	tex->apply();
	return std::shared_ptr<render::texture>(tex);
}

std::shared_ptr<render::shader>
load_shader(const std::string &file)
{
	static struct stage {
		std::string type;
		render::shader::stage stage;
	} stages[] = {
		{"vert", render::shader::stage::VERTEX},
		{"tesc", render::shader::stage::TESSELLATION_CONTROL},
		{"tese", render::shader::stage::TESSELATION_EVALUTION},
		{"geom", render::shader::stage::GEOMETRY},
		{"frag", render::shader::stage::FRAGMENT},
	};
	std::vector<engine::render::shader::code> codes;
	codes.reserve(sizeof(stages) / sizeof(stages[0]));
	for (auto &sf:stages) {
		auto path = file + "." + sf.type + ".spv";
		auto spv = engine::utils::file::read(path);
		if (spv) {
			auto &x = codes.emplace_back();
			x.name = sf.type;
			x.stage = sf.stage;
			x.spv = *spv;
		}
	};
	return std::shared_ptr<render::shader>(render::shader::create(codes));
}

std::shared_ptr<render::material>
load_material(const std::string &file)
{
	YAML::Node root = YAML::LoadFile(file);
	auto shader_file = root["shader_file"].as<std::string>();
	std::cout << root["render_queue"].as<std::string>() << shader_file << std::endl;
	auto shader = load_shader(shader_file);
	auto *m = render::material::create(shader);
	std::shared_ptr<render::material> mat(m);
	auto params = root["shader_params"];
	for (int i = 0; i < params.size(); i++) {
		auto n = params[i];
		auto name = n["name"].as<std::string>();
		auto type = n["type"].as<std::string>();
		auto file = n["file"].as<std::string>();
		if (type == "texture2d") {
			auto tex = load_texture(file);
			m->set_texture(name, tex);
		}
	}
	return mat;
}

std::shared_ptr<render::mesh>
load_mesh(const std::string &file)
{
	bool ret = false;
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(
		file.c_str(),
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals |
		aiProcess_JoinIdenticalVertices);
	if (pScene == nullptr)
		return std::shared_ptr<render::mesh>(nullptr);
	const aiMesh* paiMesh = pScene->mMeshes[0];
	auto *mesh = render::mesh::create();
	mesh->vertices.reserve(paiMesh->mNumVertices);
	mesh->uv.reserve(paiMesh->mNumVertices);
	mesh->triangles.reserve(paiMesh->mNumFaces);
	for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
		const aiVector3D pPos = (paiMesh->mVertices[i]);
		const aiVector3D pTexCoord = paiMesh->mTextureCoords[0][i];
		mesh->vertices.emplace_back(pPos.x, pPos.y, pPos.z);
		mesh->uv.emplace_back(pTexCoord.x, pTexCoord.y);
		mesh->colors.emplace_back(0, 0, 0);
	}
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		auto face = &paiMesh->mFaces[i];
		mesh->triangles.emplace_back(face->mIndices[0]);
		mesh->triangles.emplace_back(face->mIndices[1]);
		mesh->triangles.emplace_back(face->mIndices[2]);
	}
	mesh->set_dirty();
	return std::shared_ptr<render::mesh>(mesh);
}


}}

