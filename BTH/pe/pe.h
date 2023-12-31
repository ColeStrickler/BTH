#pragma once
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <unordered_map>
#include <string>

#include "..\utils\utils.h"
#include ".\sig.h"


#define INITIAL_PE_LOAD 10000

struct fh_Entry
{
	DWORD m_Offset;
	std::string m_Name;
	std::vector<unsigned char> m_Bytes;
};

struct dd_Entry
{
	std::string m_Name;
	std::vector<unsigned char> m_VirtualAddress;
	std::vector<unsigned char> m_Size;
	DWORD m_VirtualAddressRaw;
	DWORD m_SizeRaw;
};

struct fh_Section
{
	std::string m_Name;
	std::vector<fh_Entry> m_SectionData;
};


struct fh_FunctionImport
{
	DWORD m_RawThunk;
	std::string m_FunctionName;
	std::vector<fh_Entry> m_ImportInfo;
};



struct fh_LibraryImport
{
	std::string m_Library;
	std::vector<fh_Entry> m_ImportDescriptorData;
	std::vector<fh_FunctionImport> m_FunctionImports;
};


struct RichHeaderEntry
{
	std::vector<unsigned char> m_Raw;
	WORD m_prodID;
	WORD m_buildID;
	DWORD m_useCount;
	std::string m_prodIDMeaning;
	std::string m_vsVersion;
};

struct fh_RichHeader
{
	char key[4];
	std::vector<RichHeaderEntry> m_Entries;
};


std::string RichHeaderMeaning(RichHeaderEntry& entry);


class PEDisector
{
public:
	PEDisector(const std::string& loadFile);
	inline bool DisectionSuccessful() const { return m_bSuccessfulDisection; };
	inline bool ValidPE() const { return m_bValidPEFile; };
	

	void ParseDosHeader(PIMAGE_DOS_HEADER dos);
	void ParseFileHeader(PIMAGE_FILE_HEADER fh, PIMAGE_DOS_HEADER dos);
	void ParseOptionalHeader32(PIMAGE_OPTIONAL_HEADER32 opt, PIMAGE_DOS_HEADER dos);
	void ParseOptionalHeader64(PIMAGE_OPTIONAL_HEADER64 opt, PIMAGE_DOS_HEADER dos);
	void ParseOptionalHeader(PIMAGE_DOS_HEADER dos);
	void ParseDataDirectories(PIMAGE_DATA_DIRECTORY dd, DWORD num_dd);
	void ParseSectionHeaders(PIMAGE_DOS_HEADER dos);
	void ParseSectionHeaders32(PIMAGE_DOS_HEADER dos);
	void ParseSectionHeaders64(PIMAGE_DOS_HEADER dos);

	DWORD RVA_ToRaw(DWORD rva);
	/*
		We currently are not supporting ordinal imports, we will add this soon
	*/
	void ParseImports(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseImports32(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseImports64(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseRichHeader(PIMAGE_DOS_HEADER dos);


	std::string m_LoadFileName;
	bool m_bValidPEFile;
	bool m_bSuccessfulDisection;
	bool m_b64bit;
	std::vector<fh_Entry> m_ParsedDosHeader;
	fh_RichHeader m_ParsedRichHeader;
	std::vector<fh_Entry> m_ParsedFileHeader;
	std::vector<fh_Entry> m_ParsedOptionalHeader;
	std::vector<dd_Entry> m_ParsedDataDirectory_Opt;
	std::vector<fh_Section> m_ParsedSectionHeaders;
	std::vector<fh_LibraryImport> m_ParsedImports;

	IMAGE_DOS_HEADER m_CopiedDos;
	IMAGE_FILE_HEADER m_CopiedFileHeader;
	IMAGE_OPTIONAL_HEADER32 m_CopiedOpt32;
	IMAGE_OPTIONAL_HEADER64 m_CopiedOpt64;
	std::vector<IMAGE_SECTION_HEADER> m_CopiedSectionHeaders;
private:
		
	
};

