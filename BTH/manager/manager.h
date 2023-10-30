#pragma once
#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"
#include <thread>
#include <vector>
#include "../filesystem/filebrowser.h"
#include "../utils/utils.h"
#include "../decoder/byte_decoder.h"
#include "../scanner/scanner.h"
#include <chrono>
#include "..\pe\pe.h"
#include "..\db\db.h"
#include "imgui_stylewrappers.h"
#include "..\memdump\memdump.h"
#include <mutex>
#include <iostream>
#define MAX_SCANNER_DISPLAY 25



static ImVec4 RedFont = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
static ImVec4 GreenFont = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
static ImVec4 BlueFont = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 YellowFont = ImVec4(1.0f, 1.0f, 0.2f, 1.0f);


enum class PEINFO : short
{
	DOS_HEADER,
	RICH_HEADER,
	FILE_HEADER,
	OPTIONAL_HEADER,
	DATA_DIRECTORIES,
	SECTION_HEADERS,
	IMPORTS,
	EXPORTS,
};


enum class SETTINGS_DISPLAY : short
{
	VISUALS,
	PERFORMANCE
};



/*
	The Manager class is a state machine that handles most of the interactivity of the UI as well as orchestrates
	classes under the hood that provide the heavy lifting
*/
class Manager
{
public:
	Manager();
	~Manager();

	// Manager Functions
	void RenderUI();
	void BeginThreadManagerThread();


	// Memory Dump Functions
	void HandleMemoryDump();
	void HandleMemoryDumpButton();
	void HandleMemoryDumpCommandLine();
	void HandleMemoryDumpStructureEditor();



	// DB|SAVED SETTINGS FUNCTIONS
	void HandleSettings();
	void HandleSettingsButton();
	void HandleSettingsPopup();
	void HandleSettingsDisplay();
	void DisplayVisualSettings();
	void DisplayPerformanceSettings();




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
	inline void SetByteScannerProgress(const float& progress) { m_ByteScannerProgress = progress; };
	inline void SetByteScannerFinished(const bool& finished) { m_bByteScannerFinished = finished; };

	// String Scanner Functions
	void HandleStringScanner();
	void HandleStringScannerButton();
	void HandleStringScannerPopup();
	inline void SetStringScannerProgress(const float& progress) { m_StringScannerProgress = progress; };
	inline void SetStringScannerFinished(const bool& finished) { m_bStringScannerFinished = finished; };


	// PE Disector Functions
	void HandlePeDump();
	void HandlePeFileFormatButtons();
	void HandlePeDisplay();
	void HandleDosHeader();
	void HandleRichHeader();
	void HandleFileHeader();
	void HandleOptionalHeader();
	void HandleDataDirectories();
	void HandleSectionHeaders();

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
	bool m_bShowMemoryDumpView;


	// These are all mutex protected V
	std::thread m_ThreadManagerThread;
	std::mutex m_ThreadManagerMutex;
	std::vector<std::unique_ptr<std::thread>> m_ActiveThreads;
	// These are all mutex protected ^
	bool m_bThreadManagerExit;








	/*
		DB|SAVED SETTINGS
	*/
	db_mgr* m_DataBaseManager;
	bool m_bSettingsShowPopup;
	SETTINGS_DISPLAY m_SettingsCurrentDisplay;
	VISUALS_INDEX m_CurrentSelectedVisualsIndex;
	ImVec4 m_VisualSettingsColorSelector;



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
	float m_ByteScannerProgress;
	bool m_bByteScannerFinished;
	std::vector<unsigned char> m_ByteScannerPattern;
	std::vector<unsigned int> m_ByteScannerMatches;



	/*
		STRING SCANNER
	*/
	bool m_bStringScannerShowPopup;
	float m_StringScannerProgress;
	bool m_bStringScannerFinished;
	int m_StringScannerMinStringLength;
	int m_StringScannerMaxStringsDisplayed;

	/*
		PE PARSER
	*/
	int m_PEtableWidth;
	int m_PEtableHeight;
	PEINFO m_PEselected;
	int m_PEselectedImportView;

	



};

