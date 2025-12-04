cmake_minimum_required(VERSION 3.24)

project(patum)

# ===========================================================================

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error_message)
if (lto_supported)
    message(STATUS "LTO enabled")
else()
    message(STATUS "LTO not supported: <${lto_error_message}>")
endif()

# ===========================================================================

if (WIN32)
    set(platform "win")
elseif (APPLE)
	set(platform "macos")
else()
	set(platform "linux")
endif()

# ===========================================================================

if (WIN32)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_DEPRECATE /DNOMINMAX /permissive- /MP")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_DEPRECATE /DNOMINMAX /permissive- /MP")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd /Od")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT /Ox /Oy")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /Ox /Oy")
elseif (APPLE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftemplate-backtrace-limit=0")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer")
	set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
	set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")
else()
	set(BASE_FLAGS "-Wl,--as-needed -Wl,--whole-archive -lrt -lpthread -Wl,--no-whole-archive")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${BASE_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${BASE_FLAGS} -ftemplate-backtrace-limit=0")
	set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} ${BASE_FLAGS}")
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -ggdb")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -fomit-frame-pointer  -fPIC")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -fomit-frame-pointer  -fPIC")
endif()

# ===========================================================================

set(PATUM_PUBLIC_DEFINITIONS "")

file(GLOB_RECURSE LIBRARY_FILES
	${CMAKE_CURRENT_LIST_DIR}/include/*.h)

foreach(FILE ${LIBRARY_FILES})
	get_filename_component(PARENT_DIR "${FILE}" PATH)
	if(NOT "${PARENT_DIR}" STREQUAL "")
  		string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" PARENT_DIR_STRIPPED "${PARENT_DIR}")
  		string(REPLACE "/" "\\\\" GROUP "${PARENT_DIR_STRIPPED}")
  		source_group("${GROUP}" FILES "${FILE}")
	endif()
endforeach()

add_library(patum INTERFACE ${LIBRARY_FILES})
target_compile_features(patum INTERFACE cxx_std_20)
target_compile_definitions(patum INTERFACE ${PATUM_PUBLIC_DEFINITIONS})

add_subdirectory(benchmarks)
add_subdirectory(tests)
