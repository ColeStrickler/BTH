#pragma once
#include <string>
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"


namespace stylewrappers
{
	bool Button(const std::string& ButtonText, const ImVec4& Color, ImVec2 ButtonSize = ImVec2(20, 20));


	void HexDumpBackgroundStyle(const ImVec4& Color);




};

