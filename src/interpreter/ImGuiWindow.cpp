#include "ImGuiWindow.h"
#include <stdexcept>
#include <mutex>
#include <functional>
#include <array>
#include <SDL2/SDL_syswm.h>
#include <gsl/gsl_util>
#include <GL/gl3w.h>
#include <imgui.h>

// TODO: clean up CImGuiWindow to make coding conventions similar to the rest of the project
// For now, mostly copied from ImGui's SDL2+OpenGL3 example

// Desktop GL has glDrawElementsBaseVertex() which GL ES and WebGL don't have.
#if defined(IMGUI_IMPL_OPENGL_ES2) || defined(IMGUI_IMPL_OPENGL_ES3)
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     0
#else
#define IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX     1
#endif

class CImGuiWindow::Impl
{
private:
	SDL_Window* Window = nullptr;
	SDL_GLContext GlContext = 0;
	ImGuiContext* ImContext = nullptr;
	std::array<bool, 3> MousePressed;
	GLuint VertexShaderHandle = 0;
	GLuint FragmentShaderHandle = 0;
	GLuint ShaderHandle = 0;
	GLint AttribLocationTex = 0;
	GLint AttribLocationProjMtx = 0;
	GLint AttribLocationVtxPos = 0;
	GLint AttribLocationVtxUV = 0;
	GLint AttribLocationVtxColor = 0;
	GLuint VboHandle = 0;
	GLuint ElementsHandle = 0;
	GLuint FontTexture = 0;

public:
	Impl()
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		Window = SDL_CreateWindow(
			"chip8-interpreter: ImGui",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			1280, 720,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
		);

		if (!Window)
		{
			throw std::runtime_error("Failed to create window: " + std::string(SDL_GetError()));
		}

		GlContext = SDL_GL_CreateContext(Window);
		SDL_GL_MakeCurrent(Window, GlContext);
		SDL_GL_SetSwapInterval(1);

		static std::once_flag onceGl3wInit;
		std::call_once(onceGl3wInit, []()
			{
				if (gl3wInit() != GL3W_OK)
				{
					throw std::runtime_error("Failed to initialize gl3w");
				}
			});

		IMGUI_CHECKVERSION();
		ImContext = ImGui::CreateContext();
		ImGui::StyleColorsDark();

