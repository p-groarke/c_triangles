
const static char* vertexShaderSource =\
	"#version 330 core\n"
	"layout (location = 0) in vec3 position;"
	"void main()"
	"{"
	"gl_Position = vec4(position.x, position.y, position.z, 1.0);"
	"}\0";
const static char* fragmentShaderSource =\
	"#version 330 core\n"
	"out vec4 color;"
	"void main() {"
	"color = vec4(1.0, 0.5, 0.2, 1.0);"
	"}\0";


