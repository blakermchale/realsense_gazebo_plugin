cmake_minimum_required(VERSION 3.5)
project(realsense_gazebo_plugin)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # uncomment the line when a copyright and license is not present in all source files
  #set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # uncomment the line when this package is not in a git repo
  #set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()


find_package(Boost REQUIRED COMPONENTS thread)
find_package(gazebo REQUIRED)
find_package(Threads REQUIRED)
find_package(gazebo_ros REQUIRED)
find_package(camera_info_manager REQUIRED)
find_package(rclcpp REQUIRED)
find_package(image_transport REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cv_bridge REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GAZEBO_CXX_FLAGS}")

include_directories(
    include
    ${Boost_INCLUDE_DIR}
    ${GAZEBO_INCLUDE_DIRS}
    ${gazebo_ros_INCLUDE_DIRS}
    #${sensor_msgs_INCLUDE_DIRS}
    #${cv_bridge_INCLUDE_DIRS}
)
link_directories(${GAZEBO_LIBRARY_DIRS})

add_library(
    ${PROJECT_NAME}
    SHARED
    src/RealSensePlugin.cpp
    src/gazebo_ros_realsense.cpp
)
target_link_libraries(${PROJECT_NAME} ${GAZEBO_LIBRARIES})
#add_dependencies(${PROJECT_NAME})

ament_target_dependencies(${PROJECT_NAME}
  "gazebo_ros"
  "rclcpp"
  "sensor_msgs"
  "camera_info_manager"
  "image_transport"
  "cv_bridge"
)
ament_export_libraries(${PROJECT_NAME})

ament_export_include_directories(include)
ament_export_dependencies(rclcpp)
#ament_export_dependencies(gazebo_dev)
ament_export_dependencies(gazebo_ros)

#add_executable(test_realsense_streams test/realsense_streams_test.cpp)
#target_link_libraries(
#    test_realsense_streams
#    ${catkin_LIBRARIES}
#    ${GTEST_LIBRARIES}
#    ${CMAKE_THREAD_LIBS_INIT}
#)
#add_dependencies(test_realsense_streams ${catkin_EXPORTED_TARGETS})

install(
    TARGETS
        ${PROJECT_NAME}
    DESTINATION bin/$PROJECT_NAME}/
    LIBRARY DESTINATION lib/$PROJECT_NAME}/
)


ament_package()
