#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <math.h>
#include <GL/gl.h>
#include "glext.h" //https://www.opengl.org/registry/
#include "opengl_shader.h"

#include <stdio.h>

#define XRES 1440
#define YRES 900

typedef struct {
	HINSTANCE hInstance;
	HDC hDC;
	HGLRC hRC;
	HWND hWnd;
	int full;
	char wndclass[4];
} WININFO;

static WININFO win_info = {0, 0, 0, 0, 0,
	{'s', 'c', 'x', 0}};

static PIXELFORMATDESCRIPTOR pfd = {
	sizeof(PIXELFORMATDESCRIPTOR)
	,1
	,PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER
	,PFD_TYPE_RGBA
	,32
	,0, 0, 0, 0, 0, 0, 8, 0
	,0, 0, 0, 0, 0	// accum
	,32				// zbuffer
	,0				// stencil!
	,0				// aux
	,PFD_MAIN_PLANE
	,0, 0, 0, 0
};

static float fparams[4*4];
static int fsid;
static int vsid;

#define NUMFUNCS 14
const static char* gl_func_names[] = {
	"glCreateShaderProgramv"
	,"glGenProgramPipelines"
	,"glBindProgramPipeline"
	,"glUseProgramStages"
	,"glProgramUniform4fv"
	,"glGetProgramiv"
	,"glGetProgramInfoLog"
	,"glGenVertexArrays"
	,"glBindVertexArray"
	,"glGenBuffers"
	,"glBindBuffer"
	,"glBufferData"
	,"glVertexAttribPointer"
	,"glEnableVertexAttribArray"
};

static void* gl_funcs[NUMFUNCS];
#define oglCreateShaderProgramv		((PFNGLCREATESHADERPROGRAMVPROC)gl_funcs[0])
#define oglGenProgramPipelines		((PFNGLGENPROGRAMPIPELINESPROC)gl_funcs[1])
#define oglBindProgramPipeline		((PFNGLBINDPROGRAMPIPELINEPROC)gl_funcs[2])
#define oglUseProgramStages			((PFNGLUSEPROGRAMSTAGESPROC)gl_funcs[3])
#define oglProgramUniform4fv		((PFNGLPROGRAMUNIFORM4FVPROC)gl_funcs[4])
#define oglGetProgramiv				((PFNGLGETPROGRAMIVPROC)gl_funcs[5])
#define oglGetProgramInfoLog		((PFNGLGETPROGRAMINFOLOGPROC)gl_funcs[6])
#define oglGenVertexArrays			((PFNGLGENVERTEXARRAYSPROC)gl_funcs[7])
#define oglBindVertexArray			((PFNGLBINDVERTEXARRAYPROC)gl_funcs[8])
#define oglGenBuffers				((PFNGLGENBUFFERSPROC)gl_funcs[9])
#define oglBindBuffer				((PFNGLBINDBUFFERPROC)gl_funcs[10])
#define oglBufferData				((PFNGLBUFFERDATAPROC)gl_funcs[11])
#define oglVertexAttribPointer		((PFNGLVERTEXATTRIBPOINTERPROC)gl_funcs[12])
#define oglEnableVertexAttribArray	((PFNGLENABLEVERTEXATTRIBARRAYPROC)gl_funcs[13])

