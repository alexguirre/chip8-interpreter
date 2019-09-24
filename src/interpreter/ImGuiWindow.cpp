#include "ImGuiWindow.h"
#include <stdexcept>
#include <SDL2/SDL_syswm.h>

// TODO: clean up CImGuiWindow to make coding conventions similar to the rest of the project
// For now, mostly copied from ImGui's SDL2+OpenGL3 example

// Desktop GL has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if defined(IMGUI_IMPL_OPENGL_ES2) || defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     0
#else
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     1
#endif

CImGuiWindow::CImGuiWindow()
{
	mWindow = SDL_CreateWindow(
		"chip8-interpreter: ImGui",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		1280, 720,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);

	if (!mWindow)
	{
		throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	mContext = SDL_GL_CreateContext(mWindow);
	SDL_GL_MakeCurrent(mWindow, mContext);
	SDL_GL_SetSwapInterval(1);

	static bool onceGl3wInit = []()
	{
		if (gl3wInit() != GL3W_OK)
		{
			throw std::runtime_error("Failed to initialize gl3w");
		}
		return true;
	}();

	IMGUI_CHECKVERSION();
	auto restoreImGuiContext = UsingImGuiContext();	
	ImGui::StyleColorsDark();

	InitSdl();
	InitOpenGL();
}

CImGuiWindow::~CImGuiWindow()
{
	DestroyDeviceObjects();

	if (mImGuiContext)
	{
		ImGui::DestroyContext(mImGuiContext);
	}

	if (mContext)
	{
		SDL_GL_DeleteContext(mContext);
	}

	if (mWindow)
	{
		SDL_DestroyWindow(mWindow);
	}
}

void CImGuiWindow::InitSdl()
{
	auto restoreImGuiContext = UsingImGuiContext();
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
	io.BackendPlatformName = "SDL2";

	io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
	io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
	io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
	io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
	io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
	io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
	io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
	io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
	io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
	io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
	io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
	io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
	io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
	io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
	io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
	io.KeyMap[ImGuiKey_KeyPadEnter] = SDL_SCANCODE_RETURN2;
	io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
	io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
	io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
	io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
	io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
	io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;

	io.SetClipboardTextFn = SetClipboardText;
	io.GetClipboardTextFn = GetClipboardText;
	io.ClipboardUserData = NULL;

	static bool onceCreateCursors = []()
	{
		Cursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
		Cursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
		Cursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		Cursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
		Cursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
		Cursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
		Cursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
		Cursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		return true;
	}();

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(mWindow, &wmInfo);
	io.ImeWindowHandle = wmInfo.info.win.window;
}

void CImGuiWindow::InitOpenGL()
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();
	io.BackendRendererName = "OpenGL3";
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
	io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
#endif
}

void CImGuiWindow::UpdateMousePosAndButtons()
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();
	
	if (io.WantSetMousePos)
	{
		SDL_WarpMouseInWindow(mWindow, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
	}
	else
	{
		io.MousePos = ImVec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	}

	std::int32_t mouseX, mouseY;
	std::uint32_t mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
	io.MouseDown[0] = mMousePressed[0] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
	io.MouseDown[1] = mMousePressed[1] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
	io.MouseDown[2] = mMousePressed[2] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
	mMousePressed[0] = mMousePressed[1] = mMousePressed[2] = false;

#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS)
	SDL_Window* focused_window = SDL_GetKeyboardFocus();
	if (g_Window == focused_window)
	{
		// SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
		// The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that, so we retrieve that position globally.
		int wx, wy;
		SDL_GetWindowPosition(focused_window, &wx, &wy);
		SDL_GetGlobalMouseState(&mx, &my);
		mx -= wx;
		my -= wy;
		io.MousePos = ImVec2((float)mx, (float)my);
	}

	// SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
	// The function is only supported from SDL 2.0.4 (released Jan 2016)
	bool any_mouse_button_down = ImGui::IsAnyMouseDown();
	SDL_CaptureMouse(any_mouse_button_down ? SDL_TRUE : SDL_FALSE);
#else
	if (SDL_GetWindowFlags(mWindow) & SDL_WINDOW_INPUT_FOCUS)
		io.MousePos = ImVec2((float)mouseX, (float)mouseY);
#endif
}

void CImGuiWindow::UpdateMouseCursor()
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		return;

	// TODO: cursors in a second ImGuiWindow flickers
	ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
	if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
	{
		SDL_ShowCursor(SDL_DISABLE);
	}
	else
	{
		SDL_SetCursor(Cursors[cursor] ? Cursors[cursor] : Cursors[ImGuiMouseCursor_Arrow]);
		SDL_ShowCursor(SDL_ENABLE);
	}
}