		InitSdl();
		InitOpenGL();
	}

	~Impl()
	{
		DestroyDeviceObjects();

		if (ImContext)
		{
			ImGui::DestroyContext(ImContext);
		}

		if (GlContext)
		{
			SDL_GL_DeleteContext(GlContext);
		}

		if (Window)
		{
			SDL_DestroyWindow(Window);
		}
	}

	bool ProcessEvent(const SDL_Event& event)
	{
		ImGui::SetCurrentContext(ImContext);

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
			if (event.button.button == SDL_BUTTON_LEFT) MousePressed[0] = true;
			if (event.button.button == SDL_BUTTON_RIGHT) MousePressed[1] = true;
			if (event.button.button == SDL_BUTTON_MIDDLE) MousePressed[2] = true;
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

	void BeginRender()
	{
		SDL_GL_MakeCurrent(Window, GlContext);
		ImGui::SetCurrentContext(ImContext);

		// start frame
		NewFrameOpenGL();
		NewFrameSdl();
		ImGui::NewFrame();
	}

	void EndRender()
	{
		SDL_GL_MakeCurrent(Window, GlContext);
		ImGui::SetCurrentContext(ImContext);

		ImGuiIO& io = ImGui::GetIO();

		// render
		ImGui::Render();
		glViewport(0, 0, static_cast<GLsizei>(io.DisplaySize.x), static_cast<GLsizei>(io.DisplaySize.y));
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(Window);
	}

private:
	void InitSdl()
	{
		ImGui::SetCurrentContext(ImContext);

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

		static std::once_flag onceCreateCursors;
		std::call_once(onceCreateCursors, []()
			{
				Cursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
				Cursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
				Cursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
				Cursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
				Cursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
				Cursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
				Cursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
				Cursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
			});

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(Window, &wmInfo);
		io.ImeWindowHandle = wmInfo.info.win.window;
	}

	void InitOpenGL()
	{
		ImGui::SetCurrentContext(ImContext);

		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = "OpenGL3";
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
#endif
	}

	void UpdateMousePosAndButtons()
	{
		ImGui::SetCurrentContext(ImContext);

		ImGuiIO& io = ImGui::GetIO();

		if (io.WantSetMousePos)
		{
			SDL_WarpMouseInWindow(Window, static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
		}
		else
		{
			io.MousePos = ImVec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
		}

		std::int32_t mouseX, mouseY;
		std::uint32_t mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);
		io.MouseDown[0] = MousePressed[0] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
		io.MouseDown[1] = MousePressed[1] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
		io.MouseDown[2] = MousePressed[2] || (mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
		std::fill(MousePressed.begin(), MousePressed.end(), false);

#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE && !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS)
		SDL_Window* focused_window = SDL_GetKeyboardFocus();
		if (g_Window == focused_window)
		{
			// SDL_GetMouseState() gives mouse position seemingly based on the last window entered/focused(?)
			// The creation of a new windows at runtime and SDL_CaptureMouse both seems to severely mess up with that, so we retrieve that position globally.
			std::int32_t winX, winY;
			SDL_GetWindowPosition(focused_window, &winX, &winY);
			SDL_GetGlobalMouseState(&mouseX, &mouseY);
			mouseX -= winX;
			mouseY -= winY;
			io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
		}

		// SDL_CaptureMouse() let the OS know e.g. that our imgui drag outside the SDL window boundaries shouldn't e.g. trigger the OS window resize cursor.
		// The function is only supported from SDL 2.0.4 (released Jan 2016)
		bool anyMouseButtonDown = ImGui::IsAnyMouseDown();
		SDL_CaptureMouse(anyMouseButtonDown ? SDL_TRUE : SDL_FALSE);
#else
		if (SDL_GetWindowFlags(Window) & SDL_WINDOW_INPUT_FOCUS)
			io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
#endif
	}

	void UpdateMouseCursor()
	{
		ImGui::SetCurrentContext(ImContext);

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

	void NewFrameSdl()
	{
		ImGui::SetCurrentContext(ImContext);

		ImGuiIO& io = ImGui::GetIO();
		IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer back-end. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

		std::int32_t w, h, displayW, displayH;
		SDL_GetWindowSize(Window, &w, &h);
		SDL_GL_GetDrawableSize(Window, &displayW, &displayH);
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

	void NewFrameOpenGL()
	{
		if (!HasCreatedDeviceObjects())
		{
			CreateDeviceObjects();
		}
	}

	bool HasCreatedDeviceObjects() const { return FontTexture != 0; }

	void CreateDeviceObjects()
	{
		ImGui::SetCurrentContext(ImContext);

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

		VertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(VertexShaderHandle, 2, vertexShaderWithVersion, nullptr);
		glCompileShader(VertexShaderHandle);

		FragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(FragmentShaderHandle, 2, fragmentShaderWithVersion, nullptr);
		glCompileShader(FragmentShaderHandle);

		ShaderHandle = glCreateProgram();
		glAttachShader(ShaderHandle, VertexShaderHandle);
		glAttachShader(ShaderHandle, FragmentShaderHandle);
		glLinkProgram(ShaderHandle);

		AttribLocationTex = glGetUniformLocation(ShaderHandle, "Texture");
		AttribLocationProjMtx = glGetUniformLocation(ShaderHandle, "ProjMtx");
		AttribLocationVtxPos = glGetAttribLocation(ShaderHandle, "Position");
		AttribLocationVtxUV = glGetAttribLocation(ShaderHandle, "UV");
		AttribLocationVtxColor = glGetAttribLocation(ShaderHandle, "Color");

		// Create buffers
		glGenBuffers(1, &VboHandle);
		glGenBuffers(1, &ElementsHandle);

		CreateFontsTexture();

		glBindTexture(GL_TEXTURE_2D, lastTexture);
		glBindBuffer(GL_ARRAY_BUFFER_BINDING, lastArrayBuffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
		glBindVertexArray(lastVertexArray);
#endif
	}

	void CreateFontsTexture()
	{
		ImGui::SetCurrentContext(ImContext);

		ImGuiIO& io = ImGui::GetIO();

		std::uint8_t* pixels;
		std::int32_t width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

		GLint lastTexture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
		glGenTextures(1, &FontTexture);
		glBindTexture(GL_TEXTURE_2D, FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#ifdef GL_UNPACK_ROW_LENGTH
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		io.Fonts->TexID = reinterpret_cast<ImTextureID>(static_cast<std::uintptr_t>(FontTexture));

		glBindTexture(GL_TEXTURE_2D, lastTexture);
	}

	void DestroyDeviceObjects()
	{
		if (VboHandle) { glDeleteBuffers(1, &VboHandle); VboHandle = 0; }
		if (ElementsHandle) { glDeleteBuffers(1, &ElementsHandle); ElementsHandle = 0; }

		if (ShaderHandle && VertexShaderHandle) { glDetachShader(ShaderHandle, VertexShaderHandle); }
		if (VertexShaderHandle) { glDeleteShader(VertexShaderHandle); VertexShaderHandle = 0; }

		if (ShaderHandle && FragmentShaderHandle) { glDetachShader(ShaderHandle, FragmentShaderHandle); }
		if (FragmentShaderHandle) { glDeleteShader(FragmentShaderHandle); FragmentShaderHandle = 0; }

		if (ShaderHandle) { glDeleteProgram(ShaderHandle); ShaderHandle = 0; }

		DestroyFontsTexture();
	}

	void DestroyFontsTexture()
	{
		if (FontTexture)
		{
			ImGui::SetCurrentContext(ImContext);

			ImGuiIO& io = ImGui::GetIO();
			glDeleteTextures(1, &FontTexture);
			io.Fonts->TexID = 0;
			FontTexture = 0;
		}
	}

	void SetupRenderState(ImDrawData* drawData, std::int32_t fbWidth, std::int32_t fbHeight, std::uint32_t vertexArrayObject)
	{
		ImGui::SetCurrentContext(ImContext);

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
		const float orthoProjection[4][4] =
		{
			{ 2.0f / (R - L),		0.0f,				0.0f,	0.0f },
			{ 0.0f,					2.0f / (T - B),		0.0f,	0.0f },
			{ 0.0f,					0.0f,				-1.0f,	0.0f },
			{ (R + L) / (L - R),	(T + B) / (B - T),	0.0f,	1.0f },
		};
		glUseProgram(ShaderHandle);
		glUniform1i(AttribLocationTex, 0);
		glUniformMatrix4fv(AttribLocationProjMtx, 1, GL_FALSE, &orthoProjection[0][0]);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, 0); // We use combined texture/sampler state. Applications using GL 3.3 may set that otherwise.
#endif

		(void)vertexArrayObject;
#ifndef IMGUI_IMPL_OPENGL_ES2
		glBindVertexArray(vertexArrayObject);
#endif

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		glBindBuffer(GL_ARRAY_BUFFER, VboHandle);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementsHandle);
		glEnableVertexAttribArray(AttribLocationVtxPos);
		glEnableVertexAttribArray(AttribLocationVtxUV);
		glEnableVertexAttribArray(AttribLocationVtxColor);
		glVertexAttribPointer(AttribLocationVtxPos, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, pos)));
		glVertexAttribPointer(AttribLocationVtxUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, uv)));
		glVertexAttribPointer(AttribLocationVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), reinterpret_cast<GLvoid*>(IM_OFFSETOF(ImDrawVert, col)));
	}

	void RenderDrawData(ImDrawData* drawData)
	{
		ImGui::SetCurrentContext(ImContext);

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		std::int32_t fbWidth = static_cast<std::int32_t>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		std::int32_t fbHeight = static_cast<std::int32_t>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fbWidth <= 0 || fbHeight <= 0)
			return;

		// Backup GL state
		GLenum lastActiveTexture; glGetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&lastActiveTexture));
		glActiveTexture(GL_TEXTURE0);
		GLint lastProgram; glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgram);
		GLint lastTexture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTexture);
