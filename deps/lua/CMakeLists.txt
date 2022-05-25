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

add_library(lua_internal INTERFACE)

add_library(lua_include INTERFACE)

target_include_directories(lua_include INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(lua_internal INTERFACE lua_include)

if(LUA_ENABLE_SHARED)
    add_library(lua_shared SHARED ${LUA_LIB_SRCS})
    target_link_libraries(lua_shared PRIVATE lua_internal PUBLIC lua_include)
    set_target_properties(lua_shared PROPERTIES
        VERSION "${PACKAGE_VERSION}"
    )
    if(WIN32)
        target_compile_definitions(lua_shared PRIVATE LUA_BUILD_AS_DLL)
    endif()
    list(APPEND TARGETS_TO_INSTALL lua_shared)
    if(BUILD_SHARED_LIBS)
        add_library(Lua::Library ALIAS lua_shared)
    elseif(NOT TOP_LEVEL)
        set_target_properties(lua_shared PROPERTIES
            EXCLUDE_FROM_ALL ON
        )
    endif()
endif()

add_library(lua STATIC ${LUA_LIB_SRCS})

