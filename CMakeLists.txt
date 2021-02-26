cmake_minimum_required(VERSION 3.2.0)

project(InMemoryApp)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

set(CMAKE_BUILD_TYPE Release)
set(CLANG_LIBS clangFrontendTool)
llvm_map_components_to_libnames(llvm_libs support ${LLVM_TARGETS_TO_BUILD})

foreach (BIN InMemoryApp)
  add_executable("${BIN}" "${BIN}.cpp")
  set_target_properties("${BIN}" PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED Yes
    CXX_EXTENSIONS No)

set(COMPILE_OPTIONS)

if (UNIX)
  list(APPEND COMPILE_OPTIONS -fno-rtti -Wall -Wno-attributes)
else()
  list(APPEND AMD_COMGR_PRIVATE_COMPILE_OPTIONS "/W0" "/wd4244")
  list(APPEND AMD_COMGR_PRIVATE_COMPILE_DEFINITIONS _HAS_EXCEPTIONS=0)
endif()
 
  target_compile_options("${BIN}" PRIVATE ${COMPILE_OPTIONS})
  target_link_libraries("${BIN}" ${llvm_libs} ${CLANG_LIBS})
endforeach()

