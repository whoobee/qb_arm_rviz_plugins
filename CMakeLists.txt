cmake_minimum_required(VERSION 3.8)
project(qb_arm_rviz_plugins)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rviz_common REQUIRED)
find_package(rviz_rendering REQUIRED)
find_package(std_srvs REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

add_library(qb_arm_rviz_plugins SHARED
  src/trigger_gemini_panel.cpp
  include/qb_arm_rviz_plugins/trigger_gemini_panel.hpp
)

target_include_directories(qb_arm_rviz_plugins PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(qb_arm_rviz_plugins
  Qt5::Widgets
)

ament_target_dependencies(qb_arm_rviz_plugins
  rclcpp
  rviz_common
  rviz_rendering
  std_srvs
)

# Use moc for Qt signals/slots
set_target_properties(qb_arm_rviz_plugins PROPERTIES AUTOMOC ON)

ament_export_libraries(qb_arm_rviz_plugins)
ament_export_dependencies(rclcpp rviz_common rviz_rendering std_srvs)

pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)

install(
  TARGETS qb_arm_rviz_plugins
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  FILES plugin_description.xml
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
