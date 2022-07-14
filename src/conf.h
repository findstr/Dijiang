#pragma once
namespace engine {
namespace conf {
	const bool enable_validate = true;
	const int MAX_FRAMES_IN_FLIGHT = 3;
	const int VERSION_MAJOR = 1;
	const int VERSION_MINOR = 0;
}}

#ifdef __WINDOWS__ 
#define ENGINE_API	__declspec(dllexport) 
#else
#define ENGINE_API
#endif
