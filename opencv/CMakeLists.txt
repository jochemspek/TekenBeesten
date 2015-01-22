cmake_minimum_required(VERSION 2.8)
set(CMAKE_PREFIX_PATH /usr/local/opencv-3)

find_package(OpenCV REQUIRED)
find_package(cvBlob REQUIRED)

# TEST TRACKING
set(TEST_TRACKING_SRC test_tracking.cpp)
set_source_files_properties(${TEST_TRACKING_SRC}
                            PROPERTIES
                            COMPILE_FLAGS "-O3"
)
add_executable(test_tracking ${TEST_TRACKING_SRC})
target_link_libraries(test_tracking ${OpenCV_LIBS} ${cvBlob_LIBS})

set(BG_SUB_SRC bg_sub.cpp)
add_executable(bg_sub ${BG_SUB_SRC})
target_link_libraries(bg_sub ${OpenCV_LIBS} ${cvBlob_LIBS})


set(BG_SUB_CAM_SRC bgsub_cam.cpp)
add_executable(bgsub_cam ${BG_SUB_CAM_SRC})
set_source_files_properties(${BG_SUB_CAM_SRC}
                            PROPERTIES
                            COMPILE_FLAGS "-g"
)
target_link_libraries(bgsub_cam ${OpenCV_LIBS} ${cvBlob_LIBS})
