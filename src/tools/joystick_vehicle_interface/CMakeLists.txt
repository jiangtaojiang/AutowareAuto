cmake_minimum_required(VERSION 3.5)
project(joystick_vehicle_interface)

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
# Node library
ament_auto_add_library(joystick_vehicle_interface_node src/joystick_vehicle_interface_node.cpp)
autoware_set_compile_options(joystick_vehicle_interface_node)
# Node executable
ament_auto_add_executable(joystick_vehicle_interface_exe src/joystick_vehicle_interface_main.cpp)
autoware_set_compile_options(joystick_vehicle_interface_exe)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  # Test
  ament_add_gtest(joystick_vehicle_interface_test
    test/gtest_main.cpp
    test/mapping.cpp)
  target_include_directories(joystick_vehicle_interface_test PUBLIC include)
  target_link_libraries(joystick_vehicle_interface_test joystick_vehicle_interface_node)
  ament_target_dependencies(joystick_vehicle_interface_test
    "rclcpp" "autoware_auto_msgs" "sensor_msgs")
endif()

ament_auto_package(INSTALL_TO_SHARE param launch)
