cmake_minimum_required (VERSION 3.8)

# Strip debug symbols
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
set (CMAKE_C_VISIBILITY_PRESET hidden)
set (CMAKE_CXX_VISIBILITY_PRESET hidden)
set (CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Allow threading
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

add_executable (rosaserversatellite main.cpp)

set_property (TARGET rosaserversatellite PROPERTY CXX_STANDARD 17)

target_link_libraries (rosaserversatellite Threads::Threads)
target_link_libraries (rosaserversatellite ${CMAKE_SOURCE_DIR}/LuaJIT/src/libluajit.so)
include_directories (${CMAKE_SOURCE_DIR}/LuaJIT/src)
include_directories (${CMAKE_SOURCE_DIR}/shared)
include_directories (${CMAKE_SOURCE_DIR}/sol2/include)
include_directories (${CMAKE_SOURCE_DIR}/lib)