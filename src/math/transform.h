#pragma once
#include "math/math.h"
namespace engine {

struct transform {
	transform() {}
	constexpr const vector3f &position() const { return position_; }
	constexpr const quaternion &rotation() const { return rotation_; }
	constexpr const vector3f &scale() const { return scale_; }
	constexpr const vector3f &local_position() const { return local_position_; }
	constexpr const quaternion &local_rotation() const { return local_rotation_; }
	constexpr const vector3f &local_scale() const { return local_scale_; }
	constexpr const transform *parent() const { return parent_; }
	constexpr void set_parent(transform *p, bool keep_world = true) { 
		parent_ = p; 
		if (p != nullptr) {
			if (keep_world) {
				local_position_ = p->position() - position();
			} else {
				position_ = p->position() + local_position_;
				matrix_dirty = true;
			}
		}
	}
	inline void set_position(const vector3f &np) {
		position_ = np;
		if (parent_ != nullptr)
			local_position_ = parent_->position() - position();
		else
			local_position_ = position_;
		matrix_dirty = true;
	}
	inline void set_rotation(const quaternion &nq) {
		rotation_ = nq;
		if (parent_ != nullptr) 
			local_rotation_ = nq * parent_->rotation().inverse();
		else
			local_rotation_ = rotation_;
		matrix_dirty = true;
	}
	inline void set_euler_angles(float x, float y, float z) {
		rotation_.from_euler(x, y, z);
		if (parent_ != nullptr) 
			local_rotation_ = rotation_ * parent_->rotation().inverse();
		else
			local_rotation_ = rotation_;
		matrix_dirty = true;
	}
	inline void set_scale(const vector3f &ns) {
		scale_ = ns;
		if (parent_ != nullptr) {
			auto &ps = parent_->scale();
			local_scale_.x() = ns.x() / ps.x();
			local_scale_.y() = ns.y() / ps.y();
			local_scale_.z() = ns.z() / ps.z();
		} else {
			local_scale_ = scale_;
		}
		matrix_dirty = true;
	}
	inline void set_local_position(const vector3f &nlp)
	{
		local_position_ = nlp;
		if (parent_ != nullptr)
			position_ = parent_->position() + nlp;
		else
			position_ = nlp;
		matrix_dirty = true;
	}
	inline void set_local_rotation(const quaternion &nlq)
	{
		local_rotation_ = nlq;
		if (parent_ != nullptr)
			rotation_ = local_rotation_ * parent_->rotation();
		else
			rotation_ = local_rotation_;
		matrix_dirty = true;
	}
	inline void set_local_scale(const vector3f &nls)
	{
		local_scale_ = nls;
		if (parent_ != nullptr) {
			scale_.x() = local_scale_.x() * parent_->scale().x();
			scale_.y() = local_scale_.y() * parent_->scale().y();
			scale_.z() = local_scale_.z() * parent_->scale().z();
		} else {
			scale_ = nls;
		}
		matrix_dirty = true;
	}
	inline const matrix4f &local_to_world_matrix()
	{
		if (matrix_dirty == true)
			model_matrix = matrix4f::trs(position(), rotation(), scale());
		return model_matrix;
	}
private:
	vector3f position_;
	quaternion rotation_;
	vector3f scale_;
	vector3f local_position_;
	quaternion local_rotation_;
	vector3f local_scale_;
	matrix4f model_matrix;
	const transform *parent_ = nullptr;
	bool matrix_dirty = false;
};

}
