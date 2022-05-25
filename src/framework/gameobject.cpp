#include "components/component.h"
#include "gameobject.h"
namespace engine {

gameobject::~gameobject()
{
	for (auto c:components)
		delete c;
	components.clear();

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

