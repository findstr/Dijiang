#pragma once
#include <string>
namespace engine {

class gameobject;

namespace luavm {

void init();
void collect(int handle);
int new_component(gameobject *go, const std::string &name);
int get_func(int handle, const std::string &name);
void call_with_self(int self, int func, float value);
void call_with_self(int self, const std::string &func);
void set_table(int table, const std::string &key, float value);
void set_table(int table, const std::string &key, const std::string &value);

}}

