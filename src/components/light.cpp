#include "level.h"
#include "light.h"
#include "system/render_system.h"

namespace engine {

light::light(gameobject *go) : component(go)
{
}

light::~light()
{
	if (regged)
		unreg();
}

void 
light::reg()
{
	light_list.emplace_back(this);
	regged = true;
}

void
light::unreg()
{
	if (light_list.size() > 0)
		std::erase(light_list, this);
}

 
std::vector<light *> light::light_list;
const std::vector<light *> &
light::all_lights()
{
	return light_list;
}


}


