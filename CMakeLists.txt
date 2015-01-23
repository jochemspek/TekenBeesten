cmake_minimum_required(VERSION 2.8)
set(CMAKE_PREFIX_PATH /usr/local/opencv-3)
find_package(OpenCV REQUIRED)
find_package(cvBlob REQUIRED)


Project(TekenBeesten)

set (TekenBeesten_VERSION_MAJOR 1)
set (TekenBeesten_VERSION_MINOR 0)
set (LIBRARIES common GL glut m X11 boost_thread boost_system ${OpenCV_LIBS} ${cvBlob_LIBS})

add_subdirectory (common_osx)
include_directories(${TekenBeesten_SOURCE_DIR}/common_osx)

set(TEKENBEESTEN_SRC src/TekenBeesten.cpp src/tracking.cpp)
set_source_files_properties(${TEKENBEESTEN_SRC}
                            PROPERTIES
                            COMPILE_FLAGS "-O3")
add_executable(tekenbeesten ${TEKENBEESTEN_SRC})

target_link_libraries(tekenbeesten ${LIBRARIES})
install(TARGETS tekenbeesten DESTINATION bin)

