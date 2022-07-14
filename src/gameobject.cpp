#include <assert.h>
#include <unordered_map>
#include "components/component.h"
#include "gameobject.h"
namespace engine {

gameobject::gameobject(int id, const std::string &n)
{
	id_ = id;
	name = n;
	parent = nullptr;
}

gameobject::~gameobject()
{
	for (auto c:components) {
		delete c;
	}
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
gameobject::get_component(const std::string &name)
{
	for (auto c:components) {
		if (c->name() == name)
			return c;
	}
	return nullptr;
}
	
const std::vector<component *> 
gameobject::get_all_components() const
{
	return components;
}

void
gameobject::remove_component(const std::string &name)
{
	std::erase_if(components, [&name](component *c) {return c->name() == name;});
}

void
gameobject::start()
{
	for (auto c:components)
		c->start();
}

void
gameobject::pre_tick(float delta)
{
	for (auto c:components)
		c->pre_tick(delta);
}

void
gameobject::post_tick(float delta)
{
	for (auto c:components)
		c->post_tick(delta);
}

void
gameobject::tick(float delta)
{
	for (auto c:components)
		c->tick(delta);
}

}

