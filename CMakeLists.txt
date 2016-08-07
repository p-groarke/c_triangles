cmake_minimum_required(VERSION 3.5.0)
project(c_triangles)

# Include uses based paths, start at src/.
#include_directories(src)

# Add sources using GLOB pattern.
#file(GLOB_RECURSE SOURCES "src/*.c")

# Build target.
#add_executable(c_engine ${SOURCES})

# C11
#set_property(TARGET c_engine PROPERTY C_STANDARD 11)

# Do OS X application stuff.
if (APPLE)
	set(OSX_OPENGL_SRC src/osx_opengl.c)
	add_executable(osx_opengl ${OSX_OPENGL_SRC})
	set_property(TARGET osx_opengl PROPERTY C_STANDARD 11)


	find_library(OGL OpenGL)
	find_library(CG CoreGraphics)
	target_link_libraries(osx_opengl ${OGL})
	target_link_libraries(osx_opengl ${CG})

	# Linker step.
	#target_link_libraries(VoronoiEve "${PROJECT_SOURCE_DIR}/libs/osx/libboost_system-mt.a")

endif(APPLE)

