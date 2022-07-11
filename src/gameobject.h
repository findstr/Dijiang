#pragma once
#include <memory>
#include <string>
#include <vector>
#include "math/transform.h"

namespace engine {

class component;

class gameobject {
public:
	typedef unsigned int id_t;
protected:
	id_t	id_;
	bool active = true;
	std::string name;
	gameobject *parent;
	std::vector<component *> components;
public:
	gameobject(int id, const std::string &name);
	~gameobject();
	transform transform;
	void set_parent(gameobject *go, bool keep_world = false);
	constexpr gameobject *get_parent() const { return parent;}
	constexpr const std::string &get_name() const { return name; }
	constexpr void set_name(const std::string &n) { name = n; }
	constexpr id_t id() const { return id_; }
	constexpr bool get_active() const { return active; }
	constexpr void set_active(bool enable) { active = enable; }
public:
	void add_component(component *);
	component *get_component(const std::string &type);
	const std::vector<component *> get_all_components() const;
	void remove_component(const std::string &type);
	void start();
	void tick(float delta);
	void pre_tick(float delta);
	void post_tick(float delta);
};

}