void CImGuiWindow::NewFrameSdl()
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();
	IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

	std::int32_t w, h, displayW, displayH;
	SDL_GetWindowSize(mWindow, &w, &h);
	SDL_GL_GetDrawableSize(mWindow, &displayW, &displayH);
	io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));
	if (w > 0 && h > 0)
	{
		io.DisplayFramebufferScale = ImVec2(static_cast<float>(displayW) / w, static_cast<float>(displayH) / h);
	}

	static std::uint64_t frequency = SDL_GetPerformanceFrequency();
	static std::uint64_t prevTime = 0;
	std::uint64_t currentTime = SDL_GetPerformanceCounter();
	io.DeltaTime = prevTime > 0 ? 
		static_cast<float>(static_cast<double>(currentTime - prevTime) / frequency) :
		(1.0f / 60.0f);
	prevTime = currentTime;

	UpdateMousePosAndButtons();
	UpdateMouseCursor();
}


void CImGuiWindow::NewFrameOpenGL()
{
	if (!HasCreatedDeviceObjects())
	{
		CreateDeviceObjects();
	}
}

bool CImGuiWindow::HasCreatedDeviceObjects() const { return mFontTexture != 0; }

void CImGuiWindow::CreateDeviceObjects()
{
	GLint lastTexture, lastArrayBuffer;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
	glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
	GLint lastVertexArray;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArray);
