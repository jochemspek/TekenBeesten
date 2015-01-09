cmake_minimum_required(VERSION 3.1)
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
