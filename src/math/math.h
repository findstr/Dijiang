#pragma once
#include <iostream>
#include <cmath>
#include <Eigen/Eigen>
#include <Eigen/Geometry>

namespace engine {

using vector2i = Eigen::Vector2i;
using vector4f = Eigen::Vector4f;

using matrix3f = Eigen::Matrix3f;
using matrix4f = Eigen::Matrix4f;
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
};

struct quaternion : public Eigen::Quaternionf {
	quaternion() : Eigen::Quaternionf() {}
	quaternion(const Eigen::Quaternionf &q) : Eigen::Quaternionf(q) {}
	quaternion(const quaternion &q) : Eigen::Quaternionf(q) {}
	quaternion &from_euler(float x, float y, float z) {
		*this = angleaxisf(x, vector3f::UnitX()) *
			angleaxisf(y, vector3f::UnitY()) *
			angleaxisf(z, vector3f::UnitZ());
		return *this;
	}
	quaternion &from_axis_angle(const vector3f &axis, float angle) {
		*this = Eigen::Quaternionf(Eigen::AngleAxisf(angle, axis));
		return *this;
	}
	vector3f to_euler() const {
		return toRotationMatrix().eulerAngles(0, 1, 2);
	}
	float to_axis_angle(vector3f *axis) const {
		Eigen::AngleAxisf angle(toRotationMatrix());
		*axis = angle.axis();
		return angle.angle();
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
};


constexpr float PI = 3.14159265358f;

}

