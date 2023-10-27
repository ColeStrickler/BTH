#include "ui.h"

UI::UI(int WindowWidth, int WindowHeight, int LeftWidth, int RightWidth, int BottomHeight, int TopHeight) :
	m_WindowWidth(WindowWidth), m_WindowHeight(WindowHeight), m_LeftWidth(LeftWidth), m_RightWidth(RightWidth),
	m_BottomHeight(BottomHeight), m_TopHeight(TopHeight)
{
	
}

UI::~UI()
{
	Cleanup();
}

void UI::Init(GLFWwindow* Window)
{
	m_Window = Window;
	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
#endif
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui::StyleColorsDark();


}

void UI::DrawUI() const
{

	// Create top window
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(m_WindowWidth, m_TopHeight));
	ImGui::Begin("Top Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	DisplayComponentsTop();
	ImGui::End();


	// Create bottom window
	ImGui::SetNextWindowPos(ImVec2(0, m_WindowHeight - m_BottomHeight));
	ImGui::SetNextWindowSize(ImVec2(m_WindowWidth, m_BottomHeight));
	ImGui::Begin("Bottom Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	DisplayComponentsBottom();
	ImGui::End();


	// Create left window
	ImGui::SetNextWindowPos(ImVec2(0, m_TopHeight));
	ImGui::SetNextWindowSize(ImVec2(m_LeftWidth, m_WindowHeight - m_TopHeight - m_BottomHeight));
	ImGui::Begin("Left Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	DisplayComponentsLeft();
	ImGui::End();

	// Create right window
	ImGui::SetNextWindowPos(ImVec2(m_WindowWidth - m_RightWidth, m_TopHeight));
	ImGui::SetNextWindowSize(ImVec2(m_RightWidth, m_WindowHeight - m_TopHeight - m_BottomHeight));
	ImGui::Begin("Right Window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	DisplayComponentsRight();
	ImGui::End();
}

void UI::Start() const
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void UI::End()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UI::Cleanup()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}



void UI::DisplayComponentsRight() const
{
	for (auto& CF : m_ComponentsRight)
	{
		CF.m_Func();
	}
}

void UI::DisplayComponentsLeft() const
{
	for (auto& CF : m_ComponentsLeft)
	{
		CF.m_Func();
	}
}

void UI::DisplayComponentsTop() const
{
	for (auto& CF : m_ComponentsTop)
	{
		CF.m_Func();
	}
}

void UI::DisplayComponentsBottom() const
{
	for (auto& CF : m_ComponentsBottom)
	{
		CF.m_Func();
	}
}



