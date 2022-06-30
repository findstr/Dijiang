#include <string>
#include <iostream>
#include <fstream>
#include "file.h"
namespace engine {
namespace utils {
namespace file {

bool 
exist(const std::string &path) 
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

void 
save(const std::string &path, const std::string &data)
{
	std::ofstream output;
	output.open(path);
	output << data;
	output.close();
}

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

