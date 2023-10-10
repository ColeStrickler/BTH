#pragma once
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"
#include <vector>
#include "../filesystem/filebrowser.h"
#include "../utils/utils.h"
#include "../decoder/byte_decoder.h"


static ImVec4 RedFont = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
static ImVec4 GreenFont = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
static ImVec4 BlueFont = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 YellowFont = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);


class Manager
{
public:
	Manager();
	~Manager();


	void RenderUI();


	// File Browser Functions
	void HandleFileNavigatorButtons();
	void HandleFileNavigatorPopups();
	void DisplayFileNavigator();
	void HandleFileNavigator();


	// Hex Dump Functions
	int DrawHexValuesWindow();
	void HandleHexdumpPopups();
	void HandleHexdumpButtons();
	void HandleHexdump();


	// Decoder Functions
	void HandleDecoderButtons();
	void HandleDecoderPopups();
	void HandleDecoder();
	ImVec4 GetFont(DecodedInst& inst);



	FileBrowser* m_FileBrowser;
	Decoder* m_Decoder;
private:
	

	// FILE SELECT 
	bool m_bShowFileSelectPopup;



	/*
		DECODER
	*/
	int m_DecoderWidth;
	int m_DecoderHeight;
	int m_DecoderOffset;
	int m_DecoderNumInstructionsDisplayed;

	/*
		HEXDUMP 
	*/
	int m_HexDumpWidth;
	int m_HexDumpHeight;
	bool m_bHexDumpShowAscii;
	bool m_bShowHexDumpHexEditPopup;
	int m_HexDumpSelectedIndex;
	int m_HexDumpOffsetValue;
	bool m_bHexDumpShowOffsetPopup;
	char m_HexDumpHexEditorBuffer[3];
	char m_HexDumpOffsetEditorBuffer[9];

	



};

