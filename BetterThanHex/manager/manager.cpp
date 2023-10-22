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
	The Manager class is a state machine that handles most of the interactivity of the UI as well as orchestracts
	classes under the hood that provide the heavy lifting
*/
Manager::Manager() : m_HexDumpWidth(475), m_HexDumpHeight(400), m_DecoderWidth(475), m_DecoderHeight(400), m_DecoderNumInstructionsDisplayed(200),
					m_MaximumLoadSize(200000)
{
	m_FileBrowser = new FileBrowser(m_MaximumLoadSize);
	m_Decoder = new Decoder();
	m_ByteScanner = new Scanner();
}


/*
	Main Loop
*/

void Manager::RenderUI()
{
	ImGui::Columns(2);

	/*
		COLUMN 1
	*/
	HandleFileNavigator();
	// Hex Dump
	auto sel = DrawHexValuesWindow();
	if (sel != -1)
	{
		m_bShowHexDumpHexEditPopup = true;
		m_HexDumpSelectedIndex = sel;
	}
	HandleHexdump();
	
	HandleByteScanner();
	ImGui::NextColumn();
	/*
		COLUMN 2
	*/
	HandleDecoder();
	ImGui::Columns(1);
	HandlePeDump();
	
	



}





/*
	File Naviator
*/

