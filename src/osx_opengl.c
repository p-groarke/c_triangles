#include <stdio.h>
#include <stdbool.h>

#include <OpenGL/OpenGL.h>
#include <OpenGL/CGLTypes.h>
#include <CoreGraphics/CGDirectDisplay.h>
#include <OpenGL/gl3.h>

CGLContextObj gl_context;

const GLchar* vertexShaderSource = "#version 330 core\n"
	"layout (location = 0) in vec3 position;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
	"}\0";
const GLchar* fragmentShaderSource = "#version 330 core\n"
	"out vec4 color;\n"
	"void main() {\n"
	"color = vec4(1.0, 0.5, 0.2, 1.0);\n"
	"}\0";

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

	/* Tests and params. */
//	GLint vsync = 1;
//	check_error(CGLSetParameter(gl_context, kCGLCPSwapInterval, &vsync));
//	GLint opaque = 0.8;
//	check_error(CGLSetParameter (gl_context, kCGLCPSurfaceOpacity, &opaque));

	GLint fragGPU, vertGPU;
	CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUFragmentProcessing,
			&fragGPU);
	CGLGetParameter(CGLGetCurrentContext(), kCGLCPGPUVertexProcessing,
			&vertGPU);
	printf("GPU fragments : %d, GPU vertex : %d\n", fragGPU, vertGPU);


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
	GLuint vertex_shader_id;
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_id, 1, &vertexShaderSource, NULL);
	glCompileShader(vertex_shader_id);

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

	glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &s);
	if (!s) {
		glGetShaderInfoLog(vertex_shader_id, 512, NULL, info_log);
		printf("Error compiling shader : %s", info_log);
	}

	GLuint shader_program;
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader_id);
	glAttachShader(shader_program, fragment_shader_id);
	glLinkProgram(shader_program);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &s);
	if (!s) {
		glGetShaderInfoLog(shader_program, 512, NULL, info_log);
		printf("Error compiling shader : %s", info_log);
	}


	while(true) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glUseProgram(shader_program);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);

		check_error(CGLFlushDrawable(gl_context));
	}



	glUseProgram(shader_program);

}

int main(int argc, char** argv) {
	printf("\n  A CGL hello triangle.\n* Use cmd+alt+escape to exit! *\n\n");

	init_opengl();
	render_triangle();

}