#ifdef GL_SAMPLER_BINDING
		GLint lastSampler; glGetIntegerv(GL_SAMPLER_BINDING, &lastSampler);
#endif
		GLint lastArrayBuffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &lastArrayBuffer);
#ifndef IMGUI_IMPL_OPENGL_ES2
		GLint lastVertexArrayObject; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayObject);
#endif
#ifdef GL_POLYGON_MODE
		GLint lastPolygonMode[2]; glGetIntegerv(GL_POLYGON_MODE, lastPolygonMode);
#endif
		GLint lastViewport[4]; glGetIntegerv(GL_VIEWPORT, lastViewport);
		GLint lastScissorBox[4]; glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox);
		GLenum lastBlendSrcRgb; glGetIntegerv(GL_BLEND_SRC_RGB, reinterpret_cast<GLint*>(&lastBlendSrcRgb));
		GLenum lastBlendDstRgb; glGetIntegerv(GL_BLEND_DST_RGB, reinterpret_cast<GLint*>(&lastBlendDstRgb));
		GLenum lastBlendSrcAlpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, reinterpret_cast<GLint*>(&lastBlendSrcAlpha));
		GLenum lastBlendDstAlpha; glGetIntegerv(GL_BLEND_DST_ALPHA, reinterpret_cast<GLint*>(&lastBlendDstAlpha));
		GLenum lastBlendEquationRgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint*>(&lastBlendEquationRgb));
		GLenum lastBlendEquationAlpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, reinterpret_cast<GLint*>(&lastBlendEquationAlpha));
		GLboolean lastEnableBlend = glIsEnabled(GL_BLEND);
		GLboolean lastEnableCullFace = glIsEnabled(GL_CULL_FACE);
		GLboolean lastEnableDepthTest = glIsEnabled(GL_DEPTH_TEST);
		GLboolean lastEnableScissorTest = glIsEnabled(GL_SCISSOR_TEST);
		bool clipOriginLowerLeft = true;
