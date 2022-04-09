#include "gameobject.h"
namespace engine {

gameobject::~gameobject()
{

}

template<derived<component> T> void
gameobject::add_component()
{

}

template<derived<component> T> T *
gameobject::get_component(const std::string &type)
{

}

void
gameobject::remove_component(const std::string &type)
{

}

void
gameobject::tick(float delta)
{

}

}

