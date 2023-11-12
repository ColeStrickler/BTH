#pragma once
#include <string>
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"


namespace stylewrappers
{
	bool Button(const std::string& ButtonText, const ImVec4& Color, ImVec2 ButtonSize = ImVec2(100, 20),
		const ImVec4& TextColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f));


	void HexDumpBackgroundStyle(const ImVec4& Color);

	bool ColoredSelectable(const std::string& SelectableText, const ImVec4& Color);

	void ColoredInputText(const ImVec4& Color, const std::string& Label, char* Buffer, size_t BufferSize, ImGuiInputTextFlags Flags);


	class TableStyle
	{
	public:
		TableStyle(const ImVec4& ColHeaderBgColor, const ImVec4& ColHeaderTextColor);
		~TableStyle();


	private:
		ImVec4 m_OriginalColHeaderColor;
		ImVec4 m_OriginalColHeaderTextColor;
	};


};

