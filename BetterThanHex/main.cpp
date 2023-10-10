#pragma once
#include "Dependencies/glew.h"
#include "Dependencies/glfw3.h"
#include <iostream>
#include "UI/ui.h"
#include "manager/manager.h"

#define WINDOW_WIDTH 950
#define WINDOW_HEIGHT 800
#define ASSERT(x) if(!(x)) __debugbreak();

bool GLLogCall(const char* func, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[ERROR]: " << error << " "\
			<< file << " ~ " << func << " Line: " << line << std::endl;
		return false;
	}
	return true;

}

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR) {};
}

#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__));



int selectedHexValue = -1;
std::vector<int> selectedIndices; // Store selected indices
static char hexValueBuffer[4] = "";
bool showAscii = false;
bool m_bHexDumpShowOffsetPopup = false;
int offset = 0;
char offsetValueBuffer[8];







int RenderRight(Manager* manager)
{
	manager->RenderUI();
	return 1;
};





int main()
{

	GLFWwindow* window = nullptr;

	if (!glfwInit())
	{
		return -1;
	}
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Window !", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}


	UI ui(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, 0, 0, 0);
	ui.Init(window);

	// Modify the button colors
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.6f, 0.6f); // Button background color (white)
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Button background color when hovered
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Button background color when active
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set text color to black for buttons





	Manager* manager = new Manager();

	ui.AddComponentLeft("RenderLeft", RenderRight, manager);



	while (!glfwWindowShouldClose(window))
	{
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		ui.Start();
		ui.DrawUI();
		ui.End();



		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}




	glfwTerminate();
	return 0;
}