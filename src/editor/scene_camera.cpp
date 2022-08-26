#include "level.h"
#include "graphics.h"
#include "animation/skeleton.h"
#include "components/animator.h"
#include "render/debugger.h"
#include "scene_camera.h"

namespace engine {

scene_camera::scene_camera(gameobject *go) : camera(go)
{

}

void
scene_camera::draw_camera(camera *cam)
{
	float tan_half_fov = tan(cam->fov * RAD_TO_DEG / 2.0f);
	float h_near = 2 * tan_half_fov * cam->clip_near_plane;
	float w_near = h_near * cam->aspect;
	float h_far = 2 * tan_half_fov * cam->clip_far_plane;
	float w_far = h_far * cam->aspect;

	vector3f camera_up = cam->up();
	vector3f camera_right = cam->right();
	vector3f camera_forward = cam->forward();

	vector3f center_near = cam->transform->position() + camera_forward * cam->clip_near_plane;
	vector3f center_far = cam->transform->position() + camera_forward * cam->clip_far_plane;
	vector3f center = (center_near + center_far) * 0.5f;

	vector3f left_top_near = center_near + camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_top_near = center_near + camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;
	vector3f left_bottom_near = center_near - camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_bottom_near = center_near - camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;

	vector3f left_top_far = center_far + camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_top_far = center_far + camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;
	vector3f left_bottom_far = center_far - camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_bottom_far = center_far - camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;

			
	auto &dbg = render::debugger::inst();
	color c(1, 0, 0);
	dbg.draw_line(left_top_near, c, right_top_near, c);
	dbg.draw_line(left_top_near, c, left_bottom_near, c);
	dbg.draw_line(left_bottom_near, c, right_bottom_near, c);
	dbg.draw_line(right_top_near, c, right_bottom_near, c);

	dbg.draw_line(left_top_far, c, right_top_far, c);
	dbg.draw_line(left_top_far, c, left_bottom_far, c);
	dbg.draw_line(left_bottom_far, c, right_bottom_far, c);
	dbg.draw_line(right_top_far, c, right_bottom_far, c);

	dbg.draw_line(left_top_near, c, left_top_far, c);
	dbg.draw_line(left_bottom_near, c, left_bottom_far, c);
	dbg.draw_line(right_top_near, c, right_top_far, c);
	dbg.draw_line(right_bottom_near, c, right_bottom_far, c);

	dbg.draw_line(cam->transform->position(), c, left_top_near, c);
	dbg.draw_line(cam->transform->position(), c, left_bottom_near, c);
	dbg.draw_line(cam->transform->position(), c, right_top_near, c);
	dbg.draw_line(cam->transform->position(), c, right_bottom_near, c);
}

void
scene_camera::draw_light(camera *cam, light *li)
{
	float tan_half_fov = tan(cam->fov * RAD_TO_DEG / 2.0f);
	float h_near = 2 * tan_half_fov * cam->clip_near_plane;
	float w_near = h_near * cam->aspect;
	float h_far = 2 * tan_half_fov * cam->clip_far_plane;
	float w_far = h_far * cam->aspect;

	vector3f camera_up = cam->up();
	vector3f camera_right = cam->right();
	vector3f camera_forward = cam->forward();

	vector3f center_near = cam->transform->position() + camera_forward * cam->clip_near_plane;
	vector3f center_far = cam->transform->position() + camera_forward * cam->clip_far_plane;
	vector3f center = (center_near + center_far) * 0.5f;

	vector3f left_top_near = center_near + camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_top_near = center_near + camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;
	vector3f left_bottom_near = center_near - camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_bottom_near = center_near - camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;

	vector3f left_top_far = center_far + camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_top_far = center_far + camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;
	vector3f left_bottom_far = center_far - camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_bottom_far = center_far - camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;

#define extract(a) a.x(), a.y(), a.z()
	auto eye_dir = center + li->transform->rotation() * vector3f::forward();
	auto up = li->transform->rotation() * vector3f::up();
	glm::mat4 light_space = glm::lookAt(
			glm::vec3(extract(center)),
			glm::vec3(extract(eye_dir)),
			glm::vec3(extract(up)));
	std::array<glm::vec3, 8> frustumToLightView {
		light_space * glm::vec4(extract(right_bottom_near), 1.0f),
		light_space * glm::vec4(extract(right_top_near), 1.0f),
		light_space * glm::vec4(extract(left_bottom_near), 1.0f),
		light_space * glm::vec4(extract(left_top_near), 1.0f),
		light_space * glm::vec4(extract(right_bottom_far), 1.0f),
		light_space * glm::vec4(extract(right_top_far), 1.0f),
		light_space * glm::vec4(extract(left_bottom_far), 1.0f),
		light_space * glm::vec4(extract(left_top_far), 1.0f)
	};
#undef extract

	// find max and min points to define a ortho matrix around
	glm::vec3 min{ INFINITY, INFINITY, INFINITY };
	glm::vec3 max{ -INFINITY, -INFINITY, -INFINITY };
	for (unsigned int i = 0; i < frustumToLightView.size(); i++)
	{
	    if (frustumToLightView[i].x < min.x)
		min.x = frustumToLightView[i].x;
	    if (frustumToLightView[i].y < min.y)
		min.y = frustumToLightView[i].y;
	    if (frustumToLightView[i].z < min.z)
		min.z = frustumToLightView[i].z;

	    if (frustumToLightView[i].x > max.x)
		max.x = frustumToLightView[i].x;
	    if (frustumToLightView[i].y > max.y)
		max.y = frustumToLightView[i].y;
	    if (frustumToLightView[i].z > max.z)
		max.z = frustumToLightView[i].z;
	}

	float l = min.x;
	float r = max.x;
	float b = min.y;
	float t = max.y;
	// because max.z is positive and in NDC the positive z axis is 
	// towards us so need to set it as the near plane flipped same for min.z.
	float n = -min.z;
	float f = -max.z;

	float size = std::max((r - l) / 2.0f, (t - b) / 2.0f);

	{
	vector3f left_top_near = center + li->transform->rotation() * vector3f(l, t, n);
	vector3f right_top_near = center + li->transform->rotation() * vector3f(r, t, n);
	vector3f left_bottom_near = center + li->transform->rotation() * vector3f(l, b, n);
	vector3f right_bottom_near = center + li->transform->rotation() * vector3f(r, b, n);

	vector3f left_top_far = center + li->transform->rotation() * vector3f(l, t, f);
	vector3f right_top_far = center + li->transform->rotation() * vector3f(r, t, f);
	vector3f left_bottom_far = center + li->transform->rotation() * vector3f(l, b, f);
	vector3f right_bottom_far = center + li->transform->rotation() * vector3f(r, b, f);

	auto &dbg = render::debugger::inst();

	color c(0,1,0);

	dbg.draw_line(left_top_near, c, right_top_near, c);
	dbg.draw_line(left_top_near, c, left_bottom_near, c);
	dbg.draw_line(left_bottom_near, c, right_bottom_near, c);
	dbg.draw_line(right_top_near, c, right_bottom_near, c);

	dbg.draw_line(left_top_far, c, right_top_far, c);
	dbg.draw_line(left_top_far, c, left_bottom_far, c);
	dbg.draw_line(left_bottom_far, c, right_bottom_far, c);
	dbg.draw_line(right_top_far, c, right_bottom_far, c);

	dbg.draw_line(left_top_near, c, left_top_far, c);
	dbg.draw_line(left_bottom_near, c, left_bottom_far, c);
	dbg.draw_line(right_top_near, c, right_top_far, c);
	dbg.draw_line(right_bottom_near, c, right_bottom_far, c);
	}
}

void
scene_camera::render()
{
	/*
	level::cull(this, draw_list, render_pass::FORWARD);
	graphics.set_camera(this);
	if (show_skeleton) {
		for (auto &d:draw_list) {
			if (d.skeleton_pose == nullptr)
				continue;
			auto &dbg = render::debugger::inst();
			auto &pose = *d.skeleton_pose;
			auto *skel = ((animator *)d.go->get_component("animator"))->get_skeleton();
			for (size_t i = 0; i < pose.size(); i++) {
				auto &joint = pose[i];
				auto parent = skel->parent(joint.id);
				if (parent >= 0) {
					auto &trans = d.go->transform;
					dbg.draw_line(
						trans.rotation * pose[parent].position * trans.scale + trans.position, 
						color(0.0f, 0.0f, 1.f), 
						trans.rotation * joint.position * trans.scale + trans.position, 
						color(0.0, 1.0f, 0.0f)
					);
				}
			}
		}
	}
	if (show_camera) {
		for (auto cam:camera::all_cameras()) {
			draw_camera(cam);
			for (auto li:light::all_lights())
				draw_light(cam, li);
		}
	}
	graphics.renderpass_begin(render_target);
	for (auto &d:draw_list)
		graphics.draw(d);
	draw_list.clear();
	render::debugger::inst().cull(this, draw_list);
	for (auto &d:draw_list)
		graphics.draw(d);
	draw_list.clear();
	graphics.renderpass_end();
	*/
}

}


