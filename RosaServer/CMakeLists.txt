cmake_minimum_required (VERSION 3.8)

add_compile_definitions (
	SUBHOOK_SEPARATE_SOURCE_FILES
	SUBHOOK_IMPLEMENTATION
)

# Allow threading
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

set (OPENSSL_USE_STATIC_LIBS TRUE)
find_package (OpenSSL REQUIRED)

if(NOT CMAKE_CROSSCOMPILING)
  find_package(PkgConfig QUIET)
  pkg_check_modules(PC_OPUS opus)
endif()

find_library(OPUS_LIBRARY
  NAMES opus
  HINTS ${HINTS_OPUS_LIBDIR} ${PC_OPUS_LIBDIR} ${PC_OPUS_LIBRARY_DIRS}
  PATHS ${PATHS_OPUS_LIBDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

find_path(OPUS_INCLUDEDIR opus.h
  PATH_SUFFIXES opus
  HINTS ${HINTS_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDEDIR} ${PC_OPUS_INCLUDE_DIRS}
  PATHS ${PATHS_OPUS_INCLUDEDIR}
  ${CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Opus DEFAULT_MSG OPUS_INCLUDEDIR)

mark_as_advanced(OPUS_INCLUDEDIR OPUS_LIBRARY)

if(OPUS_FOUND)
  set(OPUS_INCLUDE_DIRS ${OPUS_INCLUDEDIR})
  if(OPUS_LIBRARY)
    set(OPUS_LIBRARIES ${OPUS_LIBRARY})
  else()
    set(OPUS_LIBRARIES)
  endif()
endif()

add_library (rosaserver SHARED
	api.cpp
	childprocess.cpp
	console.cpp
	crypto.cpp
	engine.cpp
	filewatcher.cpp
	hooks.cpp
	image.cpp
	opusencoder.cpp
	pointgraph.cpp
	rosaserver.cpp
	sqlite.cpp
	tcpserver.cpp
	worker.cpp
	zlib.cpp
	../subhook/subhook.c
	../subhook/subhook_unix.c
	../subhook/subhook_x86.c
	../miniz/miniz.c
	../miniz/miniz_tinfl.c
	../miniz/miniz_tdef.c
)

set_property (TARGET rosaserver PROPERTY CXX_STANDARD 17)

target_link_libraries (rosaserver Threads::Threads)
target_link_libraries (rosaserver stdc++fs)
target_link_libraries (rosaserver OpenSSL::SSL)
target_link_libraries (rosaserver OpenSSL::Crypto)
target_link_libraries (rosaserver ${OPUS_LIBRARY})
target_link_libraries (rosaserver sqlite3)
target_link_libraries (rosaserver ${CMAKE_SOURCE_DIR}/moonjit/src/libluajit.so)
include_directories (${CMAKE_SOURCE_DIR}/moonjit/src)
include_directories (${CMAKE_SOURCE_DIR}/stb)
include_directories (${OPUS_INCLUDEDIR})
include_directories (${CMAKE_SOURCE_DIR}/shared)
include_directories (${CMAKE_SOURCE_DIR}/sol2/include)
include_directories (${CMAKE_SOURCE_DIR}/subhook)
include_directories (${CMAKE_SOURCE_DIR}/miniz)