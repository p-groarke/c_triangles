#include <stdio.h>
#include <stdbool.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <OpenGL/gl3.h>

#include "triangle_shader.h"

CGLContextObj gl_context;

void check_error(CGLError err) {
	if (err == kCGLNoError)
		return;

	printf("Error : %s\n", CGLErrorString(err));
}

void init_opengl()
{
	/* Display */
	CGOpenGLDisplayMask display_mask = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);

	/* Context */
	CGLPixelFormatObj pix;
	GLint npix;

	CGLPixelFormatAttribute attribs[] = {
		kCGLPFADisplayMask, display_mask,
		kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADoubleBuffer,
		kCGLPFASampleBuffers, 1,
		kCGLPFASamples, 2,
		kCGLPFAAccelerated,
		kCGLPFAAcceleratedCompute,
		0
	};


	/* Context init. */
	check_error(CGLChoosePixelFormat(attribs, &pix, &npix));
	check_error(CGLCreateContext(pix, NULL, &gl_context));
	CGLReleasePixelFormat(pix);
	check_error(CGLClearDrawable(gl_context));
	check_error(CGLSetCurrentContext(gl_context));
	/*check_error(CGLLockContext(gl_context));*/

	/* FIXME: Deprecated, find another way to grab screen. */
	/* WTF: Can't use previously assigned display_mask... */
	check_error(CGLSetFullScreenOnDisplay(gl_context,
			CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay)));
}

void render_triangle()
{
	GLfloat vertices[] = {
		-0.5, -0.5, 0.0,
		0.5, -0.5, 0.0,
		0.0, 0.5, 0.0
	};

	/* VAO stuff */
	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	/* VBO stuff */
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* Attribute stuff */
	/* FIXME: UNDERSTAND THIS SHIT */
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
			(GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0); // Unbind because we could misconfigure.

	/* Shader stuff */
	GLuint shader_program = glCreateProgram();

	GLuint vertex_shader_id;
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_id, 1, &vertexShaderSource, NULL);
	glCompileShader(vertex_shader_id);
	glAttachShader(shader_program, vertex_shader_id);

	GLint s;
	GLchar info_log[512];
	glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &s);
	if (!s) {
		glGetShaderInfoLog(vertex_shader_id, 512, NULL, info_log);
		printf("Error compiling shader : %s", info_log);
	}

	GLuint fragment_shader_id;
	fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_id, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragment_shader_id);
	glAttachShader(shader_program, fragment_shader_id);

	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &s);
	if (!s) {
		glGetShaderInfoLog(vertex_shader_id, 512, NULL, info_log);
		printf("Error compiling shader : %s", info_log);
	}

	glLinkProgram(shader_program);
	glUseProgram(shader_program);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &s);
	if (!s) {
		glGetShaderInfoLog(shader_program, 512, NULL, info_log);
		printf("Error compiling shader : %s", info_log);
	}

	while(true) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		check_error(CGLFlushDrawable(gl_context));
	}
}

int main(int argc, char** argv) {
	printf("\n  A CGL hello triangle.\n* Use cmd+alt+escape to exit! *\n\n");

	init_opengl();
	render_triangle();

}
