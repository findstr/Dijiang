#include <assert.h>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "yaml-cpp/yaml.h"
#include "utils/file.h"
#include "stb_image.h"
#include "components/meshfilter.h"
#include "components/meshrender.h"
#include "components/camera.h"
#include "components/animator.h"
#include "components/skinrender.h"
#include "components/luacomponent.h"
#include "render/texture2d.h"
#include "render/cubemap.h"
#include "resource.h"


namespace engine {
namespace resource {

static std::unordered_map<std::string, std::shared_ptr<render::material>> mat_pool;
static std::unordered_map<std::string, std::shared_ptr<render::texture>> tex_pool;
static std::unordered_map<std::string, std::shared_ptr<render::mesh>> mesh_pool;

void
init()
{

}

void
cleanup()
{

}

static void
load_tex_file(const std::string &file, std::vector<uint8_t> &data, int *w, int *h, int *miplevels)
{
	int channels;
	stbi_uc* pixels = stbi_load(file.c_str(),
		w, h, &channels, STBI_rgb_alpha);
	if (pixels == nullptr) {
		fprintf(stderr, "[resource] load texture:%s fail\n", file.c_str());
		exit(0);
	}
	size_t image_size = *w * *h * 4;
	int max_length = std::max(*w, *h);
	//*miplevels = (uint32_t)(std::floor(std::log2(max_length))) + 1;
	*miplevels = 2;
	data.resize(image_size);
	memcpy(data.data(), pixels, (uint32_t)image_size);
	stbi_image_free(pixels);
}

std::shared_ptr<render::texture>
load_texture2d(const std::string &file)
{
	auto &tex = tex_pool[file];
	if (tex != nullptr)
		return tex;
	std::vector<uint8_t> pixels_data;
	int width, height, miplevels;
	load_tex_file(file, pixels_data, &width, &height, &miplevels);
	auto t = render::texture2d::create(width, height);
	t->setpixel(pixels_data);
	t->miplevels = miplevels;
	t->apply();
	tex.reset(t);
	return tex;
}

std::shared_ptr<render::texture>
load_cubemap(const std::array<std::string, render::cubemap::FACE_COUNT> &pathes)
{
	static const std::array<render::cubemap::face, render::cubemap::FACE_COUNT> faces = {
		render::cubemap::POSITIVE_X, render::cubemap::NEGATIVE_X,
		render::cubemap::POSITIVE_Y, render::cubemap::NEGATIVE_Y,
		render::cubemap::POSITIVE_Z, render::cubemap::NEGATIVE_Z
	};
	int width, height, miplevels;
	std::vector<uint8_t> pixels_data;
	load_tex_file(pathes[0], pixels_data, &width, &height, &miplevels);
	auto *tex = render::cubemap::create(width, height);
	tex->miplevels = miplevels;
	tex->setpixel(faces[0], pixels_data);
	for (int i = 1; i < render::cubemap::FACE_COUNT; i++) {
		int w, h, mip;
		load_tex_file(pathes[i], pixels_data, &w, &h, &mip);
		assert(w == width);
		assert(h == height);
		tex->setpixel(faces[i], pixels_data);
	}
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
	auto &mat = mat_pool[file];
	if (mat != nullptr)
		return mat;
	YAML::Node root = YAML::LoadFile(file);
	bool ztest = true;
	if (root["ztest"])
		ztest = root["ztest"].as<std::string>() == "on";
	auto shader_file = root["shader_file"].as<std::string>();
	std::cout << root["render_queue"].as<std::string>() << shader_file << std::endl;
	auto shader = load_shader(shader_file);
	auto *m = render::material::create(shader, ztest);
	mat.reset(m);
	auto params = root["shader_params"];
	for (int i = 0; i < params.size(); i++) {
		auto n = params[i];
		auto name = n["name"].as<std::string>();
		auto type = n["type"].as<std::string>();
		if (type == "texture2d") {
			auto file = n["file"].as<std::string>();
			auto tex = load_texture2d(file);
			m->set_texture(name, tex);
		} else if (type == "cubemap") {
			std::array<std::string, render::cubemap::FACE_COUNT> pathes;
			pathes[0] = n["x+"].as<std::string>();
			pathes[1] = n["x-"].as<std::string>();
			pathes[2] = n["y+"].as<std::string>();
			pathes[3] = n["y-"].as<std::string>();
			pathes[4] = n["z+"].as<std::string>();
			pathes[5] = n["z-"].as<std::string>();
			auto tex = load_cubemap(pathes);
			m->set_texture(name, tex);
		}
	}
	return mat;
}

std::shared_ptr<render::mesh>
load_mesh(const std::string &file)
{
	auto &mesh_ptr = mesh_pool[file];
	if (mesh_ptr != nullptr)
		return mesh_ptr;
	bool ret = false;
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(
		file.c_str(),
		aiProcess_Triangulate |
		aiProcess_CalcTangentSpace |
		aiProcess_FlipUVs |
		aiProcess_GenSmoothNormals);
	if (pScene == nullptr)
		return std::shared_ptr<render::mesh>(nullptr);
	const aiMesh* paiMesh = pScene->mMeshes[0];
	auto *mesh = render::mesh::create();
	mesh->vertices.reserve(paiMesh->mNumVertices);
	mesh->uv.reserve(paiMesh->mNumVertices);
	mesh->triangles.reserve(paiMesh->mNumFaces);
	for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
		const aiVector3D pPos = paiMesh->mVertices[i];
		const aiVector3D pTexCoord = paiMesh->mTextureCoords[0][i];
		mesh->vertices.emplace_back(pPos.x, pPos.y, pPos.z);
		mesh->uv.emplace_back(pTexCoord.x, pTexCoord.y);
		mesh->colors.emplace_back(0.f, 0.f, 0.f);
	}
	if (paiMesh->mNormals != nullptr) {
		mesh->normals.reserve(paiMesh->mNumVertices);
		for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
			const aiVector3D pNormal = paiMesh->mNormals[i];
			mesh->normals.emplace_back(pNormal.x, pNormal.y, pNormal.z);
		}
	}
	if (paiMesh->mTangents != nullptr) {
		mesh->tangents.reserve(paiMesh->mNumVertices);
		for (unsigned int i = 0 ; i < paiMesh->mNumVertices ; i++) {
			const aiVector3D pNormal = paiMesh->mTangents[i];
			mesh->tangents.emplace_back(pNormal.x, pNormal.y, pNormal.z);
		}
	}
	for (unsigned int i = 0; i < paiMesh->mNumFaces; i++) {
		auto face = &paiMesh->mFaces[i];
		mesh->triangles.emplace_back(face->mIndices[0]);
		mesh->triangles.emplace_back(face->mIndices[1]);
		mesh->triangles.emplace_back(face->mIndices[2]);
	}
	mesh->set_dirty();
	mesh_ptr.reset(mesh);
	return mesh_ptr;
}

