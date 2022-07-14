#include <array>
#include "imgui.h"
#include "imgui_internal.h"
#include "level.h"
#include "hierarchy.h"
#include "system/render_system.h"
#include "render/debugger.h"

namespace engine {
namespace editor {

hierarchy::go_inspector::go_inspector()
{
	newname.reserve(128);
}
		
void
hierarchy::go_inspector::attach(gameobject *obj)
{
	go = obj;
	newname = go->get_name();
}

void
hierarchy::go_inspector::draw() 
{
	if (go == nullptr)
		return ;
	bool active = go->get_active();
	if (ImGui::Checkbox("##Active", &active)) {
		go->set_active(active);
	}
	ImGui::SameLine();
	if (ImGui::InputText("##Name", newname.data(), newname.capacity())) {
		newname.resize(strlen(newname.data()));
		go->set_name(newname);
	}
	
	auto &all_components = go->get_all_components();
	for (auto *c: all_components) {
		ImGui::Separator();
		ImGui::LabelText("", c->name().c_str());
	}
}

hierarchy::hierarchy() :
	title("Hierarchy")
{

}

void
hierarchy::pre_tick(engine *e, float delta)
{
}

void 
hierarchy::draw_recursive(gameobject *go)
{
	auto &children = tree[go];
	ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (go->id() == selected)
		base_flags |= ImGuiTreeNodeFlags_Selected;
	if (children.size() > 0) {
		if (ImGui::TreeNode(go->get_name().c_str())) {
			for (auto *go : children) 
				draw_recursive(go);
			ImGui::TreePop();
		}
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			inspector.attach(go);
			selected = go->id();
			is_selected = true;
		}
	} else {
		auto node_flags = base_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		ImGui::TreeNodeEx(go->get_name().c_str(), node_flags);
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
			inspector.attach(go);
			selected = go->id();
			is_selected = true;
		}
	}
}

void
hierarchy::draw_level(level *l)
{
	auto &all = l->all_object();
	for (auto &iter:all) {
		auto go = iter.second;
		auto *p = go->get_parent();
		if (p == nullptr)
			root.emplace_back(go);
		else
			tree[p].emplace_back(p);
	}
	if (ImGui::TreeNode(l->name().c_str())) {
		for (auto go : root) 
			draw_recursive(go);
		ImGui::TreePop();
	}
	root.clear();
	tree.clear();
}

inspector *
hierarchy::tick(engine *e, float delta)
{
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
	if (!is_open)
		return nullptr;
	is_selected = false;
	ImGui::Begin(title.c_str(), &is_open, window_flags);
	auto &all_levels = level::all_levels();
	for (auto &l : all_levels) {
		auto *lptr = l.get();
		draw_level(lptr);
	}
	ImGui::End();
	return is_selected ? &inspector : nullptr;
}


}}

