cmake_minimum_required(VERSION 3.5)
project(vehicle_interface)

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# Library
ament_auto_add_library(${PROJECT_NAME} SHARED
  src/platform_interface.cpp
  src/safety_state_machine.cpp
  src/vehicle_interface_node.cpp)
autoware_set_compile_options(${PROJECT_NAME})

if(BUILD_TESTING)
  # Lint
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  # Test
  ament_add_gtest(${PROJECT_NAME}_test
    test/gtest_main.cpp
    test/error_handling.cpp
    test/filtering.cpp
    test/sanity_checks.cpp
    test/state_machine.hpp
    test/state_machine.cpp
    test/state_machine_bad_state.cpp
    test/state_machine_clamp.cpp
    test/state_machine_gear.cpp
    test/state_machine_headlight.cpp
    test/state_machine_node.cpp
    test/test_vi_node.hpp)
  target_include_directories(${PROJECT_NAME}_test PRIVATE "include")
  target_link_libraries(${PROJECT_NAME}_test ${PROJECT_NAME})
  ament_target_dependencies(vehicle_interface_test
    "autoware_auto_msgs"
    "rclcpp"
    "reference_tracking_controller"
    "signal_filters")
endif()

ament_auto_package(INSTALL_TO_SHARE param)
