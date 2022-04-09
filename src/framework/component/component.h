#pragma once
#include <string>
namespace engine {

class gameobject;

class component {
protected:
	gameobject *go = nullptr;
public:
	component(gameobject *parent): go(parent) {}
	component() {}
	virtual ~component() { parent = nullptr; }
public:
	virtual const std::string type() = 0;
	virtual void tick(float delta) {};
	virtual void destroy() {};
};

};