static int init_gl_funcs()
{
	for (int i = 0; i < NUMFUNCS; ++i) {
		gl_funcs[i] = wglGetProcAddress(gl_func_names[i]);
		if (!gl_funcs[i])
			return 0;
	}
	return 1;
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_SYSCOMMAND
			&& (wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER))
		return 0;

	if (uMsg == WM_CLOSE || uMsg == WM_DESTROY
			|| (uMsg == WM_KEYDOWN && wParam == VK_ESCAPE)
			|| (uMsg == WM_CHAR && wParam == VK_ESCAPE))

	{
		PostQuitMessage(0);
		return 0;
	}

	if (uMsg == WM_SIZE)
		glViewport(0, 0, lParam & 65535, lParam >> 16);

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static int init_window()
{
	unsigned int PixelFormat;
	DWORD dwExStyle, dwStyle;
	DEVMODE dmScreenSettings;
	RECT rect;
	WNDCLASS wc;

	ZeroMemory(&wc, sizeof(WNDCLASS));
	wc.style			= CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= win_info.hInstance;
	wc.lpszClassName	= win_info.wndclass;
	wc.hbrBackground	= (HBRUSH)CreateSolidBrush(0x00102030);

	if (!RegisterClass(&wc))
		return 0;

	if (win_info.full) {
		dmScreenSettings.dmSize			= sizeof(DEVMODE);
		dmScreenSettings.dmFields		= DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;
		dmScreenSettings.dmBitsPerPel	= 32;
		dmScreenSettings.dmPelsWidth	= XRES;
		dmScreenSettings.dmPelsHeight	= YRES;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN)
				!= DISP_CHANGE_SUCCESSFUL)
			return 0;

		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_VISIBLE | WS_POPUP;

		while(ShowCursor(0) >= 0);
	} else {
		dwExStyle = 0;
		dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX
				| WS_OVERLAPPED;
		dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;
	}

	rect.left = 0;
	rect.top = 0;
	rect.right = XRES;
	rect.bottom = YRES;

	AdjustWindowRect(&rect, dwStyle, 0);

	win_info.hWnd = CreateWindowEx(dwExStyle, wc.lpszClassName, "weeeee",
			dwStyle,
			(GetSystemMetrics(SM_CXSCREEN) - rect.right + rect.left) >> 1,
			(GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) >> 1,
			rect.right - rect.left, rect.bottom - rect.top, 0, 0,
			win_info.hInstance, 0);

	if (!win_info.hWnd)
		return 0;

	if (!(win_info.hDC = GetDC(win_info.hWnd)))
		return 0;

	if (!(PixelFormat = ChoosePixelFormat(win_info.hDC, &pfd)))
		return 0;

	if (!SetPixelFormat(win_info.hDC, PixelFormat, &pfd))
		return 0;

	if (!(win_info.hRC = wglCreateContext(win_info.hDC)))
		return 0;

	if (!wglMakeCurrent(win_info.hDC, win_info.hRC))
		return 0;

	return 1;
}

static void clean_window()
{
	if (win_info.hRC) {
		wglMakeCurrent(0, 0);
		wglDeleteContext(win_info.hRC);
	}

	if (win_info.hDC)
		ReleaseDC(win_info.hWnd, win_info.hDC);

	if (win_info.hWnd)
		DestroyWindow(win_info.hWnd);

	UnregisterClass(win_info.wndclass, win_info.hInstance);

	if (win_info.full) {
		ChangeDisplaySettings(0, 0);
		while (ShowCursor(1) < 0);
	}
}

static int init_opengl()
{
	if (!init_gl_funcs())
		return 0;

	vsid = oglCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vertexShaderSource);
	fsid = oglCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fragmentShaderSource);

	unsigned int pid;
	oglGenProgramPipelines(1, &pid);
	oglBindProgramPipeline(pid);
	oglUseProgramStages(pid, GL_VERTEX_SHADER_BIT, vsid);
	oglUseProgramStages(pid, GL_FRAGMENT_SHADER_BIT, fsid);

	//#ifdef DEBUG
		int result;
		char info[1536];
		oglGetProgramiv(vsid, GL_LINK_STATUS, &result); oglGetProgramInfoLog(vsid, 1024, NULL, (char *)info); if(!result) DebugBreak();
		oglGetProgramiv(fsid, GL_LINK_STATUS, &result); oglGetProgramInfoLog(fsid, 1024, NULL, (char *)info); if(!result) DebugBreak();
		oglGetProgramiv(pid,  GL_LINK_STATUS, &result); oglGetProgramInfoLog(pid,  1024, NULL, (char *)info); if(!result) DebugBreak();
	//#endif

	return 1;
}

static int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	int done = 0;
	win_info.hInstance = GetModuleHandle(0);

	if (!init_window()) {
		clean_window();
		MessageBox(0, "init_window()!", "error", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}

	if (!init_opengl()) {
		clean_window();
		MessageBox(0, "init_opengl()!", "error", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}

	/* Draw */
	static const GLfloat vertices[] = {
		-0.5, -0.5, 0.0,
		0.5, -0.5, 0.0,
		0.0, 0.5, 0.0
	};

	GLuint VAO;
	oglGenVertexArrays(1, &VAO);
	oglBindVertexArray(VAO);

	GLuint VBO;
	oglGenBuffers(1, &VBO);
	oglBindBuffer(GL_ARRAY_BUFFER, VBO);
	oglBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	/* FIXME: UNDERSTAND THIS SHIT */
	oglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
			(GLvoid*)0);
	oglEnableVertexAttribArray(0);

	oglBindVertexArray(0); // Unbind because we could misconfigure.


	while (!done) {
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
				done = 1;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		oglBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		oglBindVertexArray(0);

		SwapBuffers(win_info.hDC);
		Sleep(50);
	}

	clean_window();
	return 0;
}
