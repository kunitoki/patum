cmake_minimum_required(VERSION 3.24)

file(GLOB_RECURSE TEST_FILES
	${CMAKE_CURRENT_LIST_DIR}/source/*.cpp
	${CMAKE_CURRENT_LIST_DIR}/source/*.h
	${CMAKE_CURRENT_LIST_DIR}/externals/*.cc
	${CMAKE_CURRENT_LIST_DIR}/externals/*.h)

foreach(FILE ${TEST_FILES})
	get_filename_component(PARENT_DIR "${FILE}" PATH)
	if(NOT "${PARENT_DIR}" STREQUAL "")
  		string(REPLACE "${CMAKE_CURRENT_LIST_DIR}" "" PARENT_DIR_STRIPPED "${PARENT_DIR}")
  		string(REPLACE "/" "\\\\" GROUP "${PARENT_DIR_STRIPPED}")
  		source_group("${GROUP}" FILES "${FILE}")
	endif()
endforeach()

add_executable(patum_tests ${TEST_FILES})
target_compile_features(patum_tests PRIVATE cxx_std_20)

target_compile_definitions(patum_tests PRIVATE
	SNITCH_DEFAULT_WITH_COLOR=0)

target_link_libraries(patum_tests PRIVATE patum)

target_include_directories(patum_tests PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/../include"
    "${CMAKE_CURRENT_LIST_DIR}/externals/re2-2023.3.1"
	"${CMAKE_CURRENT_LIST_DIR}/externals/snitch-1.0.0")

set_target_properties(patum_tests PROPERTIES
	CXX_VISIBILITY_PRESET "hidden"
	VISIBILITY_INLINES_HIDDEN ON
	COMPILE_WARNING_AS_ERROR ON)

if (lto_supported)
	set_target_properties(patum_tests PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
endif()
