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
	id_t	id = 0;
	std::vector<component *> components;
public:
	gameobject() {}
	~gameobject();
	transform transform;
public:
	void add_component(component *);
	component *get_component(const std::string &type);
	void remove_component(const std::string &type);
	void start();
	void tick(float delta);
};

}

