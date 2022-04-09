#pragma once
#include <string>
#include <optional>

namespace engine {
namespace utils {
namespace file {

std::optional<std::string> read(const std::string &path);

}}}

