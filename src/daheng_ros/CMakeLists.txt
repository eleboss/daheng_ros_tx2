cmake_minimum_required(VERSION 2.8.3)
project(daheng_ros)
add_definitions(
    -std=c++11
)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  cv_bridge
  image_transport
  camera_info_manager
  sensor_msgs
)


find_package(OpenCV)

catkin_package()

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)


add_executable(daheng src/daheng.cpp)
target_link_libraries(daheng ${catkin_LIBRARIES} ${OpenCV_LIBRARIES}
pthread
gxiapi
)



install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
