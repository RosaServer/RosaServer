# CMakeList.txt : CMake project for RosaServer, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Strip debug symbols
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_compile_definitions(
	SUBHOOK_SEPARATE_SOURCE_FILES
	SUBHOOK_IMPLEMENTATION
)

# Allow threading
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library (rosaserver SHARED
	RosaServer.cpp
	api.cpp
	hooks.cpp
	subhook.c
	subhook_unix.c
	subhook_x86.c
	tinycon.cpp
)

set_property(TARGET rosaserver PROPERTY CXX_STANDARD 17)

target_link_libraries(rosaserver Threads::Threads)
target_link_libraries(rosaserver stdc++fs)
target_link_libraries(rosaserver ${CMAKE_SOURCE_DIR}/LuaJIT/src/libluajit.so)
include_directories(${CMAKE_SOURCE_DIR}/LuaJIT/src)