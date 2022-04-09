#pragma once
#include <memory>
#include <string>
#include "math/transform.h"

namespace engine {

typedef unsigned int go_id_t;

class gameobject {
public:
	template <class T, class U>
	concept derived= std::is_base_of<U, T>::value;
protected:
	go_id_t	id;
	std::vector<std::unique_ptr<component>> components;
public:
	GObject(go_id_t n) : id(n) {}
	~GObject();
	transform transform;
public:
	template<derived<component> T> void add_component();
	template<derived<component> T> T *get_component(const std::string &type);
	void remove_component(const std::string &type);
	void tick(float delta);
}

}

