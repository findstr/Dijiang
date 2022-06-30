#pragma once
#include <string>
#include <optional>

namespace engine {
namespace utils {
namespace file {

bool exist(const std::string &path);
void save(const std::string &path, const std::string &data);
std::optional<std::string> read(const std::string &path);

}}}

