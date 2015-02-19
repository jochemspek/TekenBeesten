Project(TekenBeesten)

cmake_minimum_required(VERSION 2.8)

SET(PROJECT_NAME TekenBeesten)
SET(CMAKE_INCLUDE_CURRENT_DIR TRUE)
# SET(CMAKE_PREFIX_PATH /usr/local/opencv-3)

FIND_PACKAGE(OpenCV REQUIRED)
FIND_PACKAGE(cvBlob REQUIRED)
FIND_PACKAGE(Boost 1.57.0 COMPONENTS program_options thread system )

IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OPENGL_LIBRARY OpenGL )
   MARK_AS_ADVANCED (GLUT_LIBRARY OpenGL_LIBRARY)
   INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
   SET(EXTRA_LIBS ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
   SET(LIBRARIES common m ${Boost_LIBRARIES}  ${OpenCV_LIBS} ${cvBlob_LIBS} ${OPENGL_LIBRARY} ${GLUT_LIBRARY} )
ELSE (APPLE)
	find_package(GLUT)
	find_package(OpenGL)
	if (OPENGL_FOUND)
    	MESSAGE("OpenGL Correctly Found")
		include_directories(${OPENGL_INCLUDE_DIR})
	else (OPENGL_FOUND)
	    MESSAGE("OpenGL environment missing")
	endif (OPENGL_FOUND)
   SET(LIBRARIES common GL glut X11 m boost_thread boost_system ${OpenCV_LIBS} ${cvBlob_LIBS})
ENDIF (APPLE)


set (TekenBeesten_VERSION_MAJOR 1)
set (TekenBeesten_VERSION_MINOR 0)

add_subdirectory (common_osx)
include_directories(${TekenBeesten_SOURCE_DIR}/common_osx)

set(TEKENBEESTEN_SRC src/TekenBeesten.cpp src/tracking.cpp src/settings.cpp)
set_source_files_properties(${TEKENBEESTEN_SRC} PROPERTIES COMPILE_FLAGS "-O3")
add_executable(tekenbeesten ${TEKENBEESTEN_SRC})

set(VIDEO_TEST_SRC src/video_test.cpp)
set_source_files_properties(${VIDEO_TEST_SRC} PROPERTIES COMPILE_FLAGS "-O3")
add_executable(video_test ${VIDEO_TEST_SRC})

target_link_libraries(video_test ${LIBRARIES})
target_link_libraries(tekenbeesten ${LIBRARIES})

install(TARGETS tekenbeesten video_test DESTINATION bin)
