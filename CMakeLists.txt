cmake_minimum_required(VERSION 3.5.0)
project(c_triangles)

if (APPLE)
	set(OSX_OPENGL_SRC src/osx_opengl.c)
	add_executable(osx_opengl ${OSX_OPENGL_SRC})
	set_property(TARGET osx_opengl PROPERTY C_STANDARD 11)


	find_library(OGL OpenGL)
	find_library(CG CoreGraphics)
	target_link_libraries(osx_opengl ${OGL})
	target_link_libraries(osx_opengl ${CG})
endif(APPLE)

if (WIN32)
	# OpenGL
	set(WIN_OPENGL_SRC src/win_opengl.c)
	add_executable(win_opengl WIN32 ${WIN_OPENGL_SRC})
	set_property(TARGET win_opengl PROPERTY C_STANDARD 11)

	find_package(OpenGL REQUIRED)
	target_link_libraries(win_opengl ${OPENGL_LIBRARIES})
	#set(CMAKE_C_FLAGS "/Gz /fp:except- /Gy /Oi /GF /Os /fp:fast /Ob1 /Oy /O2 /MT /GS- /Zp1")
	#set_property(TARGET win_opengl PROPERTY LINK_FLAGS "/ENTRY:\"entrypoint\" /NODEFAULTLIB /DYNAMICBASE:NO /MACHINE:X86 /MANIFEST:NO")

	# Vulkan
	set(WIN_VULKAN_SRC src/win_vulkan.c)
	add_executable(win_vulkan ${WIN_VULKAN_SRC})
	set_property(TARGET win_vulkan PROPERTY C_STANDARD 11)

	find_library(vul vulkan-1)
	target_link_libraries(win_vulkan ${vul})
	include_directories($ENV{VULKAN_SDK}/Include)

endif(WIN32)