#if defined(GL_CLIP_ORIGIN) && !defined(__APPLE__)
		GLenum lastClipOrigin = 0; glGetIntegerv(GL_CLIP_ORIGIN, reinterpret_cast<GLint*>(&lastClipOrigin)); // Support for GL 4.5's glClipControl(GL_UPPER_LEFT)
		if (lastClipOrigin == GL_UPPER_LEFT)
			clipOriginLowerLeft = false;
#endif

		// Setup desired GL state
		// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
		// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
		GLuint vertexArrayObject = 0;
#ifndef IMGUI_IMPL_OPENGL_ES2
		glGenVertexArrays(1, &vertexArrayObject);
#endif
		SetupRenderState(drawData, fbWidth, fbHeight, vertexArrayObject);

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < drawData->CmdListsCount; n++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[n];

			// Upload vertex/index buffers
			glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(cmdList->VtxBuffer.Size) * sizeof(ImDrawVert), reinterpret_cast<const GLvoid*>(cmdList->VtxBuffer.Data), GL_STREAM_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(cmdList->IdxBuffer.Size) * sizeof(ImDrawIdx), reinterpret_cast<const GLvoid*>(cmdList->IdxBuffer.Data), GL_STREAM_DRAW);

			for (int cmdIdx = 0; cmdIdx < cmdList->CmdBuffer.Size; cmdIdx++)
			{
				const ImDrawCmd* pcmd = &cmdList->CmdBuffer[cmdIdx];
				if (pcmd->UserCallback != NULL)
				{
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
						SetupRenderState(drawData, fbWidth, fbHeight, vertexArrayObject);
					else
						pcmd->UserCallback(cmdList, pcmd);
				}
				else
				{
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
					clipRect.y = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
					clipRect.z = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
					clipRect.w = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;

					if (clipRect.x < fbWidth && clipRect.y < fbHeight && clipRect.z >= 0.0f && clipRect.w >= 0.0f)
					{
						// Apply scissor/clipping rectangle
						if (clipOriginLowerLeft)
							glScissor(static_cast<GLint>(clipRect.x), static_cast<GLint>(fbHeight - clipRect.w),
								static_cast<GLint>(clipRect.z - clipRect.x), static_cast<GLint>(clipRect.w - clipRect.y));
						else
							glScissor(static_cast<GLint>(clipRect.x), static_cast<GLint>(clipRect.y),
								static_cast<GLint>(clipRect.z), static_cast<GLint>(clipRect.w)); // Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)

						// Bind texture, Draw
						glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(pcmd->TextureId)));
#if IMGUI_IMPL_OPENGL_HAS_DRAW_WITH_BASE_VERTEX
						glDrawElementsBaseVertex(GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(pcmd->IdxOffset * sizeof(ImDrawIdx)), static_cast<GLint>(pcmd->VtxOffset));
#else
						glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(pcmd->ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(pcmd->IdxOffset * sizeof(ImDrawIdx)));
#endif
					}
				}
			}
		}

		// Destroy the temporary VAO
