#include <string>
#include <iostream>
#include <fstream>
#include "file.h"
namespace engine {
namespace utils {
namespace file {

std::optional<std::string>
read(const std::string &path)
{
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open())
		return std::nullopt;
	std::string buffer;
	size_t fileSize = (size_t) file.tellg();
	buffer.resize(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

}}}

