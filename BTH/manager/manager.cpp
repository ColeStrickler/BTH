#include "manager.h"

static std::string ucharToHexString(unsigned char value) {
	std::stringstream stream;
	stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
	return stream.str();
}


static std::string ucharVecToHexString(std::vector<unsigned char>& vec)
 {
	std::string ret;
	for (auto& c : vec)
	{
		ret += ucharToHexString(c) + " ";
	}
	return ret;
}

/*
	This function replaces raw string formatting that is required by pybind11 for executing strings of text.
	By surrounding our scripts in newline characters we can avoid using raw strings which are super annoying to work with
*/
static std::string format_func(const std::string& format)
{
	std::string ret = std::format("\n{}\n", format);

	return ret;
}


Manager::Manager() : m_HexDumpWidth(475), m_HexDumpHeight(400), m_DecoderWidth(475), m_DecoderHeight(400), m_PEtableWidth(1600), m_PEtableHeight(500),
	m_DecoderNumInstructionsDisplayed(200), m_MaximumLoadSize(200000), m_StringScannerMaxStringsDisplayed(250)
{
	m_FileBrowser = new FileBrowser(m_MaximumLoadSize);
	m_Decoder = new Decoder();
	m_ByteScanner = new Scanner();
	m_DataBaseManager = new db_mgr();
	m_PythonInterpreter = new interpreter(this);
	auto t = std::thread(&Manager::BeginThreadManagerThread, this);
	m_ThreadManagerThread = std::move(t);
	InitDefaultStructs();
	m_InterpreterThread = std::thread(&Manager::InterpreterThread, this);
}

Manager::~Manager()
{
	m_bThreadManagerExit = true;
	m_ThreadManagerThread.join();
	
	// We do detach instead of join as it allows us to exit without error if
	// the script is in an infinite loop
	m_InterpreterThread.detach();
}


/*
	Main Loop
*/

void Manager::RenderUI()
{
	ImGui::Columns(3, "global_cols", true);

	/*
		COLUMN 1
	*/

	HandleFileNavigator();
	ImGui::SameLine();
	HandleSettings();
	ImGui::SameLine();
	HandleMemoryDumpButton();


	// Hex Dump
	auto sel = DrawHexValuesWindow();
	if (sel != -1)
	{
		m_bShowHexDumpHexEditPopup = true;
		m_HexDumpSelectedIndex = sel;
	}
	HandleHexdump();



	HandleByteScanner();
	ImGui::SameLine();
	HandleStringScanner();

	/*
		COLUMN 2
	*/
	ImGui::NextColumn();
	if (!m_bShowMemoryDumpView)
		HandleDecoder();
	else
	{
		HandleMemoryDumpStructureView();
	}
	/*
		COLUMN 3

	*/
	ImGui::NextColumn();
	if (m_bShowMemoryDumpView)
	{
		
		HandleMemoryDumpStructureEditor();
	}
	else
	{

		/*
			LETS THREAD OUT THE EXEC() FUNCTION EVENTUALLY, THIS WILL LET LONG RUNNING SCRIPTS RUN IN THE BACKGROUND
			WHILE THE APP IS ALLOWED TO STILL BE USEABLE
		*/
		ImGui::Dummy(ImVec2(100, 20));	// Same size as navigator buttons
		// WE WILL NEED TO ADD CUSTOM COLOR SETTINGS FOR THIS
		stylewrappers::MultilineText(GetColorSetting(VISUALS_INDEX::INTERPRETER_BACKGROUND_COLOR), GetColorSetting(VISUALS_INDEX::INTERPRETER_TEXT_COLOR), \
			m_PythonInterpreter->m_ScriptBuffer, 8192, ImVec2(WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2 - 100));

		if (stylewrappers::Button("Run", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{		
			std::string script = format_func(m_PythonInterpreter->m_ScriptBuffer + 1).c_str();
			{
				std::lock_guard<std::mutex> lock(m_InterpreterWorkItemsMutex);
				m_InterpreterWorkItems.push_back(script);
			}
			m_InterpreterCV.notify_all();	// we only have one thread here anyways
			//m_PythonInterpreter->exec();
		}

	}


	ImGui::Columns(1);
	HandlePeDump();
	
	

	// Do Other Maintenance
	BackgroundColorSettings();


}


void Manager::BeginThreadManagerThread()
{
	while (!m_bThreadManagerExit)
	{
		if (m_ActiveThreads.size())
		{
			std::lock_guard<std::mutex> lock(m_ThreadManagerMutex);
			for (int i = m_ActiveThreads.size() - 1; i >= 0; i--)
			{
				auto& t = m_ActiveThreads[i];
				t->join();
			}
			m_ActiveThreads.clear();
		}
	}
}


/*
	Since we do not set the background setting in any other functions in our render loop,
	we give it its own function here
*/
void Manager::BackgroundColorSettings()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = GetColorSetting(VISUALS_INDEX::BACKGROUND_COLOR);
}


/*
	This function will grab all the structs saved in the database
	and add them to our Manager class for rendering
*/
void Manager::InitDefaultStructs()
{
	// load default structures into our structure vector
	for (auto& def_struct : m_DataBaseManager->m_DefaultStructs)
	{
		MemDumpStructure new_struct_def(def_struct.m_Name);

		for (auto& member : def_struct.m_Entry)
		{
			new_struct_def.AddEntry(member);
		}
		m_MemoryDumpStructureVec.push_back(new_struct_def);
	}
}

void Manager::HandleMemoryDump()
{
	
}


/*
	This function handles the button that toggles between memory dump view and disassembler view
*/
void Manager::HandleMemoryDumpButton()
{
	std::string display = (m_bShowMemoryDumpView ? "HexDump" : "MemoryDump");


	if (stylewrappers::Button(display, GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bShowMemoryDumpView = !m_bShowMemoryDumpView;
	}
}


/*
	This function handles rendering the structure overlay table in memory dump view
*/
void Manager::HandleMemoryDumpStructureView()
{
	// need to fix this offset
	int offset = (m_GlobalOffset - m_FileBrowser->m_CurrentBounds[0]) % m_MaximumLoadSize;
	if (m_MemoryDumpNewStructure_CurrentlySelected >= m_MemoryDumpStructureVec.size())
		return;
	auto structure = m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected];
	ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Currently Selected: %s", structure.m_Name.c_str());
	if (ImGui::BeginChild("StructureDump", ImVec2(WINDOW_WIDTH / 3, WINDOW_HEIGHT / 2 - 100)))
	{
		if (ImGui::BeginTable("Structure Dump View", 4))
		{
			{
				stylewrappers::TableStyle TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
				ImGui::TableSetupColumn("Field");
				ImGui::TableSetupColumn("Value");
				ImGui::TableSetupColumn("Size");
				ImGui::TableSetupColumn("Type");
				ImGui::TableHeadersRow();
			}
			
			auto data = structure.GetDisplayData(m_FileBrowser->m_FileLoadData, offset);
			int rows = data.size();
			for (int r = 0; r < rows; r++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), "%s", data[r].m_SE.m_GivenName.c_str());;
				ImGui::TableSetColumnIndex(1);
				ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), "%s", data[r].m_Display.c_str());;
				ImGui::TableSetColumnIndex(2);
				ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), "%d", data[r].m_SE.m_Size);
				ImGui::TableSetColumnIndex(3);
				ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), "%s", DumpDisplayType2String(data[r].m_SE.m_Display).c_str());
			}
			ImGui::EndTable();
		}

		ImGui::EndChild();
	}


}


