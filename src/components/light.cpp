#include "level.h"
#include "light.h"
#include "graphics.h"

namespace engine {

light::light(gameobject *go) : component(go)
{
}

light::~light()
{
	if (regged)
		unreg();
}

void 
light::reg()
{
	light_list.emplace_back(this);
	regged = true;
}

void
light::unreg()
{
	if (light_list.size() > 0)
		std::erase(light_list, this);
}


void
light::get_shadow_matrix(camera *cam, vector3f &center, glm::mat4 &view, glm::mat4 &proj)
{
#if 1
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
	center = (center_near + center_far) * 0.5f;

	vector3f left_top_near = center_near + camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_top_near = center_near + camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;
	vector3f left_bottom_near = center_near - camera_up * h_near / 2.0f - camera_right * w_near / 2.0f;
	vector3f right_bottom_near = center_near - camera_up * h_near / 2.0f + camera_right * w_near / 2.0f;

	vector3f left_top_far = center_far + camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_top_far = center_far + camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;
	vector3f left_bottom_far = center_far - camera_up * h_far / 2.0f - camera_right * w_far / 2.0f;
	vector3f right_bottom_far = center_far - camera_up * h_far / 2.0f + camera_right * w_far / 2.0f;

#define extract(a) a.x(), a.y(), a.z()
	auto eye_dir = center + this->direction();
	auto up = this->transform->rotation() * vector3f::up();
	view = glm::lookAt(
			glm::vec3(extract(center)),
			glm::vec3(extract(eye_dir)),
			glm::vec3(extract(up)));
	std::array<glm::vec3, 8> frustum_li_space {
		view * glm::vec4(extract(right_bottom_near), 1.0f),
		view * glm::vec4(extract(right_top_near), 1.0f),
		view * glm::vec4(extract(left_bottom_near), 1.0f),
		view * glm::vec4(extract(left_top_near), 1.0f),
		view * glm::vec4(extract(right_bottom_far), 1.0f),
		view * glm::vec4(extract(right_top_far), 1.0f),
		view * glm::vec4(extract(left_bottom_far), 1.0f),
		view * glm::vec4(extract(left_top_far), 1.0f)
	};
	// find max and min points to define a ortho matrix around
	glm::vec3 min{ INFINITY, INFINITY, INFINITY };
	glm::vec3 max{ -INFINITY, -INFINITY, -INFINITY };
	for (unsigned int i = 0; i < frustum_li_space.size(); i++) {
	    if (frustum_li_space[i].x < min.x)
		min.x = frustum_li_space[i].x;
	    if (frustum_li_space[i].y < min.y)
		min.y = frustum_li_space[i].y;
	    if (frustum_li_space[i].z < min.z)
		min.z = frustum_li_space[i].z;

	    if (frustum_li_space[i].x > max.x)
		max.x = frustum_li_space[i].x;
	    if (frustum_li_space[i].y > max.y)
		max.y = frustum_li_space[i].y;
	    if (frustum_li_space[i].z > max.z)
		max.z = frustum_li_space[i].z;
	}

	float l = min.x;
	float r = max.x;
	float b = min.y;
	float t = max.y;
	// because max.z is positive and in NDC the positive z axis is 
	// towards us so need to set it as the near plane flipped same for min.z.
	float n = -max.z;
	float f = -min.z;

	proj = glm::orthoRH_ZO(
		l, r,
		b, t, 
		n, f);	
	proj[1][1] *= -1;
#else
	camera cam2(go);
	cam2.clip_near_plane = 0.0f;
	cam2.clip_far_plane = 50.0f;
	cam2.perspective = false;
	cam2.orthographic_size = 30.f;
	cam2.viewport.x = 0;
	cam2.viewport.y = 0;
	cam2.viewport.width = 1.0;
	cam2.viewport.height = 1.0;

	auto eye = cam2.transform->position;
	auto eye_dir = eye + cam2.forward() * 5.0f;
	auto up = cam2.up();

	view = glm::lookAtRH(
			glm::vec3(eye.x(), eye.y(), eye.z()),
			glm::vec3(eye_dir.x(), eye_dir.y(), eye_dir.z()),
			glm::vec3(up.x(), up.y(), up.z()));
	proj = glm::orthoRH_ZO(
			-cam2.orthographic_size, cam2.orthographic_size,
			-cam2.orthographic_size, cam2.orthographic_size, 
			cam2.clip_near_plane, cam2.clip_far_plane);
	proj[1][1] *= -1;
#endif
	return ;
}
 
std::vector<light *> light::light_list;
const std::vector<light *> &
light::all_lights()
{
	return light_list;
}


}


