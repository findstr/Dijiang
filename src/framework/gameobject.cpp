#include <assert.h>
#include <unordered_map>
#include "components/component.h"
#include "gameobject.h"
namespace engine {

gameobject::gameobject(int id)
{
	id_ = id;
	parent = nullptr;
}

gameobject::~gameobject()
{
	for (auto c:components)
		delete c;
	components.clear();
}

void
gameobject::set_parent(gameobject *go, bool keep_world)
{
	parent = go;
	if (go == nullptr)
		return ;
	if (keep_world) {
		transform.local_position =
			transform.position - go->transform.position;
	} else {
		transform.position = go->transform.position + transform.local_position;
	}
	//TODO: adjust scale, local_scale, rotation, local_rotation
}

void
gameobject::add_component(component *c)
{
	components.emplace_back(c);
}

component *
gameobject::get_component(const std::string &type)
{
	for (auto c:components) {
		if (c->type() == type)
			return c;
	}
	return nullptr;
}

void
gameobject::remove_component(const std::string &type)
{
	std::erase_if(components, [&type](component *c) {return c->type() == type;});
}

void
gameobject::start()
{
	for (auto c:components)
		c->start();
}

void
gameobject::tick(float delta)
{
	for (auto c:components)
		c->tick(delta);
}

}