static inline vector3f
parse_vector3f(YAML::Node &n) 
{
	float x = n['x'].as<float>();
	float y = n['y'].as<float>();
	float z = n['z'].as<float>();
	return vector3f(x, y, z);
}

static inline quaternion
parse_euler(YAML::Node &n) 
{
	quaternion q;
	float x = n['x'].as<float>();
	float y = n['y'].as<float>();
	float z = n['z'].as<float>();
	q.from_euler(x, y, z);
	return q;
}

static quaternion
parse_quaternion(YAML::Node &n) 
{
	float x = n['x'].as<float>();
	float y = n['y'].as<float>();
	float z = n['z'].as<float>();
	float w = n['w'].as<float>();
	return quaternion(x, y, z, w);
}

static matrix4f
parse_matrix(YAML::Node &n) 
{
	matrix4f mat = matrix4f::identity();
	for (auto it = n.begin(); it != n.end(); ++it) {
		auto key = it->first.as<std::string>();
		auto val = it->second.as<float>();
		int id = strtoul(key.c_str() + 1, nullptr, 10);
		int x = id % 4;
		int y = id / 4;
		mat(y, x) = val;
	}
	return mat;
}

std::shared_ptr<animation::skeleton>
load_skeleton(const std::string &file) 
{
	std::shared_ptr<animation::skeleton> skeleton(new animation::skeleton());
	auto *sk = skeleton.get();
	YAML::Node root = YAML::LoadFile(file)["skeleton"];
	for (int i = 0; i < root.size(); i++) {
		animation::skeleton::bone bone;
		auto bone_node = root[i]["bone"];
		bone.id = bone_node["id"].as<int>();
		bone.name = bone_node["name"].as<std::string>();
		bone.parent = bone_node["parent"].as<int>();

		auto pn = bone_node["position"];
		auto rn = bone_node["rotation"];
		auto sn = bone_node["scale"];
		auto tpose = bone_node["tpose"];
		bone.position = parse_vector3f(pn);
		bone.rotation = parse_quaternion(rn);
		bone.scale = parse_vector3f(sn);

		bone.tpose_matrix = parse_matrix(tpose);
		sk->add_bone(bone);
	}
	std::cout << "eof" << std::endl;
	return skeleton;
}