#endif

	constexpr std::int32_t GlslVersion = 130;
	constexpr const GLchar* GlslVersionStr = "#version 130\n";
	constexpr const GLchar* VertexShaderSrc =
		"uniform mat4 ProjMtx;\n"
		"in vec2 Position;\n"
		"in vec2 UV;\n"
		"in vec4 Color;\n"
		"out vec2 Frag_UV;\n"
		"out vec4 Frag_Color;\n"
		"void main()\n"
		"{\n"
		"    Frag_UV = UV;\n"
		"    Frag_Color = Color;\n"
		"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
		"}\n";

	constexpr const GLchar* FragmentShaderSrc =
		"uniform sampler2D Texture;\n"
		"in vec2 Frag_UV;\n"
		"in vec4 Frag_Color;\n"
		"out vec4 Out_Color;\n"
		"void main()\n"
		"{\n"
		"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
		"}\n";

	const GLchar* const vertexShaderWithVersion[2]{ GlslVersionStr, VertexShaderSrc };
	const GLchar* const fragmentShaderWithVersion[2]{ GlslVersionStr, FragmentShaderSrc };

	mVertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(mVertexShaderHandle, 2, vertexShaderWithVersion, nullptr);
	glCompileShader(mVertexShaderHandle);
	{
		GLint status = 0, log_length = 0;
		glGetShaderiv(mVertexShaderHandle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(mVertexShaderHandle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", "Vertex Shader");
		if (log_length > 1)
		{
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(mVertexShaderHandle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
	}
	// TODO: check vertex shader

	mFragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(mFragmentShaderHandle, 2, fragmentShaderWithVersion, nullptr);
	glCompileShader(mFragmentShaderHandle);
	{
		GLint status = 0, log_length = 0;
		glGetShaderiv(mFragmentShaderHandle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(mFragmentShaderHandle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to compile %s!\n", "Fragment Shader");
		if (log_length > 1)
		{
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetShaderInfoLog(mFragmentShaderHandle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
	}
	// TODO: check fragment shader

	mShaderHandle = glCreateProgram();
	glAttachShader(mShaderHandle, mVertexShaderHandle);
	glAttachShader(mShaderHandle, mFragmentShaderHandle);
	glLinkProgram(mShaderHandle);
	{
		GLint status = 0, log_length = 0;
		glGetProgramiv(mShaderHandle, GL_LINK_STATUS, &status);
		glGetProgramiv(mShaderHandle, GL_INFO_LOG_LENGTH, &log_length);
		if ((GLboolean)status == GL_FALSE)
			fprintf(stderr, "ERROR: ImGui_ImplOpenGL3_CreateDeviceObjects: failed to link %s! (with GLSL '%s')\n", "Shader Program", GlslVersionStr);
		if (log_length > 1)
		{
			ImVector<char> buf;
			buf.resize((int)(log_length + 1));
			glGetProgramInfoLog(mShaderHandle, log_length, NULL, (GLchar*)buf.begin());
			fprintf(stderr, "%s\n", buf.begin());
		}
	}
	// TODO: check shader program

	mAttribLocationTex = glGetUniformLocation(mShaderHandle, "Texture");
	mAttribLocationProjMtx = glGetUniformLocation(mShaderHandle, "ProjMtx");
	mAttribLocationVtxPos = glGetAttribLocation(mShaderHandle, "Position");
	mAttribLocationVtxUV = glGetAttribLocation(mShaderHandle, "UV");
	mAttribLocationVtxColor = glGetAttribLocation(mShaderHandle, "Color");

	// Create buffers
	glGenBuffers(1, &mVboHandle);
	glGenBuffers(1, &mElementsHandle);

	CreateFontsTexture();

	glBindTexture(GL_TEXTURE_2D, lastTexture);
	glBindBuffer(GL_ARRAY_BUFFER_BINDING, lastArrayBuffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
	glBindVertexArray(lastVertexArray);
#endif
}

void CImGuiWindow::CreateFontsTexture()
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();

	std::uint8_t* pixels;
	std::int32_t width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	GLint lastTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
	glGenTextures(1, &mFontTexture);
	glBindTexture(GL_TEXTURE_2D, mFontTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	io.Fonts->TexID = reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(mFontTexture));

	glBindTexture(GL_TEXTURE_2D, lastTexture);
}

void CImGuiWindow::DestroyDeviceObjects()
{
	if (mVboHandle) { glDeleteBuffers(1, &mVboHandle); mVboHandle = 0; }
	if (mElementsHandle) { glDeleteBuffers(1, &mElementsHandle); mElementsHandle = 0; }

	if (mShaderHandle && mVertexShaderHandle) { glDetachShader(mShaderHandle, mVertexShaderHandle); }
	if (mVertexShaderHandle) { glDeleteShader(mVertexShaderHandle); mVertexShaderHandle = 0; }

	if (mShaderHandle && mFragmentShaderHandle) { glDetachShader(mShaderHandle, mFragmentShaderHandle); }
	if (mFragmentShaderHandle) { glDeleteShader(mFragmentShaderHandle); mFragmentShaderHandle = 0; }

	if (mShaderHandle) { glDeleteProgram(mShaderHandle); mShaderHandle = 0; }

	DestroyFontsTexture();
}

void CImGuiWindow::DestroyFontsTexture()
{
	if (mFontTexture)
	{
		auto restoreImGuiContext = UsingImGuiContext();
		ImGuiIO& io = ImGui::GetIO();
		glDeleteTextures(1, &mFontTexture);
		io.Fonts->TexID = 0;
		mFontTexture = 0;
	}
}

void CImGuiWindow::SetupRenderState(ImDrawData* drawData, std::int32_t fbWidth, std::int32_t fbHeight, std::uint32_t vertexArrayObject)
{
	auto restoreImGuiContext = UsingImGuiContext();

	// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

	// Setup viewport, orthographic projection matrix
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
	glViewport(0, 0, (GLsizei)fbWidth, (GLsizei)fbHeight);
	float L = drawData->DisplayPos.x;
	float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
	float T = drawData->DisplayPos.y;
	float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
	const float ortho_projection[4][4] =
	{
		{ 2.0f / (R - L),   0.0f,         0.0f,   0.0f },
		{ 0.0f,         2.0f / (T - B),   0.0f,   0.0f },
		{ 0.0f,         0.0f,        -1.0f,   0.0f },
		{ (R + L) / (L - R),  (T + B) / (B - T),  0.0f,   1.0f },
	};
	glUseProgram(mShaderHandle);
	glUniform1i(mAttribLocationTex, 0);
	glUniformMatrix4fv(mAttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
	glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

	(void)vertexArrayObject;
#ifndef IMGUI_IMPL_OPENGL_ES2
	glBindVertexArray(vertexArrayObject);
#endif

	// Bind vertex/index buffers and setup attributes for ImDrawVert
	glBindBuffer(GL_ARRAY_BUFFER, mVboHandle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementsHandle);
	glEnableVertexAttribArray(mAttribLocationVtxPos);
	glEnableVertexAttribArray(mAttribLocationVtxUV);
	glEnableVertexAttribArray(mAttribLocationVtxColor);
	glVertexAttribPointer(mAttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
	glVertexAttribPointer(mAttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
	glVertexAttribPointer(mAttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)IM_OFFSETOF(ImDrawVert, col));
}

void CImGuiWindow::RenderDrawData(ImDrawData* drawData)
{
	auto restoreImGuiContext = UsingImGuiContext();

	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
	int fb_width = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
	int fb_height = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);
	if (fb_width <= 0 || fb_height <= 0)
		return;

	// Backup GL state
	GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)& last_active_texture);
	glActiveTexture(GL_TEXTURE0);
	GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
	GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
#ifdef GL_SAMPLER_BINDING
	GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
#endif
	GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
	GLint last_vertex_array_object; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array_object);
#endif
#ifdef GL_POLYGON_MODE
	GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
#endif
	GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
	GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
	GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)& last_blend_src_rgb);
	GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)& last_blend_dst_rgb);
	GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)& last_blend_src_alpha);
	GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)& last_blend_dst_alpha);
	GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)& last_blend_equation_rgb);
	GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)& last_blend_equation_alpha);
	GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
	GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
	GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
	bool clip_origin_lower_left = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
	GLenum last_clip_origin = 0; glGetIntegerv(GL_CLIP_ORIGIN, (GLint*)& last_clip_origin); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
	if (last_clip_origin == GL_UPPER_LEFT)
		clip_origin_lower_left = false;
