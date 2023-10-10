#include "manager.h"

static std::string ucharToHexString(unsigned char value) {
	std::stringstream stream;
	stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
	return stream.str();
}



/*
	The Manager class is a state machine that handles most of the interactivity of the UI as well as orchestracts
	classes under the hood that provide the heavy lifting
*/
Manager::Manager() : m_FileBrowser(new FileBrowser()), m_Decoder(new Decoder()), m_ByteScanner(new Scanner()),
	m_HexDumpWidth(475), m_HexDumpHeight(400), m_DecoderWidth(475), m_DecoderHeight(400), m_DecoderNumInstructionsDisplayed(200)
{

}


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
	
	



}
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
	auto& m_CurrentDirectory = m_FileBrowser->m_CurrentDirectory;

	// Begin a scrollable child window with a fixed height of 300 pixels
	ImGui::BeginChild("SelectableList", ImVec2(600, 300), true, ImGuiWindowFlags_HorizontalScrollbar);
	for (const auto& r : m_CurrentDirectory)
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
				m_FileBrowser->LoadNewFile(str);
				m_Decoder->DecodeBytes(m_FileBrowser->m_FileLoadData);
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
	int& offset = m_HexDumpOffsetValue;

	// Create a scrollable region
	
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4 originalButtonBgColor = style.Colors[ImGuiCol_Button];

	int total = 0;
	const int bytesPerRow = 16;
	const int numRows = 16;
	int ret = -1;


	for (int row = 0; row < numRows; ++row) {
		for (int col = 0; col < bytesPerRow; ++col) {
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
	if (ImGui::Button("Change Offset"))
	{
		m_bHexDumpShowOffsetPopup = true;
	}

	if (m_bHexDumpShowOffsetPopup)
	{
		ImGui::OpenPopup("Set Offset");
	}

	// OFFSET POPUP 
	if (ImGui::BeginPopup("Set Offset"))
	{
		ImGui::InputText("Offset Value", m_HexDumpOffsetEditorBuffer, 9);
		ImGui::Text("Current Offset: 0x%x", m_HexDumpOffsetValue);

		if (ImGui::Button("Set Offset"))
		{
			int newHexValue;
			unsigned int parsedValue = 0;
			parsedValue = atoi(m_HexDumpOffsetEditorBuffer);
			m_HexDumpOffsetValue = parsedValue;

			ImGui::CloseCurrentPopup();
			m_bHexDumpShowOffsetPopup = false;
			memset(m_HexDumpOffsetEditorBuffer, 0x00, 8);
		}
		ImGui::EndPopup();
	}

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
}

void Manager::HandleHexdump()
{
	HandleHexdumpPopups();
	HandleHexdumpButtons();
}

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

	auto offset = m_Decoder->m_OffsetToInstIndex[m_DecoderOffset];

	for (int i = offset; i < offset + m_DecoderNumInstructionsDisplayed
		&& i < m_Decoder->m_DecodedBytes.size() ; i++)
	{
		auto& inst = m_Decoder->m_DecodedBytes[i];
		ImGui::TextColored(GetFont(inst), "0x%x: %s", inst.m_Offset, inst.m_DecodedInstruction.c_str());
	}
	ImGui::EndChild();
	ImGui::Text("Offset: 0x%x", m_DecoderOffset);
	ImGui::InputInt("Set Offset:", &m_DecoderOffset);
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
		m_ByteScannerTimeTaken = (end_time - start_time).count();
	}
	ImGui::SameLine();
	if (ImGui::Button("Close"))
	{
		m_bByteShowScannerPopup = false;

	}



	ImGui::ProgressBar(m_ByteScannerProgress, ImVec2(150, 30));
	ImGui::Text("Scanned %d bytes in %d ms.", m_ByteScannerBytesScanned, m_ByteScannerTimeTaken);
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




