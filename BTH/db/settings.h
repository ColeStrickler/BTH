#pragma once
#include <vector>
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"



// We will use VISUALS_INDEX to index into our color settings vector
// and also as our primary key for accessing saved color values
// in our data base schema
enum class VISUALS_INDEX : short
{
	// GENERAL VISUALS
	BACKGROUND_COLOR,

	// HEXDUMP VISUALS
	HEXDUMP_BACKGROUND_COLOR,
	HEXDUMP_BUTTON_COLOR,
	HEXDUMP_TEXT_COLOR,

	// DISASSEMBLY VISUALS
	DISASSEMBLY_INST1_COLOR,
	DISASSEMBLY_INST2_COLOR,
	DISASSEMBLY_INST3_COLOR,

	// MEMORY DUMP VISUALS


	// PE DUMP VISUALS
	PEPARSER_BUTTON_COLOR,
	PEPARSER_TEXT_COLOR
};



enum class PERFORMANCE_INDEX : short
{

};



struct ManagerSettings
{
	ManagerSettings(std::vector<ImVec4> InitVec);
	ManagerSettings();

	
	inline ImVec4 GetColorSettings(VISUALS_INDEX index) const { return m_ColorSettings[(int)index]; };


private:
	std::vector<ImVec4> m_ColorSettings;

};