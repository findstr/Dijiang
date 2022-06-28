#pragma once
#include <iostream>
#include <cmath>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

namespace engine {

constexpr float PI = 3.14159265358f;
constexpr float DEG_TO_RAD = PI / 180.f;
constexpr float RAD_TO_DEG = 180.f / PI;

using vector2i = Eigen::Vector2i;
using vector4f = Eigen::Vector4f;

using matrix3f = Eigen::Matrix3f;
using angleaxisf = Eigen::AngleAxisf;

struct vector2f : public Eigen::Vector2f {
	vector2f() : Eigen::Vector2f() {}
	vector2f(const Eigen::Vector2f &v) : Eigen::Vector2f(v) {}
	vector2f(const vector2f &v) : Eigen::Vector2f(v) {}
	vector2f(float x, float y):Eigen::Vector2f(x, y) {}
	vector2f operator *(const vector2f &v) const {
		Eigen::Vector2f a(x(), y());
		Eigen::Vector2f b(v.x(), v.y());
		Eigen::Vector2f c = a.cwiseProduct(b);
		return c;
	}
	vector2f operator -(const vector2f &v) const {
		vector2f result;
		result.x() = x() - v.x();
		result.y() = y() - v.y();
		return result;
	}
};

struct vector3f : public Eigen::Vector3f {
	vector3f():Eigen::Vector3f() {}
	vector3f(float x, float y, float z):Eigen::Vector3f(x, y, z) {}
	vector3f(const Eigen::Vector3f &v) : Eigen::Vector3f(v) {}
	float magnitude() const {
		return norm();
	}
	float dot(const vector3f &v) const {
		return Eigen::Vector3f::dot(v);
	}
	vector3f operator *(const vector3f &v) const {
		Eigen::Vector3f a(x(), y(), z());
		Eigen::Vector3f b(v.x(), v.y(), v.z());
		Eigen::Vector3f c = a.cwiseProduct(b);
		return c;
	}
	vector3f operator *(float v) const {
		vector3f result;
		result.x() = x() * v;
		result.y() = y() * v;
		result.z() = z() * v;
		return result;
	}
	vector3f operator -(const vector3f &v) const {
		vector3f result;
		result.x() = x() - v.x();
		result.y() = y() - v.y();
		result.z() = z() - v.z();
		return result;
	}
	vector3f operator +(const vector3f &v) const {
		vector3f result;
		result.x() = x() + v.x();
		result.y() = y() + v.y();
		result.z() = z() + v.z();
		return result;
	}
	static vector3f up() {
		return vector3f(0, 1, 0);
	}
	static vector3f down() {
		return vector3f(0, -1, 0);
	}
	static vector3f forward() {
		return vector3f(0, 0, 1);
	}
	static vector3f back() {
		return vector3f(0, 0, -1);
	}
	static vector3f right() {
		return vector3f(1, 0, 0);
	}
	static vector3f left() {
		return vector3f(-1, 0, 0);
	}
	static vector3f lerp(const vector3f &a, const vector3f &b, float ratio) {
		vector3f delta = b - a;
		return a + delta * ratio;
	}
};

struct quaternion : public Eigen::Quaternionf {
	quaternion() : Eigen::Quaternionf() {}
	quaternion(float x, float y, float z, float w) : Eigen::Quaternionf(w,x,y,z) {}
	quaternion(const Eigen::Quaternionf &q) : Eigen::Quaternionf(q) {}
	quaternion(const quaternion &q) : Eigen::Quaternionf(q) {}
	quaternion &from_euler(float x, float y, float z) {
		*this = angleaxisf(x * DEG_TO_RAD, vector3f::UnitX()) *
			angleaxisf(y * DEG_TO_RAD, vector3f::UnitY()) *
			angleaxisf(z * DEG_TO_RAD, vector3f::UnitZ());
		return *this;
	}
	quaternion &from_axis_angle(const vector3f &axis, float angle) {
		*this = Eigen::Quaternionf(Eigen::AngleAxisf(angle * DEG_TO_RAD, axis));
		return *this;
	}
	vector3f to_euler() const {
		vector3f result = toRotationMatrix().eulerAngles(0, 1, 2);
		return result * RAD_TO_DEG;
	}
	float to_axis_angle(vector3f *axis) const {
		Eigen::AngleAxisf angle(toRotationMatrix());
		*axis = angle.axis();
		return angle.angle() * RAD_TO_DEG;
	}
	vector3f operator *(const vector3f &v) const {
		return (vector3f)((Eigen::Quaternionf)(*this) * v);
	}
	quaternion operator *(const quaternion &q) const {
		return (quaternion)((Eigen::Quaternionf)(*this) * q);
	}
	static quaternion look_at(const vector3f &forward, const vector3f up = vector3f::up()) {
		Eigen::Matrix3f rot;
		vector3f right = up.cross(forward);
		rot(0, 0) = right.x();
		rot(1, 0) = right.y();
		rot(2, 0) = right.z();

		rot(0, 1) = up.x();
		rot(1, 1) = up.y();
		rot(2, 1) = up.z();

		rot(0, 2) = forward.x();
		rot(1, 2) = forward.y();
		rot(2, 2) = forward.z();

		return Eigen::Quaternionf(rot);
	}
	static quaternion identity() {
		return Identity();
	}
	static quaternion slerp(const quaternion &a, const quaternion &b, float ratio) {
		return ((Eigen::Quaternionf)a).slerp(ratio, b);
	}
};

struct matrix4f : public Eigen::Matrix4f {
public:
	matrix4f() : Eigen::Matrix4f() {}
	matrix4f(const Eigen::Matrix4f &m) : Eigen::Matrix4f(m) {}
	matrix4f(matrix4f &m) : Eigen::Matrix4f(m) {}
public:
	matrix4f inverse() const {
		Eigen::Matrix4f m = ((Eigen::Matrix4f *)this)->inverse();
		return m;
	}
	matrix4f operator *(const matrix4f &m) {
		return (matrix4f)((Eigen::Matrix4f)(*this) * m);
	}
	static matrix4f identity() {
		Eigen::Matrix4f m = Eigen::Matrix4f::Identity();
		return m;
	}
	static matrix4f trs(
		const vector3f &translation, 
		const quaternion &rotation,
		const vector3f &scale)
	{
		Eigen::Transform<float, 3, Eigen::Affine> t = 
			Eigen::Translation3f(translation) *
			rotation * Eigen::Scaling(scale);
		return t.matrix();
	}
};

}

