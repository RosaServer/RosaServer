cmake_minimum_required (VERSION 3.8)

# Strip debug symbols
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set (CMAKE_C_VISIBILITY_PRESET hidden)
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_VISIBILITY_INLINES_HIDDEN ON)

add_compile_definitions (
	SUBHOOK_SEPARATE_SOURCE_FILES
	SUBHOOK_IMPLEMENTATION
)

# Allow threading
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

set (OPENSSL_USE_STATIC_LIBS TRUE)
find_package (OpenSSL REQUIRED)

add_library (rosaserver SHARED
	api.cpp
	childprocess.cpp
	console.cpp
	engine.cpp
	filewatcher.cpp
	hooks.cpp
	image.cpp
	rosaserver.cpp
	subhook.c
	subhook_unix.c
	subhook_x86.c
	worker.cpp
)

set_property (TARGET rosaserver PROPERTY CXX_STANDARD 17)

target_link_libraries (rosaserver Threads::Threads)
target_link_libraries (rosaserver stdc++fs)
target_link_libraries (rosaserver OpenSSL::SSL)
target_link_libraries (rosaserver OpenSSL::Crypto)
target_link_libraries (rosaserver ${CMAKE_SOURCE_DIR}/moonjit/src/libluajit.so)
include_directories (${CMAKE_SOURCE_DIR}/moonjit/src)
include_directories (${CMAKE_SOURCE_DIR}/shared)
include_directories (${CMAKE_SOURCE_DIR}/sol2/include)
include_directories (${CMAKE_SOURCE_DIR}/lib)