std::shared_ptr<animation::clip>
load_animation_clip(const std::string &file) 
{
	int i, n;
	int frame_count;
	int bone_count;
	std::string line;
	std::ifstream input_file(file);
	std::getline(input_file, line);
	n = sscanf(line.c_str(), "frame_count:%d", &frame_count);
	assert(n == 1);
	std::getline(input_file, line);
	n = sscanf(line.c_str(), "bone_count:%d", &bone_count);
	assert(n == 1);
	std::getline(input_file, line);
	auto *clip = new animation::clip(); 
	std::shared_ptr<animation::clip> clip_ptr(clip);
	clip->bone_count = bone_count;
	clip->bone_id.reserve(bone_count);
	char *p = (char *)line.c_str();
	for (i = 0, p = std::strtok(p, ","); p && i < bone_count; i++, p = std::strtok(nullptr, ",")) {
		clip->bone_id.emplace_back(strtoul(p, nullptr, 10));
	}
	for (int fi = 0; fi < frame_count; fi++) {
		std::getline(input_file, line);
		animation::clip::frame frame;
		p = (char *)line.c_str();
		for (i = 0, p = std::strtok(p, ","); p && i < bone_count; i++, p = std::strtok(nullptr, ",")) {
			animation::clip::bone_pose bone;
			n = sscanf(p, "%f:%f:%f&%f:%f:%f&%f:%f:%f:%f", 
				&bone.position.x(), &bone.position.y(), &bone.position.z(), 
				&bone.scale.x(), &bone.scale.y(), &bone.scale.z(),
				&bone.rotation.x(), &bone.rotation.y(), &bone.rotation.z(), &bone.rotation.w());
			assert(n == 10);
			auto rot = bone.rotation.normalized();
			frame.bone_poses.emplace_back(bone);
		}
		clip->frames.emplace_back(std::move(frame));
	}
	return clip_ptr;
}

static void
parse_transform(transform *tf, YAML::Node n)
{
	auto pn = n["position"];
	auto rn = n["rotation"];
	auto sn = n["scale"];

	tf->position = parse_vector3f(pn);

	float x = rn["x"].as<float>();
	float y = rn["y"].as<float>();
	float z = rn["z"].as<float>();
	tf->rotation.from_euler(x, y, z);

	tf->scale = parse_vector3f(sn);

	tf->local_position = tf->position;
	tf->local_rotation = tf->rotation;
	tf->local_scale = tf->scale;
}

static void
parse_meshfilter(meshfilter *mf, YAML::Node n)
{
	auto path = n[0]["submesh"]["mesh"].as<std::string>();
	auto mesh = load_mesh(path);
	mf->set_mesh(mesh);
	std::cout << "meshfilter" << path << std::endl;
}

static void
parse_meshrender(meshrender *mr, YAML::Node n)
{
	auto path = n["material"].as<std::string>();
	auto material = load_material(path);
	mr->set_material(material);
	std::cout << "meshrender" << path << std::endl;
}