/*
	This function renders the structure editor table in memory dump view
*/
void Manager::HandleMemoryDumpStructureEditor()
{
	if (ImGui::BeginChild("StructureEdit", ImVec2(WINDOW_WIDTH/3, WINDOW_HEIGHT/2 - 50)))
	{
		ImGui::Columns(2, "StructureEdit_cols", false);
		ImGui::SetColumnWidth(0, WINDOW_WIDTH / 12);

		if (ImGui::BeginTable("Structure Editor", 1, 0, ImVec2(WINDOW_WIDTH/12, WINDOW_HEIGHT/2)))
		{
			{
				stylewrappers::TableStyle TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
				ImGui::TableSetupColumn("Structure", ImGuiTableColumnFlags_WidthFixed, WINDOW_WIDTH / 12);
				ImGui::TableHeadersRow();
			}

			auto& structVec = m_MemoryDumpStructureVec;
			auto rows = structVec.size();
			for (int r = 0; r < rows; r++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::PushID(r);
				if (stylewrappers::ColoredSelectable(structVec[r].m_Name, GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR)))
				{
					m_MemoryDumpNewStructure_CurrentlySelected = r;
				}
				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		ImGui::NextColumn();
		if (ImGui::BeginTable("Structure Editor", 3))
		{
			{
				stylewrappers::TableStyle TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
				ImGui::TableSetupColumn("Field", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Data Type", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableHeadersRow();
			}
			if (m_MemoryDumpNewStructure_CurrentlySelected < m_MemoryDumpStructureVec.size())
			{
				// We call an overload of GetDisplayData(), this one does not set the member variable or overlay struct onto memory
				auto display_data = m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected].GetDisplayData();
				auto rows = display_data.size();
				for (int r = 0; r < rows; r++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushID(10 * r + 0);
					if (stylewrappers::ColoredSelectable(display_data[r].m_SE.m_GivenName, GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR)))
					{
						m_MemoryDumpNewStructure_MemberSelected.first = r;
						m_MemoryDumpNewStructure_MemberSelected.second = 0;
						m_bMemoryDumpShowMemberEditPopup = true;
					}
					ImGui::PopID();
					ImGui::TableSetColumnIndex(1);
					ImGui::PushID(10 * r + 1);
					if (stylewrappers::ColoredSelectable(DumpDisplayType2String(display_data[r].m_SE.m_Display), GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR)))
					{
						m_MemoryDumpNewStructure_MemberSelected.first = r;
						m_MemoryDumpNewStructure_MemberSelected.second = 1;
						m_bMemoryDumpShowMemberEditPopup = true;
					}
					ImGui::PopID();
					ImGui::TableSetColumnIndex(2);
					ImGui::PushID(10 * r + 2);
					if (stylewrappers::ColoredSelectable(std::to_string(display_data[r].m_SE.m_Size), GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR)))
					{
						m_MemoryDumpNewStructure_MemberSelected.first = r;
						m_MemoryDumpNewStructure_MemberSelected.second = 2;
						m_bMemoryDumpShowMemberEditPopup = true;
					}
					ImGui::PopID();
				}
			}
			
			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
	HandleMemoryDumpMemberEditPopup();
	if (stylewrappers::Button("New Struct", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bMemoryDumpShowAddStructurePopup = true;
	}
	HandleMemoryDumpNewStructurePopup();
	ImGui::SameLine();
	if (stylewrappers::Button("New Member", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		if (m_MemoryDumpNewStructure_CurrentlySelected < m_MemoryDumpStructureVec.size())
		{
			MemDumpStructEntry se;
			se.m_Display = MEMDUMPDISPLAY::INT;
			se.m_Size = 4;
			m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected].AddEntry(se);
		}
		
	}
	ImGui::SameLine();
	if (stylewrappers::Button("Remove Member", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		if (m_MemoryDumpStructureVec.size())
			m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected].RemoveEntry();
	}
	ImGui::SameLine();
	if (stylewrappers::Button("Save Struct", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		if (m_MemoryDumpNewStructure_CurrentlySelected < m_MemoryDumpStructureVec.size())
		{
			auto& structure = m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected];
			m_DataBaseManager->SaveStructure(structure);
		}
	}
	
	if (stylewrappers::Button("Delete Struct", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		if (m_MemoryDumpNewStructure_CurrentlySelected < m_MemoryDumpStructureVec.size())
		{
			auto& structure = m_MemoryDumpStructureVec[m_MemoryDumpNewStructure_CurrentlySelected];
			// delete the structure member from the Manager class vector by its index
			m_DataBaseManager->DeleteStructure(structure);
			m_MemoryDumpStructureVec.erase(m_MemoryDumpStructureVec.begin() + m_MemoryDumpNewStructure_CurrentlySelected);
			
		}
	}


}

/*
	This function handles the popup for the "New Structure" button

	The actually adding of the new struct to m_MemoryDumpStructureVec is done here as well
*/
void Manager::HandleMemoryDumpNewStructurePopup()
{

	if (m_bMemoryDumpShowAddStructurePopup)
	{
		ImGui::OpenPopup("New Structure");
	}

	if (ImGui::BeginPopup("New Structure"))
	{
		ImGui::InputText("Structure Name", m_MemoryDumpNewStructureBuffer, 20);
		ImGui::SameLine();
		if (stylewrappers::Button("Add Structure", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{
			auto struct_name = std::string(m_MemoryDumpNewStructureBuffer);
			m_MemoryDumpStructureVec.push_back(MemDumpStructure(struct_name));
		}
		ImGui::SameLine();
		if (stylewrappers::Button("Done", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{
			m_bMemoryDumpShowAddStructurePopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

}

void Manager::HandleMemoryDumpMemberEditPopup()
{
	int& selected_struct_index = m_MemoryDumpNewStructure_CurrentlySelected;
	int& selected_member = m_MemoryDumpNewStructure_MemberSelected.first;
	int& selected_field = m_MemoryDumpNewStructure_MemberSelected.second;
	if (selected_struct_index >= m_MemoryDumpStructureVec.size())
		return;
	auto& selected_struct = m_MemoryDumpStructureVec[selected_struct_index];
	auto& selected_item = selected_struct.GetSelectedEntry(selected_member);


	if (m_bMemoryDumpShowMemberEditPopup)
	{
		ImGui::OpenPopup("Member Edit");
	}

	if (ImGui::BeginPopup("Member Edit"))
	{
		
		int selected_DisplayType;
		int input_size;
		ImGui::Text("Field: %s\tDisplayType: %s\t Size: %ld", selected_item.m_GivenName.c_str(), DumpDisplayType2String(selected_item.m_Display).c_str(), selected_item.m_Size);
		switch (selected_field)
		{
			case 0:
			{
				ImGui::InputText("Field", m_MemoryDumpMemberEditBuffer, 20);
				break;
			}
			case 1:
			{
				const char* items[] = { "INT","LONG_INT","UNSIGNED_INT","UNSIGNED_LONGLONG","ASCII","UNICODE","HEX", "BOOL"};
				for (int i = 0; i < 8; i++)
				{
					if (ImGui::Selectable(items[i]))
					{
						m_MemoryDumpMemberEditDisplaySelector = (MEMDUMPDISPLAY)i; 
					}
				}
				break;
			}
			case 2:
			{
				ImGui::InputInt("Size", &m_MemoryDumpMemberEditSizeSelector);
				break;
			}
			default:
				break;
		}

		if (stylewrappers::Button("Apply Changes", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{
			switch (selected_field)
			{
			case 0:
			{
				selected_struct.EditFieldEntry(selected_member, std::string(m_MemoryDumpMemberEditBuffer));
				break;
			}
			case 1:
			{
				selected_struct.EditDisplayEntry(selected_member, m_MemoryDumpMemberEditDisplaySelector);
				break;
			}
			case 2:
			{
				selected_struct.EditSizeEntry(selected_member, m_MemoryDumpMemberEditSizeSelector);
				break;
			}
			default:
				break;
			}

			memset(m_MemoryDumpMemberEditBuffer, 0x00, 20);
		}
		ImGui::SameLine();
		if (stylewrappers::Button("Done", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{
			m_bMemoryDumpShowMemberEditPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

}
	



	





void Manager::HandleSettings()
{
	HandleSettingsButton();
	HandleSettingsPopup();
}

void Manager::HandleSettingsButton()
{
	if (stylewrappers::Button("Settings", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bSettingsShowPopup = true;
	}
}

void Manager::HandleSettingsPopup()
{

	if (m_bSettingsShowPopup)
	{
		ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(800, 500)); // Set the desired size
		ImGui::Begin("Settings", &m_bSettingsShowPopup);
		HandleSettingsDisplay();
		

		if (ImGui::Button("Done"))
		{
			m_bSettingsShowPopup = false;
			//ImGui::CloseCurrentPopup();
		}
		//ImGui::EndPopup();

		ImGui::End();
	}

}

void Manager::HandleSettingsDisplay()
{
	switch (m_SettingsCurrentDisplay)
	{
		case SETTINGS_DISPLAY::VISUALS:
		{
			DisplayVisualSettings();
			return;
		}
		case SETTINGS_DISPLAY::PERFORMANCE:
		{
			DisplayPerformanceSettings();
			return;
		}
		default:
			return;
	}
}

void Manager::DisplayVisualSettings()
{
	ImGui::Columns(2);
	if (ImGui::BeginTable("Visual Settings", 5))
	{
		{
			stylewrappers::TableStyle TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed, 200.0f);
			ImGui::TableSetupColumn("R", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("G", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("B", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("A", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();
		}
		
		auto rows = COLORSETTINGS_QUERYSTRING.size();
		for (int r = 0; r < rows; r++)
		{
			auto curr_color = GetColorSetting((VISUALS_INDEX)r);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (stylewrappers::ColoredSelectable(COLORSETTINGS_QUERYSTRING[r].c_str(), GetColorSetting(VISUALS_INDEX::TEXT_COLOR)))
			{
				m_CurrentSelectedVisualsIndex = (VISUALS_INDEX)r;
				m_VisualSettingsColorSelector = curr_color;
			}
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%.2f", curr_color.x);
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%.2f", curr_color.y);
			ImGui::TableSetColumnIndex(3);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%.2f", curr_color.z);
			ImGui::TableSetColumnIndex(4);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%.2f", curr_color.w);
		}
		ImGui::EndTable();
	}
	ImGui::NextColumn();
	ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Currently selected: %s", COLORSETTINGS_QUERYSTRING[(int)m_CurrentSelectedVisualsIndex].c_str());
	ImGui::SliderFloat("Red",	&m_VisualSettingsColorSelector.x, 0.0f, 1.0f);
	ImGui::SliderFloat("Green", &m_VisualSettingsColorSelector.y, 0.0f, 1.0f);
	ImGui::SliderFloat("Blue",	&m_VisualSettingsColorSelector.z, 0.0f, 1.0f);
	ImGui::ColorButton("MyColoredBox", m_VisualSettingsColorSelector, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip, ImVec2(300, 20));

	// Update Database with changes and reload settings to match updates
	if (stylewrappers::Button("Apply Changes", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_DataBaseManager->UpdateColorSetting(COLORSETTINGS_QUERYSTRING[(int)m_CurrentSelectedVisualsIndex], m_VisualSettingsColorSelector);
	}
	ImGui::SameLine();
}

void Manager::DisplayPerformanceSettings()
{
}

ImVec4 Manager::GetColorSetting(VISUALS_INDEX index)
{
	std::lock_guard<std::mutex> lock(m_FunctionBindingsMutex);	// we do this to avoid data races with certain binding functions
	return m_DataBaseManager->GetColorSetting(index);
}




/*
	File Naviator
*/

void Manager::HandleFileNavigatorButtons()
{
	if (stylewrappers::Button("Select File", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bShowFileSelectPopup = true;
	}
}
void Manager::HandleFileNavigatorPopups()
{
	if (m_bShowFileSelectPopup)
	{
		ImGui::OpenPopup("Select File");
	}

	if (ImGui::BeginPopup("Select File"))
	{

		DisplayFileNavigator();
		
		if (stylewrappers::Button("Done", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR)))
		{
			m_bShowFileSelectPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void Manager::DisplayFileNavigator()
{
	auto& InputPath = m_FileBrowserNavigationBuffer;
	ImGui::InputText("Enter path", InputPath, sizeof(InputPath));
	m_FileBrowser->SetInputPath(InputPath);
	m_FileBrowser->ListDirectory(InputPath);
	//auto& m_CurrentDirectory = m_FileBrowser->m_CurrentDirectory;

	// Begin a scrollable child window with a fixed height of 300 pixels
	ImGui::BeginChild("SelectableList", ImVec2(600, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
	auto filtered = m_FileBrowser->DisplayFilter();
	for (const auto& r :  filtered)
	{
		auto str = r.path().string();
		auto c_str = str.c_str();
		if (ImGui::Selectable(c_str))
		{
			if (std::filesystem::is_directory(r))
			{
				m_FileBrowser->SetInputPath(c_str);
				memcpy(m_FileBrowserNavigationBuffer, c_str, str.size());
			}
			else
			{
				m_FileBrowser->SetInputPath(c_str);
				auto fb_retcode = m_FileBrowser->LoadFile(str);
				

				if (fb_retcode == FB_RETCODE::FILE_CHANGE_LOAD)
				{
					if (m_PEDisector != nullptr)
						delete m_PEDisector;
					m_PEDisector = new PEDisector(m_FileBrowser->m_LoadedFileName);
					
				}
				m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData, m_PEDisector);
			}
		}
	}
	ImGui::EndChild();
}
void Manager::HandleFileNavigator()
{
	HandleFileNavigatorButtons();
	HandleFileNavigatorPopups();
}


/*
	Hex Dump
*/


int Manager::DrawHexValuesWindow()
{
	
	ImGui::BeginChild("ScrollingRegion", ImVec2(m_HexDumpWidth, m_HexDumpHeight), false);
	stylewrappers::HexDumpBackgroundStyle(GetColorSetting(VISUALS_INDEX::HEXDUMP_BACKGROUND_COLOR));


	std::vector<unsigned char>& hexValues = m_FileBrowser->m_FileLoadData;
	if (hexValues.size() == 0)
	{
		ImGui::EndChild();
		return -1;
	}
		
	bool& showAscii = m_bHexDumpShowAscii;
	int& offset = m_GlobalOffset;

	// Create a scrollable region

	int total = 0;
	const int bytesPerRow = 16;
	const int numRows = 16;
	int ret = -1;


	// Some vertical spacing 
	ImGui::Spacing();
	ImGui::Spacing();

	// if we pull these out so they are only called once we can optimize use of the mutex and speed our scripts that
	// change colors up
	auto hex_dump_button_color = GetColorSetting(VISUALS_INDEX::HEXDUMP_BUTTON_COLOR);
	auto hex_dump_buttontext_color = GetColorSetting(VISUALS_INDEX::HEXDUMP_TEXT_COLOR);

	for (int row = 0; row < numRows; ++row) {
		for (int col = 0; col < bytesPerRow; ++col) {
			int offset = (m_GlobalOffset - m_FileBrowser->m_CurrentBounds[0]) % m_MaximumLoadSize;
			int index = row * bytesPerRow + col + offset;

			ImGui::SameLine();
			if (index < hexValues.size()) {
				ImGui::PushID(index);

				std::string value;
				if (!showAscii)
					value = ucharToHexString(hexValues[index]);
				else
					value += (char)hexValues[index];


				//ImGui::Button(value.c_str(), ImVec2(20, 20);
				if (stylewrappers::Button(value, hex_dump_button_color,
					ImVec2(20, 20), hex_dump_buttontext_color))
				{
					ret = index;
				}

				ImGui::PopID();
			}
			else {
				ImGui::Text("  "); // Placeholder for empty cells
			}
		}
		ImGui::Spacing();
	}

	ImGui::EndChild();
	return ret;
}
void Manager::HandleHexdumpPopups()
{
	// HEX EDIT POPUP
	if (m_bShowHexDumpHexEditPopup)
	{
		ImGui::OpenPopup("EditHexValuePopup");
	}

	if (ImGui::BeginPopup("EditHexValuePopup"))
	{
		ImGui::InputText("Hex Value", m_HexDumpHexEditorBuffer, 3);
		ImGui::Text("Current Value: 0x%x", m_FileBrowser->m_FileLoadData[m_HexDumpSelectedIndex]);
		if (stylewrappers::Button("Set", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			if (m_HexDumpSelectedIndex != -1)
			{
				int offset = (m_GlobalOffset - m_FileBrowser->m_CurrentBounds[0]) % m_MaximumLoadSize;
				int newHexValue;
				unsigned int parsedValue = 0;
				std::istringstream(m_HexDumpHexEditorBuffer) >> std::hex >> parsedValue;
				m_FileBrowser->EditByte(m_HexDumpSelectedIndex, static_cast<unsigned char>(parsedValue)); // We have to use the EditByte() call to make sure edits can be saved
			}

			ImGui::CloseCurrentPopup();
			m_bShowHexDumpHexEditPopup = false;
			m_HexDumpSelectedIndex = -1;
			memset(m_HexDumpHexEditorBuffer, 0x00, 2);

		}

		ImGui::EndPopup();
	}


	// SAVE FILE POPUP
	if (m_bShowSaveFilePopup)
	{
		ImGui::OpenPopup("SaveFilePopup");
	}

	if (ImGui::BeginPopup("SaveFilePopup"))
	{
		ImGui::InputText("Save to path:", m_SaveFilePathBuffer, _MAX_PATH);
		bool show_error_text = false;

		m_FileBrowser->SetInputPath(m_SaveFilePathBuffer);
		m_FileBrowser->ListDirectory(m_SaveFilePathBuffer);
		ImGui::BeginChild("SelectableList", ImVec2(600, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
		auto filtered = m_FileBrowser->DisplayFilter();
		for (const auto& r : filtered)
		{
			auto str = r.path().string();
			auto c_str = str.c_str();
			if (ImGui::Selectable(c_str))
			{
				if (std::filesystem::is_directory(r) || std::filesystem::is_regular_file(r))
				{
					m_FileBrowser->SetInputPath(c_str);
					memcpy(m_SaveFilePathBuffer, c_str, str.size());
				}

			}
		}
		ImGui::EndChild();

		
		if (stylewrappers::Button("Save File", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			if (fs::exists(m_SaveFilePathBuffer))
			{
				if (!fs::remove(m_SaveFilePathBuffer))
				{
					show_error_text = true;
				}
				else
				{
					show_error_text = !m_FileBrowser->SaveFile(m_SaveFilePathBuffer);

				}		
			}
			else
			{
				show_error_text = !m_FileBrowser->SaveFile(m_SaveFilePathBuffer);
			}
	
			

		}
		ImGui::SameLine();
		if (stylewrappers::Button("Done", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			ImGui::CloseCurrentPopup();
			m_bShowSaveFilePopup = false;
			memset(m_SaveFilePathBuffer, 0x00, _MAX_PATH);
		}


		if (fs::exists(m_SaveFilePathBuffer))
		{
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Warning: File already exists. File on disk will be overwritten.");
		}


		if (show_error_text)
		{
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Error: could not save file");
		}


		ImGui::EndPopup();
	}
	

}

void Manager::HandleHexdumpButtons()
{
	/*
		Here we grab the input for the offset editor, convert it to hex, and check if it is different than the current offset.
		If it is, then we call LoadFile to see if we need to load a different portion of the file
	*/
	ImGui::SetNextItemWidth(200.0f);
	stylewrappers::ColoredInputText(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Offset", m_OffsetEditorBuffer, sizeof(m_OffsetEditorBuffer), ImGuiInputTextFlags_CharsHexadecimal);
	std::string offset(m_OffsetEditorBuffer);
	int new_offset = utils::stringToHex(offset);


	// We check to see if we need to load a different portion of the file when the offset changes
	if (m_GlobalOffset != new_offset)
	{
		m_GlobalOffset = new_offset;
		auto parse_inst = m_FileBrowser->LoadFile(m_FileBrowser->m_LoadedFileName, m_GlobalOffset);
		if (parse_inst == FB_RETCODE::FILE_CHANGE_LOAD || parse_inst == FB_RETCODE::OFFSET_CHANGE_LOAD)
		{
			
			m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData, m_PEDisector);
		}
	}
	
	ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Loaded File: %s\t File Size: %x\n", m_FileBrowser->m_LoadedFileName.c_str(), m_FileBrowser->m_LoadedFileSize);
	
	// Show error message if offset is bigger than file
	if (m_GlobalOffset > m_FileBrowser->m_LoadedFileSize)
	{
		
		ImGui::TextColored(RedFont, "[ERROR]: Offset of %x is larger than file size.", m_GlobalOffset);
	}

	// ASCII Button
	if (m_bHexDumpShowAscii)
	{
		if (stylewrappers::Button("Show Hex", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			m_bHexDumpShowAscii = !m_bHexDumpShowAscii;
		}
	}
	else
	{
		if (stylewrappers::Button("Show Ascii", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			m_bHexDumpShowAscii = !m_bHexDumpShowAscii;
		}
	}
	ImGui::SameLine();

	// SAVE FILE BUTTON
	if (stylewrappers::Button("Save File", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bShowSaveFilePopup = true;
	}
	ImGui::SameLine();


}

void Manager::HandleHexdump()
{
	HandleHexdumpPopups();
	HandleHexdumpButtons();
}


/*
	Decoder 
*/

void Manager::HandleDecoderButtons()
{

}

void Manager::HandleDecoderPopups()
{
}

void Manager::HandleDecoder()
{
	ImVec2 buttonSize;
	buttonSize.x = ImGui::CalcTextSize("Select File").x + ImGui::GetStyle().FramePadding.x * 2;
	buttonSize.y = ImGui::CalcTextSize("Select File").y + ImGui::GetStyle().FramePadding.y * 2;

	ImGui::Dummy(buttonSize); // Add 10 units of padding
	stylewrappers::DisassemblerBackground DISASSEMBLER_BG_COLOR(GetColorSetting(VISUALS_INDEX::DISASSEMBLY_BACKGROUND_COLOR));
	ImGui::BeginChild("DisassemblyRegion", ImVec2(WINDOW_WIDTH/3 - 17, WINDOW_HEIGHT/2 - 75), false);

	
	auto offset = m_Decoder->m_OffsetToInstIndex[m_GlobalOffset];

	for (int i = offset; i < offset + m_DecoderNumInstructionsDisplayed
		&& i < m_Decoder->m_DecodedBytes.size() ; i++)
	{
		auto& inst = m_Decoder->m_DecodedBytes[i];
		ImGui::TextColored(GetFont(inst), "0x%x: %s", inst.m_Offset + (m_GlobalOffset > m_MaximumLoadSize ? m_GlobalOffset : 0), inst.m_DecodedInstruction.c_str());
	}
	ImGui::EndChild();
}

ImVec4 Manager::GetFont(DecodedInst& inst)
{
	if (inst.m_DecodedInstruction.substr(0, 3) == "mov")
	{
		return GetColorSetting(VISUALS_INDEX::DISASSEMBLY_INST1_COLOR);
	}
	if (inst.m_DecodedInstruction.substr(0, 1) == "j")
	{
		return GetColorSetting(VISUALS_INDEX::DISASSEMBLY_INST2_COLOR);
	}
	if (inst.m_DecodedInstruction.substr(0, 3) == "sub" || inst.m_DecodedInstruction.substr(0, 3) == "add")
	{
		return GetColorSetting(VISUALS_INDEX::DISASSEMBLY_INST3_COLOR);
	}
	return GetColorSetting(VISUALS_INDEX::DISASSEMBLY_INST4_COLOR);
}




/*
	Byte Scanner
*/

void Manager::HandleByteScanner()
{
	HandleByteScannerButton();
	HandleByteScannerPopup();
}

void Manager::HandleByteScannerPopupButton()
{
	if (stylewrappers::Button("Scan", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{

		m_ByteScannerPattern.clear();
		for (int i = 0; i < m_ByteScannerInputPatternSize; i++)
		{
			m_ByteScannerPattern.push_back(m_ByteScannerPatternBuffer[i]);
		}
		m_ByteScannerBytesScanned = m_FileBrowser->m_LoadedFileSize;

		std::thread scan_thread(&Scanner::byte_scan_file, m_ByteScanner, m_FileBrowser, std::ref(m_ByteScannerPattern), this);
		{
			std::lock_guard<std::mutex> lock(m_ThreadManagerMutex);
			m_ActiveThreads.push_back(std::make_unique<std::thread>(std::move(scan_thread)));
		}
	}
	ImGui::SameLine();
	if (stylewrappers::Button("Close", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bByteShowScannerPopup = false;

	}



	ImGui::ProgressBar(m_ByteScannerProgress, ImVec2(150, 30));
	if (m_bByteScannerFinished)
	{
		ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Scanned %d bytes in %.2f s.", m_ByteScannerBytesScanned, m_ByteScanner->m_ByteScanTime.count());
		ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Found %d matches.", m_ByteScanner->m_ByteMatches.size());

		for (int i = 0; i < min(m_ByteScanner->m_ByteMatches.size(), MAX_SCANNER_DISPLAY); i++)
		{
			auto& inst = m_ByteScanner->m_ByteMatches[i];
			ImGui::TextColored(GreenFont, "0x%x", inst);
		}
	}
	

}

void Manager::HandleByteScannerButton()
{
	if (stylewrappers::Button("Byte Scan", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bByteShowScannerPopup = true;
	}

}

void Manager::HandleByteScannerPopup()
{
	if (m_bByteShowScannerPopup)
	{
		ImGui::OpenPopup("Byte Scan");
	}

	if (ImGui::BeginPopup("Byte Scan"))
	{
		ImVec2 buttonSize(100, 20); // Adjust the size as needed
		ImGui::InputInt("Pattern Size:", &m_ByteScannerInputPatternSize, 1, 16);
		if (m_ByteScannerInputPatternSize > 16)
			m_ByteScannerInputPatternSize = 16;
		else if (m_ByteScannerInputPatternSize < 1)
			m_ByteScannerInputPatternSize = 1;


		
		for (int i = 0; i < m_ByteScannerInputPatternSize; i++)
		{
			
			
			ImGui::PushID(i);

			auto value = ucharToHexString(m_ByteScannerPatternBuffer[i]);
			if (stylewrappers::Button(value, GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(20, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR))) {
				m_ByteScannerSelectedIndex = i;
				m_bByteScannerShowPatternEditPopup = true;
			}
			ImGui::PopID();
			if (i != 7 && i != m_ByteScannerInputPatternSize - 1)
				ImGui::SameLine();
		}

		HandleByteScannerPopupButton();
		
		if (!m_bByteShowScannerPopup)
		{
			ImGui::CloseCurrentPopup();
		}


		if (m_bByteScannerShowPatternEditPopup)
		{
			ImGui::OpenPopup("Edit Byte");
		}

		if (ImGui::BeginPopup("Edit Byte"))
		{
			ImGui::InputText("Hex Value", m_ByteScannerPatternEditorBuffer, 3);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Current Value: 0x%x", m_ByteScannerPatternBuffer[m_ByteScannerSelectedIndex]);
			if (stylewrappers::Button("Set", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
			{
				if (m_HexDumpSelectedIndex != -1)
				{
					int newHexValue;
					unsigned int parsedValue = 0;
					std::istringstream(m_ByteScannerPatternEditorBuffer) >> std::hex >> parsedValue;
					m_ByteScannerPatternBuffer[m_ByteScannerSelectedIndex] = static_cast<unsigned char>(parsedValue);
				}

				ImGui::CloseCurrentPopup();
				m_bByteScannerShowPatternEditPopup = false;
				m_ByteScannerSelectedIndex = -1;
				memset(m_ByteScannerPatternEditorBuffer, 0x00, 2);

			}

			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}

}

void Manager::HandleStringScanner()
{
	HandleStringScannerButton();
	HandleStringScannerPopup();
}

void Manager::HandleStringScannerButton()
{
	if (stylewrappers::Button("String Scan", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
	{
		m_bStringScannerShowPopup = !m_bStringScannerShowPopup;
	}
}

void Manager::HandleStringScannerPopup()
{


	// HEX EDIT POPUP
	if (m_bStringScannerShowPopup)
	{
		ImGui::OpenPopup("String Scanner Popup");
	}
	ImGui::SetNextWindowSize(ImVec2(700, 400));
	if (ImGui::BeginPopup("String Scanner Popup"))
	{
		ImGui::InputInt("Minimum String Size", &m_StringScannerMinStringLength, 1, 20, ImGuiInputTextFlags_CharsDecimal);
		if (ImGui::Button("Scan"))
		{
			std::thread scan_thread(&Scanner::string_scan_file, m_ByteScanner, m_FileBrowser, this, std::ref(m_StringScannerMinStringLength));
			{
				std::lock_guard<std::mutex> lock(m_ThreadManagerMutex);
				m_ActiveThreads.push_back(std::make_unique<std::thread>(std::move(scan_thread)));
			}
		}
		ImGui::SameLine();

		if (stylewrappers::Button("Close", GetColorSetting(VISUALS_INDEX::BUTTON_COLOR), ImVec2(100, 20), GetColorSetting(VISUALS_INDEX::BUTTON_TEXT_COLOR)))
		{
			m_bStringScannerShowPopup = false;
			ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
			return;
		}

		auto matches = m_ByteScanner->m_StringMatches;
		if (m_bStringScannerFinished)
		{
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "Scanned %d bytes for strings in %.2f s", m_FileBrowser->m_LoadedFileSize, m_ByteScanner->m_StringScanTime);
			ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%d ASCII matches, %d Unicode Matches", matches.m_StandardStrings.size(), matches.m_UnicodeStrings.size());
		}
		else
		{
			ImGui::SameLine();
			ImGui::ProgressBar(m_StringScannerProgress, ImVec2(200, 20));
		}

		
		if (matches.m_StandardStrings.size() || matches.m_UnicodeStrings.size())
		{
			ImGui::Columns(2);
			if (ImGui::BeginTable("Ascii Strings", 2))
			{

				{
					stylewrappers::TableStyle TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
					ImGui::TableSetupColumn("Offset");
					ImGui::TableSetupColumn("ASCII String");
					ImGui::TableHeadersRow();
				}
				auto strings = matches.m_StandardStrings;
				auto rows = strings.size();
				for (int r = 0; r < min(rows, m_StringScannerMaxStringsDisplayed); r++)
				{
					
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%x", strings[r].m_Offset);
					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%s", strings[r].m_StringVal.c_str());
				}
				ImGui::EndTable();
			}
			ImGui::NextColumn();
			if (ImGui::BeginTable("Unicode Strings", 2))
			{
				ImGui::TableSetupColumn("Offset");
				ImGui::TableSetupColumn("UNICODE String");
				ImGui::TableHeadersRow();

				auto strings = matches.m_UnicodeStrings;
				auto rows = strings.size();
				for (int r = 0; r < min(rows, m_StringScannerMaxStringsDisplayed); r++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%x", strings[r].m_Offset);
					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(GetColorSetting(VISUALS_INDEX::TEXT_COLOR), "%ws", strings[r].m_StringVal.c_str());
				}
				ImGui::EndTable();
			}

		}

		ImGui::EndPopup();
	}
}



/*
	PE Dump
*/

void Manager::HandlePeDump()
{
	HandlePeFileFormatButtons();
	if (!m_PEDisector)
		return;
	if (!m_PEDisector->ValidPE())
		return;
	if (!m_PEDisector->DisectionSuccessful())
		return;
	HandlePeDisplay();
}

void Manager::HandlePeFileFormatButtons()
{
	auto pe_button_color = GetColorSetting(VISUALS_INDEX::PEPARSER_BUTTON_COLOR);
	auto pe_buttontext_color = GetColorSetting(VISUALS_INDEX::PEPARSER_BUTTONTEXT_COLOR);

	if (stylewrappers::Button("DOS Header", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::DOS_HEADER;
	ImGui::SameLine();
	if (stylewrappers::Button("Rich Header", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::RICH_HEADER;
	ImGui::SameLine();
	if (stylewrappers::Button("File Header", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::FILE_HEADER;
	ImGui::SameLine();
	if (stylewrappers::Button("Optional Header", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::OPTIONAL_HEADER;
	ImGui::SameLine();
	if (stylewrappers::Button("Data Directories", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::DATA_DIRECTORIES;
	ImGui::SameLine();
	if (stylewrappers::Button("Section Headers", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::SECTION_HEADERS;
	ImGui::SameLine();
	if (stylewrappers::Button("Imports", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::IMPORTS;
	ImGui::SameLine();
	if (stylewrappers::Button("Exports", pe_button_color, ImVec2(150, 20), pe_buttontext_color))
		m_PEselected = PEINFO::EXPORTS;
}

void Manager::HandlePeDisplay()
{
	switch (m_PEselected)
	{
		case PEINFO::DOS_HEADER:
		{
			HandleDosHeader();
			return;
		}	
		case PEINFO::RICH_HEADER:
		{
			HandleRichHeader();
			return;
		}
		case PEINFO::FILE_HEADER:
		{
			HandleFileHeader();
			return;
		}
		case PEINFO::OPTIONAL_HEADER:
		{
			HandleOptionalHeader();
			return;
		}
		case PEINFO::DATA_DIRECTORIES:
		{
			HandleDataDirectories();
			return;
		}
		case PEINFO::SECTION_HEADERS:
		{
			HandleSectionHeaders();
			return;
		}
		case PEINFO::IMPORTS:
		{
			HandleImports();
			return;
		}
		case PEINFO::EXPORTS:
			return;
		default:
			return;
	}
}

void Manager::HandleDosHeader()
{
	if (ImGui::BeginTable("Dos Header", 3))
	{
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Offset");
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();
		}

		auto dosHeader = m_PEDisector->m_ParsedDosHeader;
		auto rows = dosHeader.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int r = 0; r < rows; r++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, "%x", dosHeader[r].m_Offset);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(text_color, dosHeader[r].m_Name.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(text_color, ucharVecToHexString(dosHeader[r].m_Bytes).c_str());
		}
		ImGui::EndTable();
	}
}

void Manager::HandleRichHeader()
{
	auto richHeader = m_PEDisector->m_ParsedRichHeader;
	auto rhEntries = richHeader.m_Entries;
	if (!rhEntries.size())
		return;
	auto ex_Entry = richHeader.m_Entries[0];
	
	auto num_Rows = rhEntries.size();
	auto key = richHeader.key;

	if (ImGui::BeginTable("Rich Header", 7))
	{
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableSetupColumn("Meaning");
			ImGui::TableSetupColumn("ProductId");
			ImGui::TableSetupColumn("BuildId");
			ImGui::TableSetupColumn("Use Count");
			ImGui::TableSetupColumn("VS Version");
			ImGui::TableHeadersRow();
		}

		auto rows = rhEntries.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int r = 0; r < rows; r++)
		{
			ImGui::TableNextRow();
			auto current = rhEntries[r];
			// handle new row
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, "Comp ID");
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(text_color, ucharVecToHexString(current.m_Raw).c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(text_color, RichHeaderMeaning(current).c_str());
			ImGui::TableSetColumnIndex(3);
			ImGui::TextColored(text_color, current.m_prodIDMeaning.c_str());
			ImGui::TableSetColumnIndex(4);
			ImGui::TextColored(text_color, "%d", current.m_buildID);
			ImGui::TableSetColumnIndex(5);
			ImGui::TextColored(text_color, "%d", current.m_useCount);
			ImGui::TableSetColumnIndex(6);
			ImGui::TextColored(text_color, current.m_vsVersion.c_str());
		}
		ImGui::EndTable();
	}
}

void Manager::HandleFileHeader()
{
	auto fileHeader = m_PEDisector->m_ParsedFileHeader;
	if (ImGui::BeginTable("File Header", 3))
	{
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Offset");
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();
		}

		auto rows = fileHeader.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int r = 0; r < rows; r++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, "%x", fileHeader[r].m_Offset);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(text_color, fileHeader[r].m_Name.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(text_color, ucharVecToHexString(fileHeader[r].m_Bytes).c_str());
		}
		ImGui::EndTable();
	}
}

void Manager::HandleOptionalHeader()
{
	if (ImGui::BeginTable("Optional Header", 3))
	{
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Offset");
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();
		}

		auto ntHeader = m_PEDisector->m_ParsedOptionalHeader;
		auto rows = ntHeader.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int r = 0; r < rows; r++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, "%x", ntHeader[r].m_Offset);
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(text_color, ntHeader[r].m_Name.c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(text_color, ucharVecToHexString(ntHeader[r].m_Bytes).c_str());
		}
		ImGui::EndTable();
	}
}

void Manager::HandleDataDirectories()
{
	if (ImGui::BeginTable("Data Directories", 3))
	{
		
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Virtual Address");
			ImGui::TableSetupColumn("Size");
			ImGui::TableHeadersRow();

		}
		auto& dataDir = m_PEDisector->m_ParsedDataDirectory_Opt;
		int num_rows = dataDir.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int i = 0; i < num_rows; i++)
		{
			auto& curr = dataDir[i];
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, curr.m_Name.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextColored(text_color, ucharVecToHexString(curr.m_VirtualAddress).c_str());
			ImGui::TableSetColumnIndex(2);
			ImGui::TextColored(text_color, ucharVecToHexString(curr.m_Size).c_str());
		}
		ImGui::EndTable();
	}
}

void Manager::HandleSectionHeaders()
{
	if (ImGui::BeginTable("Section Header", 11))
	{			
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Field Name");
			ImGui::TableSetupColumn("Misc.PhysicalAddress");
			ImGui::TableSetupColumn("Misc.VirtualSize");
			ImGui::TableSetupColumn("VirtualAddress");
			ImGui::TableSetupColumn("SizeOfRawData");
			ImGui::TableSetupColumn("PointerToRawData");
			ImGui::TableSetupColumn("PointerToRelocations");
			ImGui::TableSetupColumn("PointerToLineNumbers");
			ImGui::TableSetupColumn("NumberOfRelocations");
			ImGui::TableSetupColumn("NumberOfLineNumbers");
			ImGui::TableSetupColumn("Characteristics");
			ImGui::TableHeadersRow();
		}

		auto sectionHeader = m_PEDisector->m_ParsedSectionHeaders;
		auto rows = sectionHeader.size();
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int r = 0; r < rows; r++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextColored(text_color, sectionHeader[r].m_Name.c_str());
			for (int i = 0; i < 10; i++)
			{
				ImGui::TableSetColumnIndex(i+1);
				ImGui::TextColored(text_color, ucharVecToHexString(sectionHeader[r].m_SectionData[i].m_Bytes).c_str());
			}
		}
		ImGui::EndTable();
	}



}

void Manager::HandleImports()
{
	ImGui::Columns(2);
	ImGui::BeginChild("PEtableRegion1", ImVec2(m_PEtableWidth/2, m_PEtableHeight), true);
	int x = 0;
	auto& imports = m_PEDisector->m_ParsedImports;
	auto first_lib_ex = imports[0];

	if (ImGui::BeginTable("Functions", 7))
	{
		{
			stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
			ImGui::TableSetupColumn("Library");
			ImGui::TableSetupColumn("Characteristics");
			ImGui::TableSetupColumn("OrginalFirstThunk");
			ImGui::TableSetupColumn("TimeDateStamp");
			ImGui::TableSetupColumn("ForwarderChain");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("FirstThunk");
			ImGui::TableHeadersRow();
		}
		auto text_color = GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR);
		for (int row = 0; row < imports.size(); row++)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (stylewrappers::ColoredSelectable(imports[row].m_Library, text_color))
			{
				m_PEselectedImportView = x;
			}
			x++;
			for (int column = 1; column < 7; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::TextColored(text_color, ucharVecToHexString(imports[row].m_ImportDescriptorData[column-1].m_Bytes).c_str());
			}
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
	
	ImGui::NextColumn();
	ImGui::BeginChild("PEtableRegion2", ImVec2(m_PEtableWidth/2, m_PEtableHeight), true);
	
	if (m_PEselectedImportView < m_PEDisector->m_ParsedImports.size())
	{
		auto& selected = m_PEDisector->m_ParsedImports[m_PEselectedImportView].m_FunctionImports;
		auto num_cols = selected.size() ? selected[0].m_ImportInfo.size() : 0;
		if (ImGui::BeginTable("Functions", 3))
		{
			{
				stylewrappers::TableStyle PE_TABLE_STYLE(GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADER_COLOR), GetColorSetting(VISUALS_INDEX::PEPARSER_COLHEADERTEXT_COLOR));
				ImGui::TableSetupColumn("Function");
				ImGui::TableSetupColumn("Thunk");
				ImGui::TableSetupColumn("Hint");
				ImGui::TableHeadersRow();
			}
			if (num_cols)
			{
				for (int row = 0; row < selected.size(); row++)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), selected[row].m_FunctionName.c_str());
					for (int column = 1; column < 3; column++)
					{
						ImGui::TableSetColumnIndex(column);
						ImGui::TextColored(GetColorSetting(VISUALS_INDEX::PEPARSER_TEXT_COLOR), ucharVecToHexString(selected[row].m_ImportInfo[column - 1].m_Bytes).c_str());
					}
				}
			}	
			ImGui::EndTable();
		}
	}
	
	ImGui::EndChild();
}

int Manager::HandleFileLoad(const std::string& path)
{
	auto ret_code = m_FileBrowser->LoadFile(path);
	if (ret_code != FB_RETCODE::FILE_CHANGE_LOAD)
		return -1;
	else
	{
		if (m_PEDisector != nullptr)
			delete m_PEDisector;
		m_PEDisector = new PEDisector(m_FileBrowser->m_LoadedFileName);

	}
	m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData, m_PEDisector);
	return 0;
}

int Manager::GetStructureId(const std::string& struct_name) const
{
	int i = 0;
	for (auto& st : m_MemoryDumpStructureVec)
	{
		if (st.m_Name == struct_name)
			return i;
		i++;
	}
	return -1;
}

int Manager::AddStructMember(int struct_id, std::string member_name, int size, int display_type)
{
	if (struct_id >= 0 && struct_id < m_MemoryDumpStructureVec.size())
	{
		auto& selected_struct = m_MemoryDumpStructureVec[struct_id];
		MemDumpStructEntry se;
		se.m_GivenName = member_name;
		se.m_Size = size;
		se.m_Display = (MEMDUMPDISPLAY)display_type;
		selected_struct.AddEntry(se);
		return 0;
	}
	else
	{
		return -1;
	}
}

void Manager::SaveStructure(int struct_id) const
{
	auto& structure = m_MemoryDumpStructureVec[struct_id];
	m_DataBaseManager->SaveStructure(structure);
}

void Manager::DeleteStructure(int struct_id)
{
	auto& structure = m_MemoryDumpStructureVec[struct_id];
	m_DataBaseManager->DeleteStructure(structure);
	m_MemoryDumpStructureVec.erase(m_MemoryDumpStructureVec.begin() + struct_id);
}

std::vector<size_t> Manager::RequestByteScan(std::vector<unsigned char>& bytes)
{
	memset(m_ByteScannerPatternBuffer, 0x00, 17);						// clear the buffer before copying
	for (int i = 0; i < bytes.size(); i++)
	{
		memcpy(&m_ByteScannerPatternBuffer[i], &bytes[i], 1);			// We copy the pattern from the script into the pattern buffer to avoid confusion/discrepancies
	}
	return m_ByteScanner->byte_scan_file(m_FileBrowser, bytes, this);	// return the matches
}

int Manager::SaveFile(const std::string& path)
{
	return m_FileBrowser->SaveFile(path);
}

int Manager::ColorChangeRequest(const std::string& component, float r, float g, float b)
{
	std::string COLOR_QUERYSEL_STRING = std::format("'{}'", component);
	ImVec4 NewColor = { r, g, b, 1.00f };

	for (auto& qs : COLORSETTINGS_QUERYSTRING)
	{
		if (COLOR_QUERYSEL_STRING == qs)
		{
			m_DataBaseManager->UpdateColorSetting(qs, NewColor);
			return 0;
		}
	}
	return -1;
}

DecodedInst Manager::GetOpcode(int offset)
{
	if (m_Decoder->m_OffsetToInstIndex.find(offset) == m_Decoder->m_OffsetToInstIndex.end())
		return {0, 0, ""};
	int index = m_Decoder->m_OffsetToInstIndex[offset];
	return m_Decoder->m_DecodedBytes[index];
}

void Manager::InterpreterThread()
{
	pybind11::scoped_interpreter guard{};


	while (true)
	{
		std::unique_lock lock(m_InterpreterWorkItemsMutex);
		m_InterpreterCV.wait(lock, [this] {return m_InterpreterWorkItems.size() != 0; });
		auto script = m_InterpreterWorkItems.back();
		m_PythonInterpreter->exec(script);
		m_InterpreterWorkItems.pop_back();
	}

}



/*
	PYTHON INTERPRETER METHODS
*/


interpreter::interpreter(Manager* mgr)
{
	m_ScriptBuffer = new char[8192]{ 0 };
	m_ScriptBufferSize = 8192;
}

interpreter::~interpreter()
{
	delete m_ScriptBuffer;
}




void interpreter::exec(const std::string& script)
{
	// Interpreter was initialized in global main()

	try
	{
		pybind11::exec(script.c_str());
	}
	catch (const pybind11::error_already_set& e)
	{
		PyErr_Print(); // Print the Python error traceback
		std::cerr << "Caught a Python error: " << e.what() << std::endl;
	}

}