void Manager::HandleFileNavigatorButtons()
{
	if (ImGui::Button("Select File"))
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
		
		if (ImGui::Button("Done"))
		{
			m_bShowFileSelectPopup = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void Manager::DisplayFileNavigator()
{
	auto& InputPath = m_FileBrowser->m_InputPath;
	ImGui::InputText("Enter path", InputPath, sizeof(InputPath));
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
			}
			else
			{
				m_FileBrowser->SetInputPath(c_str);
				auto fb_retcode = m_FileBrowser->LoadFile(str);
				m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData);

				if (fb_retcode == FB_RETCODE::FILE_CHANGE_LOAD)
				{
					if (m_PEDisector != nullptr)
						delete m_PEDisector;
					m_PEDisector = new PEDisector(m_FileBrowser->m_LoadedFileName);
				}			
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
	ImGui::BeginChild("ScrollingRegion", ImVec2(m_HexDumpWidth, m_HexDumpHeight), true);
	std::vector<unsigned char>& hexValues = m_FileBrowser->m_FileLoadData;
	if (hexValues.size() == 0)
	{
		ImGui::EndChild();
		return -1;
	}
		
	bool& showAscii = m_bHexDumpShowAscii;
	int& offset = m_GlobalOffset;

	// Create a scrollable region
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 originalButtonBgColor = style.Colors[ImGuiCol_Button];

	int total = 0;
	const int bytesPerRow = 16;
	const int numRows = 16;
	int ret = -1;


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

				if (ImGui::Button(value.c_str(), ImVec2(20, 20)))
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
		if (ImGui::Button("Set"))
		{
			if (m_HexDumpSelectedIndex != -1)
			{
				int newHexValue;
				unsigned int parsedValue = 0;
				std::istringstream(m_HexDumpHexEditorBuffer) >> std::hex >> parsedValue;
				m_FileBrowser->m_FileLoadData[m_HexDumpSelectedIndex] = static_cast<unsigned char>(parsedValue);
			}

			ImGui::CloseCurrentPopup();
			m_bShowHexDumpHexEditPopup = false;
			m_HexDumpSelectedIndex = -1;
			memset(m_HexDumpHexEditorBuffer, 0x00, 2);

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
	ImGui::InputText("Offset", m_OffsetEditorBuffer, sizeof(m_OffsetEditorBuffer), ImGuiInputTextFlags_CharsHexadecimal);
	std::string offset(m_OffsetEditorBuffer);
	int new_offset = utils::stringToHex(offset);


	// We check to see if we need to load a different portion of the file when the offset changes
	if (m_GlobalOffset != new_offset)
	{
		m_GlobalOffset = new_offset;
		auto parse_inst = m_FileBrowser->LoadFile(m_FileBrowser->m_LoadedFileName, m_GlobalOffset);
		if (parse_inst == FB_RETCODE::FILE_CHANGE_LOAD || parse_inst == FB_RETCODE::OFFSET_CHANGE_LOAD)
		{
			
			m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData);
		}
	}
	
	ImGui::Text("Loaded File: %s\t File Size: %x\n", m_FileBrowser->m_LoadedFileName.c_str(), m_FileBrowser->m_LoadedFileSize);
	
	// Show error message if offset is bigger than file
	if (m_GlobalOffset > m_FileBrowser->m_LoadedFileSize)
	{
		
		ImGui::TextColored(RedFont, "[ERROR]: Offset of %x is larger than file size.", m_GlobalOffset);
	}

	// ASCII Button
	if (m_bHexDumpShowAscii)
	{
		if (ImGui::Button("Show Hex"))
		{
			m_bHexDumpShowAscii = !m_bHexDumpShowAscii;
		}
	}
	else
	{
		if (ImGui::Button("Show Ascii"))
		{
			m_bHexDumpShowAscii = !m_bHexDumpShowAscii;
		}
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
	ImGui::BeginChild("DisassemblyRegion", ImVec2(m_DecoderWidth, m_DecoderHeight), true);

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
		return RedFont;
	}
	if (inst.m_DecodedInstruction.substr(0, 1) == "j")
	{
		return BlueFont;
	}
	if (inst.m_DecodedInstruction.substr(0, 3) == "sub" || inst.m_DecodedInstruction.substr(0, 3) == "add")
	{
		return YellowFont;
	}
	return GreenFont;
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
	if (ImGui::Button("Scan"))
	{
		m_ByteScannerProgress = 0.0f;

		m_ByteScannerPattern.clear();
		for (int i = 0; i < m_ByteScannerInputPatternSize; i++)
		{
			m_ByteScannerPattern.push_back(m_ByteScannerPatternBuffer[i]);
		}
		m_ByteScannerBytesScanned = m_FileBrowser->m_FileLoadData.size();


		auto start_time = std::chrono::high_resolution_clock::now();
		m_ByteScanner->scan_bytes(m_ByteScannerPattern, m_FileBrowser->m_FileLoadData, &m_ByteScannerProgress);
		//std::thread(&Scanner::scan_bytes, this->m_ByteScanner, m_ByteScannerPattern, m_FileBrowser->m_FileLoadData, &m_ByteScannerProgress);
		auto end_time = std::chrono::high_resolution_clock::now();
		m_ByteScannerTimeTaken = end_time - start_time;
	}
	ImGui::SameLine();
	if (ImGui::Button("Close"))
	{
		m_bByteShowScannerPopup = false;

	}



	ImGui::ProgressBar(m_ByteScannerProgress, ImVec2(150, 30));
	ImGui::Text("Scanned %d bytes in %.2f ms.", m_ByteScannerBytesScanned, m_ByteScannerTimeTaken.count());
	ImGui::Text("Found %d matches.", m_ByteScanner->m_ByteMatches.size());

	for (int i = 0; i < m_ByteScanner->m_ByteMatches.size(); i++)
	{
		auto& inst = m_ByteScanner->m_ByteMatches[i];
		ImGui::TextColored(GreenFont, "0x%x", inst);
	}

}

void Manager::HandleByteScannerButton()
{
	if (ImGui::Button("Byte Scan"))
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
			if (ImGui::Button(value.c_str())) {
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
			ImGui::Text("Current Value: 0x%x", m_ByteScannerPatternBuffer[m_ByteScannerSelectedIndex]);
			if (ImGui::Button("Set"))
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
	if (ImGui::Button("DOS Header"))
		m_PEselected = PEINFO::DOS_HEADER;
	ImGui::SameLine();
	if (ImGui::Button("Rich Header"))
		m_PEselected = PEINFO::RICH_HEADER;
	ImGui::SameLine();
	if (ImGui::Button("File Header"))
		m_PEselected = PEINFO::FILE_HEADER;
	ImGui::SameLine();
	if (ImGui::Button("NT Header"))
		m_PEselected = PEINFO::NT_HEADER;
	ImGui::SameLine();
	if (ImGui::Button("Data Directories"))
		m_PEselected = PEINFO::DATA_DIRECTORIES;
	ImGui::SameLine();
	if (ImGui::Button("Section Headers"))
		m_PEselected = PEINFO::SECTION_HEADERS;
	ImGui::SameLine();
	if (ImGui::Button("Imports"))
		m_PEselected = PEINFO::IMPORTS;
	ImGui::SameLine();
	if (ImGui::Button("Exports"))
		m_PEselected = PEINFO::EXPORTS;
}

void Manager::HandlePeDisplay()
{
	switch (m_PEselected)
	{
		case PEINFO::DOS_HEADER:
			HandleDosHeader();
		case PEINFO::RICH_HEADER:
			return;
		case PEINFO::FILE_HEADER:	
			return;
		case PEINFO::NT_HEADER:
			HandleNtHeader();
		case PEINFO::DATA_DIRECTORIES:
			return;
		case PEINFO::SECTION_HEADERS:
			return;
		case PEINFO::IMPORTS:
			HandleImports();
		case PEINFO::EXPORTS:
			return;
		default:
			return;
	}
}

void Manager::HandleDosHeader()
{
	for (auto& e : m_PEDisector->m_ParsedDosHeader)
	{
		ImGui::Text(e.m_Name.c_str());
		ImGui::SameLine();
		ImGui::Text(ucharVecToHexString(e.m_Bytes).c_str());
	}
}

void Manager::HandleNtHeader()
{
	for (auto& e : m_PEDisector->m_ParsedOptionalHeader)
	{
		ImGui::Text(e.m_Name.c_str());
		ImGui::SameLine();
		ImGui::Text(ucharVecToHexString(e.m_Bytes).c_str());
	}
}

void Manager::HandleImports()
{
	int i = 0;
	ImGui::Columns(2);
	for (auto& e : m_PEDisector->m_ParsedImports)
	{
		if (ImGui::Selectable(e.m_Library.c_str()))
		{
			m_PEselectedImportView = i;
		}
		i++;
	}
	ImGui::NextColumn();
	if (m_PEselectedImportView < m_PEDisector->m_ParsedImports.size())
	{
		
		for (auto& i : m_PEDisector->m_ParsedImports[m_PEselectedImportView].m_FunctionImports)
		{
			ImGui::Text(i.m_FunctionName.c_str());
			//ImGui::Spacing();
			//ImGui::SameLine();
			// Will need to add descriptor data here i.m_DescriptorData
		}
	}

}

