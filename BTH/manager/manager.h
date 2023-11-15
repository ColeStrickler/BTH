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
#include "..\db\defaultstructs.h"
#include <mutex>
#include <iostream>
#include <condition_variable>


#define MAX_SCANNER_DISPLAY 25
#define WINDOW_WIDTH 1600
#define WINDOW_HEIGHT 1000


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

// We forward declare this class so it can be used within Manager
class interpreter;


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
	void BackgroundColorSettings();


	// Memory Dump Functions
	void InitDefaultStructs();
	void HandleMemoryDump();
	void HandleMemoryDumpButton();
	void HandleMemoryDumpStructureView();
	void HandleMemoryDumpStructureEditor();
	void HandleMemoryDumpNewStructurePopup();
	void HandleMemoryDumpMemberEditPopup();


	// DB|SAVED SETTINGS FUNCTIONS
	void HandleSettings();
	void HandleSettingsButton();
	void HandleSettingsPopup();
	void HandleSettingsDisplay();
	void DisplayVisualSettings();
	void DisplayPerformanceSettings();
	ImVec4 GetColorSetting(VISUALS_INDEX index);



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



	/*
		Python Intepreter Display Functions
	*/



	/*
		Python Function Bindings
	*/
	std::mutex m_FunctionBindingsMutex;
	inline void SetGlobalOffset(const std::string& offset) { memset(m_OffsetEditorBuffer, 0x00, 9); memcpy(m_OffsetEditorBuffer, offset.c_str(), offset.size()); };
	inline size_t GetGlobalOffset() const { return m_GlobalOffset; };
	inline int NewStructure(const std::string& struct_name) { m_MemoryDumpStructureVec.push_back(MemDumpStructure(struct_name)); return m_MemoryDumpStructureVec.size() - 1; };
	int GetStructureId(const std::string& struct_name) const;
	int AddStructMember(int struct_id, std::string member_name, int size, int display_type);
	void SaveStructure(int struct_id) const;
	void DeleteStructure(int struct_id);
	std::vector<size_t> RequestByteScan(std::vector<unsigned char>& bytes);
	int SaveFile(const std::string& path);
	int ColorChangeRequest(const std::string& component, float r, float g, float b);

	

	interpreter* m_PythonInterpreter;
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
		MEMORY DUMP/STRUCTURE OVERLAY
	*/
	std::vector<MemDumpStructure> m_MemoryDumpStructureVec;
	bool m_bMemoryDumpShowAddStructurePopup;
	char m_MemoryDumpNewStructureBuffer[20];
	char m_MemoryDumpMemberEditBuffer[20];
	MEMDUMPDISPLAY m_MemoryDumpMemberEditDisplaySelector;
	int m_MemoryDumpMemberEditSizeSelector;


	int m_MemoryDumpNewStructure_CurrentlySelected;
	std::pair<int, int> m_MemoryDumpNewStructure_MemberSelected;
	bool m_bMemoryDumpShowMemberEditPopup;


	//std::unordered_map<std::string, std::vector<MemDumpDisplayEntry>> m_StructureMap;


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
	bool m_bShowSaveFilePopup;
	char m_FileBrowserNavigationBuffer[_MAX_PATH];
	char m_SaveFilePathBuffer[_MAX_PATH];



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

	
	/*
		PYTHON INTERPRETER
	*/
	std::thread m_InterpreterThread;
	std::vector<std::string> m_InterpreterWorkItems;
	std::mutex m_InterpreterWorkItemsMutex;
	std::condition_variable m_InterpreterCV;
	void InterpreterThread();


};




/*
	BEGIN INTERPRETER CLASS

	We create the Interpreter class implementation here becasuse
	it will reference the Manager class and vice versa, so to avoid
	circular imports we just create our implementation here
*/
#include "../Dependencies/python/include/Python.h"
#include "../Dependencies/python/pybind11/pybind11.h"
#include "../Dependencies/python/pybind11/embed.h"
#include "../Dependencies/python/pybind11/stl.h"

// This 


class interpreter
{
public:
	interpreter(Manager* mgr);
	~interpreter();

	void exec(const std::string& script);

	size_t m_ScriptBufferSize;
	char* m_ScriptBuffer;

private:


};