static void
parse_camera(camera *cam, YAML::Node n)
{
	cam->fov = n["fov"].as<float>();
	cam->aspect = n["aspect"].as<float>();
	cam->clip_near_plane = n["clip_near_plane"].as<float>();
	cam->clip_far_plane = n["clip_far_plane"].as<float>();
}

static void
parse_animator(animator *ani, YAML::Node n)
{
	auto skl = load_skeleton(n["skeleton"].as<std::string>());
	ani->set_skeleton(skl);
}

static void
parse_skinrender(skinrender *sr, YAML::Node n) 
{
	auto mesh = load_mesh(n["mesh"].as<std::string>());
	auto mat = load_material(n["material"].as<std::string>());
	std::ifstream input_file(n["skin"].as<std::string>());
	auto *m = mesh.get();
	m->bone_weights.clear();
	std::string line;
	std::getline(input_file, line);
	int count = std::strtoul(line.c_str(), nullptr, 0);
	for (int i = 0; i < count; i++) {
		int j;
		render::mesh::bone_weight bone_weight;
		std::getline(input_file, line);
		char *p = (char *)line.c_str();
		for (j = 0, p = std::strtok(p, ","); p && j < 4; j++, p = std::strtok(nullptr, ",")) {
			int index;
			float weight;
			int n = sscanf(p, "%d:%f", &index, &weight);
			assert(n == 2);
			bone_weight.index[j] = index;
			bone_weight.weight[j] = weight;
		}
		while (j < 4) {
			bone_weight.index[j] = 0;
			bone_weight.weight[j] = 0;
			++j;
		}
		m->bone_weights.emplace_back(bone_weight);
	}
	assert(m->bone_weights.size() == m->vertices.size());
	sr->set_mesh(mesh);
	sr->set_material(mat);
}

static void
parse_lua(luacomponent *lua, YAML::Node n)
{
	for (auto it = n.begin(); it != n.end(); ++it) {
		auto key = it->first.as<std::string>();
		auto val = it->second.as<std::string>();
		if (val.size() == 0)
			continue;
		int ch = val[0];
		if (ch == '+' || ch == '-')
			lua->set_attr(key, std::stof(val));
		else
			lua->set_attr(key, val);
	}
}

void
load_level(const std::string &file, std::function<void(gameobject *, int)> add_go)
{
	YAML::Node root = YAML::LoadFile(file)["root"];
	for (int i = 0; i < root.size(); i++) {
		auto go_node = root[i]["gameobject"];
		auto id = go_node["id"].as<int>();
		auto parent = go_node["parent"].as<int>();
		auto name = go_node["name"].as<std::string>();
		auto coms_node = go_node["components"];
		auto go = new gameobject(id, name);
		add_go(go, parent);
		for (int j = 0; j < coms_node.size(); j++) {
			auto com_node = coms_node[j];
			for (auto it = com_node.begin(); it != com_node.end(); ++it) {
				auto type = it->first.as<std::string>();
				if (type == "transform") {
					parse_transform(&go->transform, it->second);
				} else if (type == "meshfilter") {
					auto mf = new meshfilter(go);
					go->add_component(mf);
					parse_meshfilter(mf, it->second);
				} else if (type == "meshrender") {
					auto mr = new meshrender(go);
					go->add_component(mr);
					parse_meshrender(mr, it->second);
				} else if (type == "camera") {
					auto cam = new camera(go);
					go->add_component(cam);
					parse_camera(cam, it->second);
				} else if (type == "animator") {
					auto *ani = new animator(go);
					go->add_component(ani);
					parse_animator(ani, it->second);
				} else if (type == "skinrender") {
					auto *sr = new skinrender(go);
					go->add_component(sr);
					parse_skinrender(sr, it->second);
				} else {
					auto lc = new luacomponent(go, type);
					go->add_component(lc);
					parse_lua(lc, it->second);
				}
			}
		}
	}
	std::cout << "eof" << std::endl;
	return ;
}

}}

