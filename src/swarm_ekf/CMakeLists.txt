cmake_minimum_required(VERSION 3.8)
project(swarm_ekf)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(rclcpp REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(Eigen3 REQUIRED)

add_executable(ekf_node src/ekf_node.cpp)

target_include_directories(ekf_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${EIGEN3_INCLUDE_DIRS})

ament_target_dependencies(ekf_node rclcpp px4_msgs)

install(TARGETS ekf_node
  DESTINATION lib/${PROJECT_NAME})

ament_package()