#endif

	// Setup desired GL state
	// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
	// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
	GLuint vertex_array_object = 0;
#ifndef IMGUI_IMPL_OPENGL_ES2
	glGenVertexArrays(1, &vertex_array_object);
#endif
	SetupRenderState(drawData, fb_width, fb_height, vertex_array_object);

	// Will project scissor/clipping rectangles into framebuffer space
	ImVec2 clip_off = drawData->DisplayPos;         // (0,0) unless using multi-viewports
	ImVec2 clip_scale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

	// Render command lists
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];

		// Upload vertex/index buffers
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback != NULL)
			{
				// User callback, registered via ImDrawList::AddCallback()
				// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
				if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
					SetupRenderState(drawData, fb_width, fb_height, vertex_array_object);
				else
					pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clip_rect;
				clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
				clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
				clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
				clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;

				if (clip_rect.x < fb_width && clip_rect.y < fb_height && clip_rect.z >= 0.0f && clip_rect.w >= 0.0f)
				{
					// Apply scissor/clipping rectangle
					if (clip_origin_lower_left)
						glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
					else
						glScissor((int)clip_rect.x, (int)clip_rect.y, (int)clip_rect.z, (int)clip_rect.w); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)

					// Bind texture, Draw
					glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
					glDrawElementsBaseVertex(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)), (GLint)pcmd->VtxOffset);
#else
					glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(pcmd->IdxOffset * sizeof(ImDrawIdx)));
#endif
				}
			}
		}
	}

	// Destroy the temporary VAO
#ifndef IMGUI_IMPL_OPENGL_ES2
	glDeleteVertexArrays(1, &vertex_array_object);
#endif

	// Restore modified GL state
	glUseProgram(last_program);
	glBindTexture(GL_TEXTURE_2D, last_texture);
#ifdef GL_SAMPLER_BINDING
	glBindSampler(0, last_sampler);
#endif
	glActiveTexture(last_active_texture);
#ifndef IMGUI_IMPL_OPENGL_ES2
	glBindVertexArray(last_vertex_array_object);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
	glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
	glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
	if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
	if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
	if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
	if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
	glPolygonMode(GL_FRONT_AND_BACK, (GLenum)last_polygon_mode[0]);
#endif
	glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
	glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

bool CImGuiWindow::ProcessEvent(const SDL_Event& event)
{
	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_MOUSEWHEEL:
	{
		if (event.wheel.x > 0) { io.MouseWheelH += 1; }
		if (event.wheel.x < 0) { io.MouseWheelH -= 1; }
		if (event.wheel.y > 0) { io.MouseWheel += 1; }
		if (event.wheel.y < 0) { io.MouseWheel -= 1; }
		return true;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		if (event.button.button == SDL_BUTTON_LEFT) mMousePressed[0] = true;
		if (event.button.button == SDL_BUTTON_RIGHT) mMousePressed[1] = true;
		if (event.button.button == SDL_BUTTON_MIDDLE) mMousePressed[2] = true;
		return true;
	}
	case SDL_TEXTINPUT:
	{
		io.AddInputCharactersUTF8(event.text.text);
		return true;
	}
	case SDL_KEYDOWN:
	case SDL_KEYUP:
	{
		std::int32_t key = event.key.keysym.scancode;
		IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
		io.KeysDown[key] = (event.type == SDL_KEYDOWN);
		io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
		io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
		io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
		io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
		return true;
	}
	}

	return false;
}

void CImGuiWindow::Render()
{
	SDL_GL_MakeCurrent(mWindow, mContext);

	auto restoreImGuiContext = UsingImGuiContext();

	ImGuiIO& io = ImGui::GetIO();

	// start frame
	NewFrameOpenGL();
	NewFrameSdl();
	ImGui::NewFrame();

	{
		// draw ImGui stuff
		ImGui::ShowDemoWindow();
	}

	// render
	ImGui::Render();
	glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT);
	RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(mWindow);
}

std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> CImGuiWindow::Cursors{};

const char* CImGuiWindow::GetClipboardText(void*)
{
	static std::unique_ptr<char, decltype(&SDL_free)> clipboardData{ nullptr, SDL_free };
	
	clipboardData.reset(SDL_GetClipboardText());
	return clipboardData.get();

}

void CImGuiWindow::SetClipboardText(void*, const char* text)
{
	SDL_SetClipboardText(text);
}
