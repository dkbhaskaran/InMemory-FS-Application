cmake_minimum_required(VERSION 3.2.0)

project(InMemoryApp)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
link_directories(${LLVM_LIBRARY_DIRS})

set(CLANG_LIBS clangFrontendTool)
llvm_map_components_to_libnames(llvm_libs support ${LLVM_TARGETS_TO_BUILD})

foreach (BIN InMemoryApp)
  add_executable("${BIN}" "${BIN}.cpp")
  set_target_properties("${BIN}" PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED Yes
    CXX_EXTENSIONS No)

  target_compile_options("${BIN}" PRIVATE -fno-rtti)
  target_link_libraries("${BIN}" ${llvm_libs} ${CLANG_LIBS})
endforeach()

