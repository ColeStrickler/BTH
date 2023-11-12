#include "imgui_stylewrappers.h"


static ImU32 Vec4ToU32(const ImVec4& v4)
{
	ImU32 bgColorU32 = IM_COL32(
		static_cast<int>(v4.x * 255),
		static_cast<int>(v4.y * 255),
		static_cast<int>(v4.z * 255),
		static_cast<int>(v4.w * 255)
	);
	return bgColorU32;
}


bool stylewrappers::Button(const std::string& ButtonText, const ImVec4& Color, ImVec2 ButtonSize, const ImVec4& TextColor)
{
	bool ret = false;
	ImGuiStyle& style = ImGui::GetStyle();
	auto original_button_style = style.Colors[ImGuiCol_Button];
	auto original_text_style = style.Colors[ImGuiCol_Text];

	style.Colors[ImGuiCol_Text] = TextColor;
	style.Colors[ImGuiCol_Button] = Color;
	if (ImGui::Button(ButtonText.c_str(), ButtonSize)) {
		return true;
	}

	style.Colors[ImGuiCol_Text] = original_text_style;
	style.Colors[ImGuiCol_Button] = original_button_style;
	return ret;
}

void stylewrappers::HexDumpBackgroundStyle(const ImVec4& Color)
{
	ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
	ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
	contentMin.y -= 5;
	contentMax.x -= 10;
	contentMax.y += 20;
	ImU32 bgColor = Vec4ToU32(Color); // Change background color (R, G, B, A)
	ImGui::GetWindowDrawList()->AddRectFilled(contentMin, contentMax, bgColor);
}

bool stylewrappers::ColoredSelectable(const std::string& SelectableText, const ImVec4& Color)
{
	bool ret = false;
	ImGuiStyle& style = ImGui::GetStyle();
	auto original_text_style = style.Colors[ImGuiCol_Text];
	style.Colors[ImGuiCol_Text] = Color;
	if (ImGui::Selectable(SelectableText.c_str()))
	{
		ret = true;
	}

	style.Colors[ImGuiCol_Text] = original_text_style;
	return ret;
}

void stylewrappers::ColoredInputText(const ImVec4& Color, const std::string& Label, char* Buffer, size_t BufferSize, ImGuiInputTextFlags Flags)
{
	ImGuiStyle& style = ImGui::GetStyle();
	auto original_text_style = style.Colors[ImGuiCol_Text];
	style.Colors[ImGuiCol_Text] = Color;
	ImGui::InputText(Label.c_str(), Buffer, BufferSize, Flags);
	style.Colors[ImGuiCol_Text] = original_text_style;
}



stylewrappers::TableStyle::TableStyle(const ImVec4& ColHeaderBgColor, const ImVec4& ColHeaderTextColor)
{
	ImGuiStyle& style = ImGui::GetStyle();
	m_OriginalColHeaderColor = style.Colors[ImGuiCol_TableHeaderBg];
	m_OriginalColHeaderTextColor = style.Colors[ImGuiCol_Text];
	style.Colors[ImGuiCol_TableHeaderBg] = ColHeaderBgColor;
	style.Colors[ImGuiCol_Text] = ColHeaderTextColor;

}

stylewrappers::TableStyle::~TableStyle()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TableHeaderBg] = m_OriginalColHeaderColor;
	style.Colors[ImGuiCol_Text] = m_OriginalColHeaderTextColor;
}