#ifndef IMGUI_IMPL_OPENGL_ES2
		glDeleteVertexArrays(1, &vertexArrayObject);
#endif

		// Restore modified GL state
		glUseProgram(lastProgram);
		glBindTexture(GL_TEXTURE_2D, lastTexture);
#ifdef GL_SAMPLER_BINDING
		glBindSampler(0, lastSampler);
#endif
		glActiveTexture(lastActiveTexture);
#ifndef IMGUI_IMPL_OPENGL_ES2
		glBindVertexArray(lastVertexArrayObject);
#endif
		glBindBuffer(GL_ARRAY_BUFFER, lastArrayBuffer);
		glBlendEquationSeparate(lastBlendEquationRgb, lastBlendEquationAlpha);
		glBlendFuncSeparate(lastBlendSrcRgb, lastBlendDstRgb, lastBlendSrcAlpha, lastBlendDstAlpha);
		if (lastEnableBlend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
		if (lastEnableCullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
		if (lastEnableDepthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
		if (lastEnableScissorTest) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
		glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(lastPolygonMode[0]));
#endif
		glViewport(lastViewport[0], lastViewport[1], static_cast<GLsizei>(lastViewport[2]), static_cast<GLsizei>(lastViewport[3]));
		glScissor(lastScissorBox[0], lastScissorBox[1], static_cast<GLsizei>(lastScissorBox[2]), static_cast<GLsizei>(lastScissorBox[3]));
	}

	static const char* GetClipboardText(void*)
	{
		static std::unique_ptr<char, decltype(&SDL_free)> clipboardData{ nullptr, SDL_free };

		clipboardData.reset(SDL_GetClipboardText());
		return clipboardData.get();

	}

	static void SetClipboardText(void*, const char* text)
	{
		SDL_SetClipboardText(text);
	}

	inline static std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> Cursors{};
};

CImGuiWindow::CImGuiWindow()
	: mImpl{ std::make_unique<Impl>() }
{
}

CImGuiWindow::~CImGuiWindow() = default;

bool CImGuiWindow::ProcessEvent(const SDL_Event& event)
{
	return mImpl->ProcessEvent(event);
}

void CImGuiWindow::Render()
{
	mImpl->BeginRender();
	Draw();
	mImpl->EndRender();
}
