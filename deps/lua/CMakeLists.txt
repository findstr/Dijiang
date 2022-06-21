cmake_minimum_required(VERSION 3.22)
PROJECT(lua)
set(LUA_LIB_SRCS
    "lapi.c"
    "lcode.c"
    "lctype.c"
    "ldebug.c"
    "ldo.c"
    "ldump.c"
    "lfunc.c"
    "lgc.c"
    "llex.c"
    "lmem.c"
    "lobject.c"
    "lopcodes.c"
    "lparser.c"
    "lstate.c"
    "lstring.c"
    "ltable.c"
    "ltm.c"
    "lundump.c"
    "lvm.c"
    "lzio.c"
    "lauxlib.c"
    "lbaselib.c"
    "lcorolib.c"
    "ldblib.c"
    "liolib.c"
    "lmathlib.c"
    "loadlib.c"
    "loslib.c"
    "lstrlib.c"
    "ltablib.c"
    "lutf8lib.c"
    "linit.c"
)

if(LUA_BUILD_AS_CXX)
    set_source_files_properties(${LUA_LIB_SRCS} "src/lua.c" "src/luac.c" PROPERTIES LANGUAGE CXX )
endif()

if (CMAKE_HOST_WIN32)
	MESSAGE(STATUS "LUA Win32")
	set(CMAKE_C_FLAGS "-DLUA_BUILD_AS_DLL")
elseif(CMAKE_HOST_APPLE)
	add_compile_options(-DLUA_USE_MACOSX -DLUA_USE_READLINE)
elseif(CMAKE_HOST_UNIX)

endif()

add_library(lua_internal INTERFACE)
add_library(lua_include INTERFACE)

target_include_directories(lua_include INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lua_internal INTERFACE lua_include)
add_library(lua SHARED ${LUA_LIB_SRCS})