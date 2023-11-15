#pragma once
#include "Dependencies/glew.h"
#include "Dependencies/glfw3.h"
#include <iostream>
#include "UI/ui.h"
#include "manager/manager.h"
#include "utils/utils.h"




bool GLLogCall(const char* func, const char* file, int line)
{
	while (GLenum error = glGetError())
	{
		std::cout << "[ERROR]: " << error << " "\
			<< file << " ~ " << func << " Line: " << line << std::endl;
		return false;
	}
	return true;

}

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR) {};
}

#define GLCall(x) GLClearError();\
	x;\
	ASSERT(GLLogCall(#x, __FILE__, __LINE__));




int RenderRight(Manager* manager)
{
	manager->RenderUI();
	return 1;
};



int add_c(int a, int b)
{
	return a + b;
}

int add(int a, int b) {
	return a + b;
}




std::string format_func(const std::string& format)
{
	std::string ret = std::format("\n{}\n", format);

	return ret;
}

std::string ucharToHexString(unsigned char value) {
	std::stringstream stream;
	stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
	return stream.str();
}



// GLOBAL MANAGER CLASS
/*
	We have to make manager a global so that we can create the python bindings for its methods
*/
Manager manager;


/*
	All of our C++ -> Python bindings are implemented here

	These allow for instrumentation of the Manager class with scripts
*/
PYBIND11_EMBEDDED_MODULE(mgr, m) {
	// `m` is a `py::module_` which is used to bind functions and classes

	/*
		mgr.GetFileLoadSize()

		Returns the current number of bytes loaded
	*/
	m.def("GetFileLoadSize", []() {
		return manager.m_FileBrowser->m_FileLoadData.size();
		});


	/*
		mgr.GetFileSize()

		Returns the size of the entire file
	*/
	m.def("GetFileSize", []() {
		return manager.m_FileBrowser->m_LoadedFileSize;
		});


	/*
		mgr.GetByte(int offset)

		Pass in the offset from the current load position and return the byte at that location as a string
	*/
	m.def("GetByte", [](int offset) {
		if (offset < 0 || offset > manager.m_FileBrowser->m_FileLoadData.size())
			return std::string("");
		return ucharToHexString(manager.m_FileBrowser->m_FileLoadData[offset]);
		});

	/*
		mgr.SetByte(int offset, int val)

		Pass in an offset from the current load location and attempt to set the byte equal to val
	*/
	m.def("SetByte", [](int offset, std::string val) {
		unsigned char byte_val = static_cast<unsigned char>(utils::stringToHex(val));
		if (offset < 0 || offset > manager.m_FileBrowser->m_FileLoadData.size() || byte_val > 0xff || byte_val < 0)
			return -1;

		manager.m_FileBrowser->EditByte(offset, byte_val);
		return 0;
		});



	/*
		mgr.GetFileLoadOffset()

		Returns the current offset in the loaded file as an integer
	*/
	m.def("GetFileLoadOffset", []() {
		if (manager.m_FileBrowser->m_FileLoadData.size() == 0)
			return (size_t)0;
		return manager.GetGlobalOffset();
		});



	/*
		mgr.SetFileLoadOffset(std::string offset)
		- The offset parameter must be passed in as bytes i.e. "ff45"
		- We also restrict the hex string size to 8 characters


		This function will attempt to set the offset of the file to the parameter passed in.


		1. If the offset is invalid we return -1
		2. If offset was not invalid we return 0

		NOTE: you will need to call GetFileLoadOffset to confirm the new offset as
			  this function may not actually move the offset if the requested offset
			  is already in the loaded range

	*/
	m.def("SetFileLoadOffset", [](std::string offset) {

		if (offset.size() > 8)
			return -1;
		manager.SetGlobalOffset(offset);
		return 0;
		});



	/*
		mgr.LoadFile(std::string path)

		This function can load a new file from disk
		Return Values:
		1. If the path is invalid or it is already the currently loaded file we return -1
		2. If a new file is loaded successfully we return 0
	*/
	m.def("LoadFile", [](std::string path) {
		auto current_load_file = manager.m_FileBrowser->m_LoadedFileName;

		if (fs::is_regular_file(path) && path != current_load_file)
		{
			return manager.HandleFileLoad(path);
		}
		else
		{
			return -1;
		}
		});

	/*
		mgr.SaveFile(std::string path)


		This function will attempt to save the currently loaded file and all of its edits to the input
		Return Values:
		1. If the file was unable to be saved, -1 is returned
		2. On a successful save, 0 is returned
	*/
	m.def("SaveFile", [](std::string path) {

		return !manager.SaveFile(path) - 1;
		});



	/*
		mgr.NewStructure(std::string name)

		This function will add a new structure to manager.m_MemoryDumpStructureVec for editing
		NOTE: This function does not save the created structure to the database, please see mgr.SaveStructure()
			  to do that

		This function returns a structure id that you will want to use to add new members;
	*/
	m.def("NewStructure", [](std::string name) {
		return manager.NewStructure(name);
		});



	/*
		mgr.GetStructId(std::string name)

		This function returns an Id that is used for calling other functions that can do something with that function

		If the struct does not exist the return Id is -1
	*/
	m.def("GetStructId", [](std::string name) {
		return manager.GetStructureId(name);
		});


	/*
		mgr.AddStructMember(int struct_id, std::string member_name, int size, int display_type)

		struct_id indexes into the structure in m_MemoryDumpStructureVec
		member_name is what the member variable will be named
		size is the size of the variable
		display type options are:
			0 = INT
			1 = LONG_INT
			2 = UNSIGNED_INT
			3 = UNSIGNED_LONGLONG
			4 = ASCII
			5 = UNICODE
			6 = HEX
			7 = BOOL

		 This function adds a new member to the structure identified by its Id

		 Return 0 on success, -1 on failure

	*/

	m.def("AddStructMember", [](int struct_id, std::string member_name, int size, int display_type) {
		return manager.AddStructMember(struct_id, member_name, size, display_type);
		});


	/*
		mgr.SaveStructure(int struct_id)

		Saves the structure identified by struct_id to the database for use in later sessions
	*/
	m.def("SaveStructure", [](int struct_id) {
		manager.SaveStructure(struct_id);
		return;
		});


	/*
		mgr.DeleteStructure(int struct_id)

		Deletes the structure identified by struct_id and all of its member variables from the database
	*/
	m.def("DeleteStructure", [](int struct_id) {
		manager.DeleteStructure(struct_id);
		return;
	});


	/*
		mgr.StringScan(int min_string_length)

		This function scans for all strings in the currently loaded file

		It returns all strings found with min_string_length, both ASCII and Unicode, in a list
	*/
	m.def("StringScan", [](int min_string_length) {
		auto fb = manager.m_FileBrowser;
		auto scanner = manager.m_ByteScanner;
		scanner->string_scan_file(fb, &manager, min_string_length);
		pybind11::list python_strings;
		for (auto& s : scanner->m_StringMatches.m_StandardStrings)
		{
			python_strings.append(pybind11::str(s.m_StringVal));
		}
		for (auto& ws : scanner->m_StringMatches.m_UnicodeStrings)
		{
			auto buffer_size = ws.m_StringVal.size() * 2 + 1;
			auto buf = new char[buffer_size];
			sprintf_s((char*)buf, buffer_size, "%ws", ws.m_StringVal.data());
			std::string wx((char*)buf);
			python_strings.append(pybind11::str(wx));
			delete buf;
		}

		return python_strings;
		});

	/*
		mgr.ByteScan(const std::vector<std::string>& bytes)

		Call this function in python like so:
		mgr.ByteScan(['4d', '5a','ff', '41'])

		We return a list of addresses as size_t of where the matches of this pattern occurred

	*/
	m.def("ByteScan", [](const std::vector<std::string>& bytes) {
		std::vector<size_t> ret;	// return the addresses of hits
		if (bytes.size() > 16)		// We accept 16 as the max size of the byte pattern
			return ret;

		std::vector<unsigned char> byte_vector;
		for (auto& b : bytes)
		{
			if (b.size() > 2)		// do not accept bytes that arent of the form "ff" or "4d"
				return ret;
			byte_vector.push_back(static_cast<unsigned char>(utils::stringToHex(b)));
		}
		return manager.RequestByteScan(byte_vector);
		});


	/*
		mgr.ChangeColor(std::string component, float r, float g, float b)

		This function takes in a component name string which must be one of the string
		in COLORSETTINGS_QUERYSTRING listed in the db.h file

		This function will then attempt to set this color setting to (r, g, b, 1.00f)

	*/
	m.def("ChangeColor", [](const std::string& component, float r, float g, float b) {
		if (r < 0.0f || r > 1.0f || g < 0.0f || g > 1.0f || b < 0.0f || b > 1.0f)
			return -1;


		std::lock_guard<std::mutex> lock(manager.m_FunctionBindingsMutex);	// We gotta get this or we will be editing non thread safe structures
		return manager.ColorChangeRequest(component, r, g, b);
		});


	/*
		mgr.GetOpcode(int offset)

		This function takes in an offset from the current load position and returns a python list formatted like the following:

		return = [opcode_string, relative_offset_from_load_pos, opcode_size]

	*/
	m.def("GetOpcode", [](int offset) {
		pybind11::list opcode_data;
		DecodedInst decoded_inst;
		decoded_inst = manager.GetOpcode(offset);
		if (decoded_inst.m_DecodedInstruction == "" && decoded_inst.m_Offset == 0 && decoded_inst.m_OpcodeSize == 0)
			return opcode_data;
		opcode_data.append(decoded_inst.m_DecodedInstruction);
		opcode_data.append(decoded_inst.m_Offset);
		opcode_data.append(decoded_inst.m_OpcodeSize);

		return opcode_data;
		});


	/*
		mgr.rvaToRaw(unsigned int rva)

		This method takes in a relative virtual address and will output the raw offset into the file where
		that data is located.

		If there is an error 0 is returned.
		
	*/
	m.def("rvaToRaw", [](DWORD rva) {
		return manager.m_PEDisector->RVA_ToRaw(rva);
	});
	

	/*
		Defines the how we will return the IMAGE_DOS_HEADER structure in python
	*/

	pybind11::class_<IMAGE_DOS_HEADER>(m, "IMAGE_DOS_HEADER")\
		.def(pybind11::init<>())\
		.def_readwrite("e_magic", &IMAGE_DOS_HEADER::e_magic)\
		.def_readwrite("e_cblp", &IMAGE_DOS_HEADER::e_cblp)\
		.def_readwrite("e_cp", &IMAGE_DOS_HEADER::e_cp)\
		.def_readwrite("e_crlc", &IMAGE_DOS_HEADER::e_crlc)\
		.def_readwrite("e_cparhdr", &IMAGE_DOS_HEADER::e_cparhdr)\
		.def_readwrite("e_minalloc", &IMAGE_DOS_HEADER::e_minalloc)\
		.def_readwrite("e_maxalloc", &IMAGE_DOS_HEADER::e_maxalloc)\
		.def_readwrite("e_ss", &IMAGE_DOS_HEADER::e_ss)\
		.def_readwrite("e_sp", &IMAGE_DOS_HEADER::e_sp)\
		.def_readwrite("e_csum", &IMAGE_DOS_HEADER::e_csum)\
		.def_readwrite("e_ip", &IMAGE_DOS_HEADER::e_ip)\
		.def_readwrite("e_cs", &IMAGE_DOS_HEADER::e_cs)\
		.def_readwrite("e_lfarlc", &IMAGE_DOS_HEADER::e_lfarlc)\
		.def_readwrite("e_ovno", &IMAGE_DOS_HEADER::e_ovno)\
		.def_property("e_res", &BIND::SetDosRes1, nullptr)\
		.def_readwrite("e_oemid", &IMAGE_DOS_HEADER::e_oemid)\
		.def_readwrite("e_oeminfo", &IMAGE_DOS_HEADER::e_oeminfo)\
		.def_property("e_res2", &BIND::SetDosRes2, nullptr)\
		.def_readwrite("e_lfanew", &IMAGE_DOS_HEADER::e_lfanew);
		

	m.def("DosHeader", []() {
		return manager.m_PEDisector->m_CopiedDos;
	});

	/*
		Defines the how we will return the IMAGE_FILE_HEADER structure in python
	*/
	pybind11::class_<IMAGE_FILE_HEADER>(m, "IMAGE_FILE_HEADER")\
		.def(pybind11::init<>())\
		.def_readwrite("Machine", &IMAGE_FILE_HEADER::Machine)\
		.def_readwrite("NumberOfSections", &IMAGE_FILE_HEADER::NumberOfSections)\
		.def_readwrite("TimeDateStamp", &IMAGE_FILE_HEADER::TimeDateStamp)\
		.def_readwrite("PointerToSymbolTable", &IMAGE_FILE_HEADER::PointerToSymbolTable)\
		.def_readwrite("NumberOfSymbols", &IMAGE_FILE_HEADER::NumberOfSymbols)\
		.def_readwrite("SizeOfOptionalHeader", &IMAGE_FILE_HEADER::SizeOfOptionalHeader)\
		.def_readwrite("Characteristics", &IMAGE_FILE_HEADER::Characteristics);



	m.def("FileHeader", []() {
		return manager.m_PEDisector->m_CopiedFileHeader;
	});


	/*
		Defines the how we will return the IMAGE_OPTIONAL_HEADER structure in python
	*/
	pybind11::class_<IMAGE_OPTIONAL_HEADER64>(m, "IMAGE_OPTIONAL_HEADER64")\
		.def(pybind11::init<>())\
		.def_readwrite("Magic", &IMAGE_OPTIONAL_HEADER64::Magic)\
		.def_readwrite("MajorLinkerVersion", &IMAGE_OPTIONAL_HEADER64::MajorLinkerVersion)\
		.def_readwrite("MinorLinkerVersion", &IMAGE_OPTIONAL_HEADER64::MinorLinkerVersion)\
		.def_readwrite("SizeOfCode", &IMAGE_OPTIONAL_HEADER64::SizeOfCode)\
		.def_readwrite("SizeOfInitializedData", &IMAGE_OPTIONAL_HEADER64::SizeOfInitializedData)\
		.def_readwrite("SizeOfUninitializedData", &IMAGE_OPTIONAL_HEADER64::SizeOfUninitializedData)\
		.def_readwrite("AddressOfEntryPoint", &IMAGE_OPTIONAL_HEADER64::AddressOfEntryPoint)\
		.def_readwrite("BaseOfCode", &IMAGE_OPTIONAL_HEADER64::BaseOfCode)\
		.def_readwrite("ImageBase", &IMAGE_OPTIONAL_HEADER64::ImageBase)\
		.def_readwrite("SectionAlignment", &IMAGE_OPTIONAL_HEADER64::SectionAlignment)\
		.def_readwrite("FileAlignment", &IMAGE_OPTIONAL_HEADER64::FileAlignment)\
		.def_readwrite("MajorOperatingSystemVersion", &IMAGE_OPTIONAL_HEADER64::MajorOperatingSystemVersion)\
		.def_readwrite("MinorOperatingSystemVersion", &IMAGE_OPTIONAL_HEADER64::MinorOperatingSystemVersion)\
		.def_readwrite("MajorSubsystemVersion", &IMAGE_OPTIONAL_HEADER64::MajorSubsystemVersion)\
		.def_readwrite("MinorSubsystemVersion", &IMAGE_OPTIONAL_HEADER64::MinorSubsystemVersion)\
		.def_readwrite("Win32VersionValue", &IMAGE_OPTIONAL_HEADER64::Win32VersionValue)\
		.def_readwrite("SizeOfImage", &IMAGE_OPTIONAL_HEADER64::SizeOfImage)\
		.def_readwrite("SizeOfHeaders", &IMAGE_OPTIONAL_HEADER64::SizeOfHeaders)\
		.def_readwrite("CheckSum", &IMAGE_OPTIONAL_HEADER64::CheckSum)\
		.def_readwrite("Subsystem", &IMAGE_OPTIONAL_HEADER64::Subsystem)\
		.def_readwrite("DllCharacteristics", &IMAGE_OPTIONAL_HEADER64::DllCharacteristics)\
		.def_readwrite("SizeOfStackReserve", &IMAGE_OPTIONAL_HEADER64::SizeOfStackReserve)\
		.def_readwrite("SizeOfStackCommit", &IMAGE_OPTIONAL_HEADER64::SizeOfStackCommit)\
		.def_readwrite("SizeOfHeapReserve", &IMAGE_OPTIONAL_HEADER64::SizeOfHeapReserve)\
		.def_readwrite("SizeOfHeapCommit", &IMAGE_OPTIONAL_HEADER64::SizeOfHeapCommit)\
		.def_readwrite("LoaderFlags", &IMAGE_OPTIONAL_HEADER64::LoaderFlags)\
		.def_readwrite("NumberOfRvaAndSizes", &IMAGE_OPTIONAL_HEADER64::NumberOfRvaAndSizes)\
		;

	pybind11::class_<IMAGE_OPTIONAL_HEADER32>(m, "IMAGE_OPTIONAL_HEADER32")\
		.def(pybind11::init<>())\
		.def(pybind11::init<>())\
		.def_readwrite("Magic",							&IMAGE_OPTIONAL_HEADER32::Magic)\
		.def_readwrite("MajorLinkerVersion",			&IMAGE_OPTIONAL_HEADER32::MajorLinkerVersion)\
		.def_readwrite("MinorLinkerVersion",			&IMAGE_OPTIONAL_HEADER32::MinorLinkerVersion)\
		.def_readwrite("SizeOfCode",					&IMAGE_OPTIONAL_HEADER32::SizeOfCode)\
		.def_readwrite("SizeOfInitializedData",			&IMAGE_OPTIONAL_HEADER32::SizeOfInitializedData)\
		.def_readwrite("SizeOfUninitializedData",		&IMAGE_OPTIONAL_HEADER32::SizeOfUninitializedData)\
		.def_readwrite("AddressOfEntryPoint",			&IMAGE_OPTIONAL_HEADER32::AddressOfEntryPoint)\
		.def_readwrite("BaseOfCode",					&IMAGE_OPTIONAL_HEADER32::BaseOfCode)\
		.def_readwrite("BaseOfData",					&IMAGE_OPTIONAL_HEADER32::BaseOfData)\
		.def_readwrite("ImageBase",						&IMAGE_OPTIONAL_HEADER32::ImageBase)\
		.def_readwrite("SectionAlignment",				&IMAGE_OPTIONAL_HEADER32::SectionAlignment)\
		.def_readwrite("FileAlignment",					&IMAGE_OPTIONAL_HEADER32::FileAlignment)\
		.def_readwrite("MajorOperatingSystemVersion",	&IMAGE_OPTIONAL_HEADER32::MajorOperatingSystemVersion)\
		.def_readwrite("MinorOperatingSystemVersion",	&IMAGE_OPTIONAL_HEADER32::MinorOperatingSystemVersion)\
		.def_readwrite("MajorSubsystemVersion",			&IMAGE_OPTIONAL_HEADER32::MajorSubsystemVersion)\
		.def_readwrite("MinorSubsystemVersion",			&IMAGE_OPTIONAL_HEADER32::MinorSubsystemVersion)\
		.def_readwrite("Win32VersionValue",				&IMAGE_OPTIONAL_HEADER32::Win32VersionValue)\
		.def_readwrite("SizeOfImage",					&IMAGE_OPTIONAL_HEADER32::SizeOfImage)\
		.def_readwrite("SizeOfHeaders",					&IMAGE_OPTIONAL_HEADER32::SizeOfHeaders)\
		.def_readwrite("CheckSum",						&IMAGE_OPTIONAL_HEADER32::CheckSum)\
		.def_readwrite("Subsystem",						&IMAGE_OPTIONAL_HEADER32::Subsystem)\
		.def_readwrite("DllCharacteristics",			&IMAGE_OPTIONAL_HEADER32::DllCharacteristics)\
		.def_readwrite("SizeOfStackReserve",			&IMAGE_OPTIONAL_HEADER32::SizeOfStackReserve)\
		.def_readwrite("SizeOfStackCommit",				&IMAGE_OPTIONAL_HEADER32::SizeOfStackCommit)\
		.def_readwrite("SizeOfHeapReserve",				&IMAGE_OPTIONAL_HEADER32::SizeOfHeapReserve)\
		.def_readwrite("SizeOfHeapCommit",				&IMAGE_OPTIONAL_HEADER32::SizeOfHeapCommit)\
		.def_readwrite("LoaderFlags",					&IMAGE_OPTIONAL_HEADER32::LoaderFlags)\
		.def_readwrite("NumberOfRvaAndSizes",			&IMAGE_OPTIONAL_HEADER32::NumberOfRvaAndSizes)\
		;
		
	/*
		mgr.OptionalHeader--()

		returns a blank optional header if wrong word size, or if no file is initialized
	*/
	m.def("OptionalHeader64", []() {
		auto pe = manager.m_PEDisector;
		if (pe->m_b64bit)
			return pe->m_CopiedOpt64;
		else
		{
			IMAGE_OPTIONAL_HEADER64 blank{0};
			return blank;
		}
	});

	m.def("OptionalHeader32", []() {
		auto pe = manager.m_PEDisector;
		if (!pe->m_b64bit)
			return pe->m_CopiedOpt32;
		else
		{
			IMAGE_OPTIONAL_HEADER32 blank{0};
			return blank;
		}
	});


	/*
		Defines the how we will return the IMAGE_RICH_HEADER structure in python
	*/
	pybind11::class_<BIND::RichHeaderEntryBinding>(m, "RichHeaderEntry")\
		.def(pybind11::init<>())\
		.def_readwrite("ProductId", &BIND::RichHeaderEntryBinding::ProductId)\
		.def_readwrite("VsVersion", &BIND::RichHeaderEntryBinding::VsVersion)\
		.def_readwrite("UseCount", &BIND::RichHeaderEntryBinding::UseCount);

	m.def("RichHeader", []() {
		pybind11::list ret_list;

		auto pe = manager.m_PEDisector;
		for (auto& entry : pe->m_ParsedRichHeader.m_Entries)
		{
			BIND::RichHeaderEntryBinding rhe;
			rhe.ProductId = entry.m_prodIDMeaning;
			rhe.VsVersion = entry.m_vsVersion;
			rhe.UseCount = entry.m_useCount;
			ret_list.append(rhe);
		}
		return ret_list;
	});


	/*
		Defines the how we will return the DataDirectoryEntry structure in python
	*/
	pybind11::class_<dd_Entry>(m, "DataDirectoryEntry")\
		.def(pybind11::init<>())\
		.def_readwrite("Name", &dd_Entry::m_Name)\
		.def_readwrite("Size", &dd_Entry::m_SizeRaw)\
		.def_readwrite("VirtualAddress", &dd_Entry::m_VirtualAddressRaw);

	m.def("DataDirectories", []() {
		pybind11::list ret_list;

		auto pe = manager.m_PEDisector;
		for (auto& entry : pe->m_ParsedDataDirectory_Opt)
		{
			ret_list.append(entry);
		}
		return ret_list;
	});


	/*
		Define how we will return IMAGE_SECTION_HEADER structures in python
	*/
	pybind11::class_<IMAGE_SECTION_HEADER>(m, "IMAGE_SECTION_HEADER")\
		.def(pybind11::init<>())\
		.def_property("Name", &BIND::RecoverSectionHeaderName, nullptr)\
		.def_readwrite("Misc_PhysicalAddress", &IMAGE_SECTION_HEADER::Misc)\
		.def_readwrite("Misc_VirtualSize", &IMAGE_SECTION_HEADER::Misc)\
		.def_readwrite("VirtualAddress", &IMAGE_SECTION_HEADER::VirtualAddress)\
		.def_readwrite("SizeOfRawData", &IMAGE_SECTION_HEADER::SizeOfRawData)\
		.def_readwrite("PointerToRawData", &IMAGE_SECTION_HEADER::PointerToRawData)\
		.def_readwrite("PointerToRelocations", &IMAGE_SECTION_HEADER::PointerToRelocations)\
		.def_readwrite("PointerToLineNumbers", &IMAGE_SECTION_HEADER::PointerToLinenumbers)\
		.def_readwrite("NumberOfRelocations", &IMAGE_SECTION_HEADER::NumberOfRelocations)\
		.def_readwrite("NumberOfLineNumbers", &IMAGE_SECTION_HEADER::NumberOfLinenumbers)\
		.def_readwrite("Characteristics", &IMAGE_SECTION_HEADER::Characteristics)\
		;


	m.def("SectionHeaders", []() {
		pybind11::list ret_list;

		auto pe = manager.m_PEDisector;
		for (auto& entry : pe->m_CopiedSectionHeaders)
		{
			ret_list.append(entry);
		}
		return ret_list;
	});


	/*
		Define how we will return Import Data in python
	*/
	pybind11::class_<fh_LibraryImport>(m, "ImportEntry")\
		.def(pybind11::init<>())\
		.def_readwrite("Library", &fh_LibraryImport::m_Library)\
		.def_property("FunctionImports", &BIND::SetLibImports, nullptr);

	m.def("ImportData", []() {
		pybind11::list ret_list;

		auto pe = manager.m_PEDisector;
		for (auto& entry : pe->m_ParsedImports)
		{
			ret_list.append(entry);
		}
		return ret_list;
	});

}



int main()
{
	// We only need to initialize the python interpreter once so we can do it in main
	
	GLFWwindow* window = nullptr;
	if (!glfwInit())
	{
		return -1;
	}
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "BTH", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		// Problem: glewInit failed
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return -1;
	}


	UI ui(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, 0, 0, 0);
	ui.Init(window);

	// Modify the button colors
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Button] = ImVec4(0.1f, 0.1f, 0.6f, 0.6f); // Button background color (white)
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.9f, 0.9f, 0.9f, 1.0f); // Button background color when hovered
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Button background color when active
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // Set text color to black for buttons

	// add our management loop into the UI render loop 
	ui.AddComponentLeft("RenderLeft", RenderRight, &manager);



	while (!glfwWindowShouldClose(window))
	{
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		ui.Start();
		ui.DrawUI();
		ui.End();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}




	glfwTerminate();
	return 0;
}