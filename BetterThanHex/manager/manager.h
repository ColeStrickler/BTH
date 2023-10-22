#pragma once
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"
#include <vector>
#include "../filesystem/filebrowser.h"
#include "../utils/utils.h"
#include "../decoder/byte_decoder.h"
#include "../scanner/scanner.h"
#include <chrono>
#include "..\pe\pe.h"

static ImVec4 RedFont = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
static ImVec4 GreenFont = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
static ImVec4 BlueFont = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 YellowFont = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);


enum class PEINFO : short
{
	DOS_HEADER,
	RICH_HEADER,
	FILE_HEADER,
	NT_HEADER,
	DATA_DIRECTORIES,
	SECTION_HEADERS,
	IMPORTS,
	EXPORTS,
};



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

	// Byte Scanner Functions
	void HandleByteScanner();
	void HandleByteScannerPopupButton();
	void HandleByteScannerButton();
	void HandleByteScannerPopup();



	// PE Disector Functions
	void HandlePeDump();
	void HandlePeFileFormatButtons();
	void HandlePeDisplay();
	void HandleDosHeader();

	void HandleNtHeader();

	void HandleImports();



	FileBrowser* m_FileBrowser;
	Decoder* m_Decoder;
	Scanner* m_ByteScanner;
	PEDisector* m_PEDisector;
private:
	/*
		GLOBAL STATE
	*/
	int m_GlobalOffset;
	char m_OffsetEditorBuffer[9];
	int m_MaximumLoadSize;	// the maximum amount of a file we can load at a time




	/*
		FILE BROWSER
	*/
	bool m_bShowFileSelectPopup;

	/*
		DECODER
	*/
	int m_DecoderWidth;
	int m_DecoderHeight;
	int m_DecoderNumInstructionsDisplayed;

	/*
		HEXDUMP 
	*/
	int m_HexDumpWidth;
	int m_HexDumpHeight;
	bool m_bHexDumpShowAscii;
	bool m_bShowHexDumpHexEditPopup;
	int m_HexDumpSelectedIndex;
	char m_HexDumpHexEditorBuffer[3];



	/*
		BYTE SCANNER
	*/
	bool m_bByteShowScannerPopup;
	bool m_bByteScannerShowPatternEditPopup;
	int m_ByteScannerInputPatternSize;
	int m_ByteScannerSelectedIndex;
	char m_ByteScannerPatternEditorBuffer[3];
	unsigned char m_ByteScannerPatternBuffer[17];
	int m_ByteScannerBytesScanned;
	std::chrono::duration<double> m_ByteScannerTimeTaken;
	float m_ByteScannerProgress;
	std::vector<unsigned char> m_ByteScannerPattern;
	std::vector<unsigned int> m_ByteScannerMatches;


	/*
		PE PARSER
	*/
	PEINFO m_PEselected;

	



};

