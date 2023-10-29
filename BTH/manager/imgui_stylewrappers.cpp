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


bool stylewrappers::Button(const std::string& ButtonText, const ImVec4& Color, ImVec2 ButtonSize)
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = Color;
	if (ImGui::Button(ButtonText.c_str(), ButtonSize)) {
		return true;
	}
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
