#include <chrono>
#include "level.h"
#include "camera.h"
#include "glm/glm.hpp"
#include "graphics.h"

namespace engine {

camera::camera(gameobject *go) : 
	component(go)
{
}

camera::~camera()
{
	if (regged)
		unreg();
}

void 
camera::reg()
{
	camera_list.emplace_back(this);
	regged = true;
}

void
camera::unreg()
{
	if (camera_list.size() > 0)
		std::erase(camera_list, this);
}

void
camera::render()
{
	/*
	graphics.shadowpass_begin();

	vector3f center;
	ubo::lights lights;
	vector3f direction = li->direction().normalized();
	auto c = li->color;
	c.r *= li->intensity;
	c.g *= li->intensity;
	c.b *= li->intensity;
	ubo_lights.engine_light_ambient = glm::vec4(1.0f);
	ubo_lights.engine_light_direction.x = direction.x();
	ubo_lights.engine_light_direction.y = direction.y();
	ubo_lights.engine_light_direction.z = direction.z();
	ubo_lights.engine_light_radiance = glm::vec4(c.r, c.g, c.b, 1.0f);

	li->get_shadow_matrix(cam,
		center,
		ubo_lights.engine_light_matrix_view[0],
		ubo_lights.engine_light_matrix_project[0]);

	graphics.set_light(light::all_lights()[0], this);

	vector3f center;
	ubo_per_camera = uniform_per_camera->alloc();
	li->get_shadow_matrix(cam, center, 
		ubo_per_camera->view, 
		ubo_per_camera->proj);
	ubo_per_camera->engine_camera_pos = glm::vec4(center.x(), center.y(), center.z(), 1.0f);
	ubo_offset[vulkan::ENGINE_PER_CAMERA_BINDING] = uniform_per_camera->offset();
	uniform_per_camera->unmap();

	graphics.set_light_camera(light::all_lights()[0], this);
	level::cull(this, draw_list, render::shader::light_mode::SHADOWCASTER);
	graphics.init_for_object(draw_list);
	graphics.draw(draw_list);
	graphics.shadowpass_end();
	*/

	auto eye = transform->position();
	auto eye_dir = eye + forward() * 5.0f;
	auto up = this->up();
	ubo::camera ubo_cam;
	ubo_cam.engine_camera_pos = glm::vec4(
		this->transform->position().x(),
		this->transform->position().y(),
		this->transform->position().z(), 1.0);
	ubo_cam.view = glm::lookAt(
		glm::vec3(eye.x(), eye.y(), eye.z()),
		glm::vec3(eye_dir.x(), eye_dir.y(), eye_dir.z()),
		glm::vec3(up.x(), up.y(), up.z()));
	if (this->perspective) {
		ubo_cam.proj = glm::perspectiveRH_ZO(
			glm::radians(fov), aspect,
			clip_near_plane, clip_far_plane);
	}
	else {
		ubo_cam.proj = glm::orthoRH_ZO(
			-orthographic_size, orthographic_size,
			-orthographic_size, orthographic_size,
			clip_near_plane, clip_far_plane);
	}
	ubo_cam.proj[1][1] *= -1;
	ubo_cam.view_proj = ubo_cam.proj * ubo_cam.view;
	graphics.set_camera(ubo_cam);
	graphics.render_target = render_target;
	draw_list.clear();
	level::cull(this, draw_list, render::shader::light_mode::FORWARD);
	graphics.draw(draw_list, viewport);
	draw_list.clear();
}

std::vector<camera *> camera::camera_list;
const std::vector<camera *> &
camera::all_cameras()
{
	return camera_list;
}



}


