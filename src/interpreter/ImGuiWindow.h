#pragma once
#include <SDL2/SDL.h>
#include <imgui.h>
#include <array>
#include <gsl/gsl_util>
#include <functional>
#include <GL/gl3w.h>

class CImGuiWindow
{
private:
	SDL_Window* mWindow;
	SDL_GLContext mContext;
	ImGuiContext* mImGuiContext;
	std::array<bool, 3> mMousePressed;
	GLuint mVertexShaderHandle;
	GLuint mFragmentShaderHandle;
	GLuint mShaderHandle;
	GLint mAttribLocationTex;
	GLint mAttribLocationProjMtx;
	GLint mAttribLocationVtxPos;
	GLint mAttribLocationVtxUV;
	GLint mAttribLocationVtxColor;
	GLuint mVboHandle;
	GLuint mElementsHandle;
	GLuint mFontTexture;
public:
	CImGuiWindow();
	~CImGuiWindow();

	bool ProcessEvent(const SDL_Event& event);
	void Render();

private:
	void InitSdl();
	void InitOpenGL();
	void UpdateMousePosAndButtons();
	void UpdateMouseCursor();
	void NewFrameSdl();
	void NewFrameOpenGL();

	void SetupRenderState(ImDrawData* drawData, int fbWidth, int fbHeight, GLuint vertexArrayObject);
	void RenderDrawData(ImDrawData* drawData);

	bool HasCreatedDeviceObjects() const;
	void CreateDeviceObjects();
	void CreateFontsTexture();
	void DestroyDeviceObjects();
	void DestroyFontsTexture();

	auto UsingImGuiContext()
	{
		ImGuiContext* prevContext = ImGui::GetCurrentContext();
		if (!mImGuiContext)
		{
			mImGuiContext = ImGui::CreateContext();
		}
		ImGui::SetCurrentContext(mImGuiContext);
		return gsl::finally(std::bind(ImGui::SetCurrentContext, prevContext));
	}

	static std::array<SDL_Cursor*, ImGuiMouseCursor_COUNT> Cursors;

	static const char* GetClipboardText(void*);
	static void SetClipboardText(void*, const char* text);
};

