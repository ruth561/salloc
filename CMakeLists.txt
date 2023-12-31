cmake_minimum_required(VERSION 3.8)
project(salloc)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

build_library(salloc
  src/salloc.cpp
  src/utils.cpp)

test_library(test_salloc
  src/salloc.cpp
  src/utils.cpp)

ament_add_gtest(test_heaphook_by_salloc ../../test/test_heaphook.cpp src/salloc.cpp src/utils.cpp ${HEAPHOOK_SOURCES})
target_include_directories(test_heaphook_by_salloc
  PRIVATE ${heaphook_SOURCE_DIR}/include)

ament_package()
