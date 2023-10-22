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
	std::string m_Name;
	std::vector<unsigned char> m_Bytes;
};

struct dd_Entry
{
	std::string m_Name;
	std::vector<unsigned char> m_VirtualAddress;
	std::vector<unsigned char> m_Size;
};

struct fh_Section
{
	std::string m_Name;
	std::vector<fh_Entry> m_SectionData;
};


struct fh_FunctionImport
{
	std::string m_FunctionName;
	std::vector<fh_Entry> m_ImportDescriptorData;
};



struct fh_LibraryImport
{
	std::string m_Library;
	std::vector<fh_FunctionImport> m_FunctionImports;
};


struct RichHeaderEntry
{
	unsigned char m_raw[8];
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


class PEDisector
{
public:
	PEDisector(const std::string& loadFile);
	inline bool DisectionSuccessful() const { return m_bSuccessfulDisection; };
	inline bool ValidPE() const { return m_bValidPEFile; };
	

	void ParseDosHeader(PIMAGE_DOS_HEADER dos);
	void ParseFileHeader(PIMAGE_FILE_HEADER fh);
	void ParseOptionalHeader32(PIMAGE_OPTIONAL_HEADER32 opt);
	void ParseOptionalHeader64(PIMAGE_OPTIONAL_HEADER64 opt);
	void ParseOptionalHeader(PIMAGE_DOS_HEADER dos);
	void ParseDataDirectories(PIMAGE_DATA_DIRECTORY dd, DWORD num_dd);
	void ParseSectionHeaders(PIMAGE_DOS_HEADER dos);
	void ParseSectionHeaders32(PIMAGE_DOS_HEADER dos);
	void ParseSectionHeaders64(PIMAGE_DOS_HEADER dos);
	void ParseImports(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseImports32(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseImports64(PIMAGE_DOS_HEADER dos, std::ifstream& file);
	void ParseRichHeader(PIMAGE_DOS_HEADER dos);


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



private:
	
	
};

