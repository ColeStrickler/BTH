#include "pe.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <fstream>



static void fhInsertEntry(unsigned char* bytes, int size, const std::string& name, std::vector<fh_Entry>& target)
{
	fh_Entry entry;
	entry.m_Name = name;
	for (int i = 0; i < size; i++)
	{
		entry.m_Bytes.push_back(bytes[i]);
	}
	target.push_back(entry);
}


static void toByteVector(unsigned char* bytes, int size, std::vector<unsigned char>& tgt)
{
	for (int i = 0; i < size; i++)
	{
		tgt.push_back(bytes[i]);
	}
}



static bool EqualBytes(unsigned char* a, unsigned char* b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}
	return true;
}


void PrintByteVector(std::vector<unsigned char>& bytes)
{
	std::stringstream ss;
	for (const unsigned char byte : bytes) {
		ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
	}
	std::cout << ss.str()  << std::endl;

}

static void PrintRichHeader(fh_RichHeader& m_ParsedRichHeader)
{
	for (int i = 0; i < m_ParsedRichHeader.m_Entries.size(); i++) {
		printf(" 0x%X 0x%X 0x%X: %d.%d.%d\n",
			m_ParsedRichHeader.m_Entries[i].m_buildID,
			m_ParsedRichHeader.m_Entries[i].m_prodID,
			m_ParsedRichHeader.m_Entries[i].m_useCount,
			m_ParsedRichHeader.m_Entries[i].m_buildID,
			m_ParsedRichHeader.m_Entries[i].m_prodID,
			m_ParsedRichHeader.m_Entries[i].m_useCount);
		std::cout << m_ParsedRichHeader.m_Entries[i].m_prodIDMeaning << std::endl;
		std::cout << m_ParsedRichHeader.m_Entries[i].m_vsVersion << "\n" << std::endl;
	}
}




static DWORD Rva2Offset(DWORD Rva, PIMAGE_SECTION_HEADER pSectionCopy, PIMAGE_NT_HEADERS64 pNtHeaders)
{
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSectionHeader;
	if (Rva == 0)
	{
		return (Rva);
	}
	pSectionHeader = pSectionCopy;
	for (i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		if (Rva >= pSectionHeader->VirtualAddress && Rva < pSectionHeader->VirtualAddress +
			pSectionHeader->Misc.VirtualSize)
		{
			break;
		}
		pSectionHeader++;
	}
	return (Rva - pSectionHeader->VirtualAddress + pSectionHeader->PointerToRawData);
}


static DWORD Rva2Offset32(DWORD Rva, PIMAGE_SECTION_HEADER pSectionCopy, PIMAGE_NT_HEADERS32 pNtHeaders)
{
	size_t i = 0;
	PIMAGE_SECTION_HEADER pSectionHeader;
	if (Rva == 0)
	{
		return (Rva);
	}
	pSectionHeader = pSectionCopy;
	for (i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++)
	{
		if (Rva >= pSectionHeader->VirtualAddress && Rva < pSectionHeader->VirtualAddress +
			pSectionHeader->Misc.VirtualSize)
		{
			break;
		}
		pSectionHeader++;
	}
	return (Rva - pSectionHeader->VirtualAddress + pSectionHeader->PointerToRawData);
}





/*
	If we change the unordered_maps to std::vector and make the entries a custom struct, we could iterate through in order
	during display time in the GUI
*/

PEDisector::PEDisector(const std::string& loadFile)
{
	/*
		We need to open the file again here because in some of the RVA->RawOffset values we get during a parse
		are larger than the amount of bytes we store in our filebrowser class. This was giving me some super annoying
		and hard to debug errors
	*/ 
	std::ifstream file(loadFile, std::ios::binary);

	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	auto fs = static_cast<int>(fileSize);
	utils::NewBuffer filebuf(INITIAL_PE_LOAD);
	auto load = min(INITIAL_PE_LOAD,fs);
	file.read((char*)filebuf.Get(), load);

	auto base = filebuf.Get();
	auto dos = (PIMAGE_DOS_HEADER)filebuf.Get();
	auto nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
	auto fh = &nt->FileHeader;
	m_bValidPEFile = EqualBytes((unsigned char*)& nt->Signature, (unsigned char*)"PE\0\0", sizeof(nt->Signature));
	if (m_bValidPEFile)
	{
		ParseDosHeader(dos);
		ParseRichHeader(dos);
		ParseFileHeader(fh);
		ParseOptionalHeader(dos);
		ParseSectionHeaders(dos);
		ParseImports(dos, file);
		m_bSuccessfulDisection = true;
	}
	file.close();
}







void PEDisector::ParseDosHeader(PIMAGE_DOS_HEADER dos)
{
	fhInsertEntry((unsigned char*)&dos->e_magic,		sizeof(dos->e_magic),			"e_magic", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_cblp,		sizeof(dos->e_cblp),			"e_cblp", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_cp,		sizeof(dos->e_cp),				"e_cp", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_crlc,		sizeof(dos->e_crlc),			"e_rlc", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_cparhdr,	sizeof(dos->e_cparhdr),			"e_cparhdr", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_minalloc,	sizeof(dos->e_minalloc),		"e_minalloc", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_maxalloc,	sizeof(dos->e_maxalloc),		"e_maxalloc", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_ss,		sizeof(dos->e_ss),				"e_ss", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_sp,		sizeof(dos->e_sp),				"e_sp", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_csum,		sizeof(dos->e_csum),			"e_csum", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_ip,		sizeof(dos->e_ip),				"e_ip", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_cs,		sizeof(dos->e_cs),				"e_cs", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_lfarlc,	sizeof(dos->e_lfarlc),			"e_lfarlc", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_ovno,		sizeof(dos->e_ovno),			"e_ovno", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_res,		sizeof(dos->e_res),				"e_res", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_oemid,		sizeof(dos->e_oemid),			"e_oemid", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_oeminfo,	sizeof(dos->e_oeminfo),			"e_oeminfo", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_res2,		sizeof(dos->e_res2),			"e_res2", m_ParsedDosHeader);
	fhInsertEntry((unsigned char*)&dos->e_lfanew,	sizeof(dos->e_lfanew),			"e_lfanew", m_ParsedDosHeader);
}

void PEDisector::ParseFileHeader(PIMAGE_FILE_HEADER fh)
{
	fhInsertEntry((unsigned char*)&fh->Machine,				sizeof(fh->Machine),					"Machine", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->NumberOfSections,		sizeof(fh->NumberOfSections),			"NumberOfSections", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->TimeDateStamp,		sizeof(fh->TimeDateStamp),				"TimeDateStamp", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->PointerToSymbolTable, sizeof(fh->PointerToSymbolTable),		"PointerToSymbolTable", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->NumberOfSymbols,		sizeof(fh->NumberOfSymbols),			"NumberOfSymbols", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->SizeOfOptionalHeader, sizeof(fh->SizeOfOptionalHeader),		"SizeOfOptionalHeader", m_ParsedFileHeader);
	fhInsertEntry((unsigned char*)&fh->Characteristics,		sizeof(fh->Characteristics),			"Characteristics", m_ParsedFileHeader);
}

void PEDisector::ParseOptionalHeader32(PIMAGE_OPTIONAL_HEADER32 opt)
{
	fhInsertEntry((unsigned char*)&opt->Magic,						sizeof(opt->Magic),							"Magic", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorLinkerVersion,			sizeof(opt->MajorLinkerVersion),			"MajorLinkerVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorLinkerVersion,			sizeof(opt->MinorLinkerVersion),			"MinorLinkerVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfCode,					sizeof(opt->SizeOfCode),					"SizeOfCode", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfInitializedData,		sizeof(opt->SizeOfInitializedData),			"SizeOfInitializedData", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfUninitializedData,		sizeof(opt->SizeOfUninitializedData),		"SizeOfUninitializedData", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->AddressOfEntryPoint,			sizeof(opt->AddressOfEntryPoint),			"AddressOfEntryPoint", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->BaseOfCode,					sizeof(opt->BaseOfCode),					"BaseOfCode", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->BaseOfData,					sizeof(opt->BaseOfData),					"BaseOfData", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->ImageBase,					sizeof(opt->ImageBase),						"ImageBase", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SectionAlignment,			sizeof(opt->SectionAlignment),				"SectionAlignment", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->FileAlignment,				sizeof(opt->FileAlignment),					"FileAlignment", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorOperatingSystemVersion, sizeof(opt->MajorOperatingSystemVersion),	"MajorOperatingSystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorOperatingSystemVersion, sizeof(opt->MinorOperatingSystemVersion),	"MinorOperatingSystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorSubsystemVersion,		sizeof(opt->MajorSubsystemVersion),			"MajorSubsystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorSubsystemVersion,		sizeof(opt->MinorSubsystemVersion),			"MinorSubsystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->Win32VersionValue,			sizeof(opt->Win32VersionValue),				"Win32VersionValue", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfImage,					sizeof(opt->SizeOfImage),					"SizeOfImage", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeaders,				sizeof(opt->SizeOfHeaders),					"SizeOfHeaders", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->CheckSum,					sizeof(opt->CheckSum),						"CheckSum", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->Subsystem,					sizeof(opt->Subsystem),						"Subsystem", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->DllCharacteristics,			sizeof(opt->DllCharacteristics),			"DllCharacteristics", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfStackReserve,			sizeof(opt->SizeOfStackReserve),			"SizeOfStackReserve", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfStackCommit,			sizeof(opt->SizeOfStackCommit),				"SizeOfStackCommit", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapReserve,			sizeof(opt->SizeOfHeapReserve),				"SizeOfHeapReserve", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapCommit,			sizeof(opt->SizeOfHeapCommit),				"SizeOfHeapCommit", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->LoaderFlags,					sizeof(opt->LoaderFlags),					"LoaderFlags", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->NumberOfRvaAndSizes,			sizeof(opt->NumberOfRvaAndSizes),			"NumberOfRvaAndSizes", m_ParsedOptionalHeader);
	ParseDataDirectories(opt->DataDirectory, opt->NumberOfRvaAndSizes);
}

// no BaseOfData in IMAGE_OPTIONAL_HEADER64
void PEDisector::ParseOptionalHeader64(PIMAGE_OPTIONAL_HEADER64 opt)
{
	fhInsertEntry((unsigned char*)&opt->Magic, sizeof(opt->Magic), "Magic", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorLinkerVersion, sizeof(opt->MajorLinkerVersion), "MajorLinkerVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorLinkerVersion, sizeof(opt->MinorLinkerVersion), "MinorLinkerVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfCode, sizeof(opt->SizeOfCode), "SizeOfCode", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfInitializedData, sizeof(opt->SizeOfInitializedData), "SizeOfInitializedData", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfUninitializedData, sizeof(opt->SizeOfUninitializedData), "SizeOfUninitializedData", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->AddressOfEntryPoint, sizeof(opt->AddressOfEntryPoint), "AddressOfEntryPoint", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->BaseOfCode, sizeof(opt->BaseOfCode), "BaseOfCode", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->ImageBase, sizeof(opt->ImageBase), "ImageBase", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SectionAlignment, sizeof(opt->SectionAlignment), "SectionAlignment", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->FileAlignment, sizeof(opt->FileAlignment), "FileAlignment", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorOperatingSystemVersion, sizeof(opt->MajorOperatingSystemVersion), "MajorOperatingSystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorOperatingSystemVersion, sizeof(opt->MinorOperatingSystemVersion), "MinorOperatingSystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MajorSubsystemVersion, sizeof(opt->MajorSubsystemVersion), "MajorSubsystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->MinorSubsystemVersion, sizeof(opt->MinorSubsystemVersion), "MinorSubsystemVersion", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->Win32VersionValue, sizeof(opt->Win32VersionValue), "Win32VersionValue", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfImage, sizeof(opt->SizeOfImage), "SizeOfImage", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeaders, sizeof(opt->SizeOfHeaders), "SizeOfHeaders", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->CheckSum, sizeof(opt->CheckSum), "CheckSum", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->Subsystem, sizeof(opt->Subsystem), "Subsystem", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->DllCharacteristics, sizeof(opt->DllCharacteristics), "DllCharacteristics", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfStackReserve, sizeof(opt->SizeOfStackReserve), "SizeOfStackReserve", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfStackCommit, sizeof(opt->SizeOfStackCommit), "SizeOfStackCommit", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapReserve, sizeof(opt->SizeOfHeapReserve), "SizeOfHeapReserve", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapCommit, sizeof(opt->SizeOfHeapCommit), "SizeOfHeapCommit", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->LoaderFlags, sizeof(opt->LoaderFlags), "LoaderFlags", m_ParsedOptionalHeader);
	fhInsertEntry((unsigned char*)&opt->NumberOfRvaAndSizes, sizeof(opt->NumberOfRvaAndSizes), "NumberOfRvaAndSizes", m_ParsedOptionalHeader);
	ParseDataDirectories(opt->DataDirectory, opt->NumberOfRvaAndSizes);
}


void PEDisector::ParseOptionalHeader(PIMAGE_DOS_HEADER dos)
{
	auto& machine = m_ParsedFileHeader[0].m_Bytes;
	auto data = (static_cast<WORD>(machine[1]) << 8) | machine[0];
	if ( data == IMAGE_FILE_MACHINE_I386)
	{
		m_b64bit = false;
		auto nt = (PIMAGE_NT_HEADERS32)((unsigned char*)dos + dos->e_lfanew);
		auto opt = &nt->OptionalHeader;
		ParseOptionalHeader32(opt);
	}
	else if (data == IMAGE_FILE_MACHINE_AMD64 || data == IMAGE_FILE_MACHINE_IA64)
	{
		m_b64bit = true;
		auto nt = (PIMAGE_NT_HEADERS64)((unsigned char*)dos + dos->e_lfanew);
		auto opt = &nt->OptionalHeader;
		ParseOptionalHeader64(opt);
	}
}

void PEDisector::ParseDataDirectories(PIMAGE_DATA_DIRECTORY dd, DWORD num_dd)
{
	for (int i = 0; i < num_dd; i++)
	{
		dd_Entry entry;
		auto& dd_data = dd[i];
		toByteVector((unsigned char*)&dd_data.Size, sizeof(dd_data.Size), entry.m_Size);
		toByteVector((unsigned char*)&dd_data.VirtualAddress, sizeof(dd_data.VirtualAddress), entry.m_VirtualAddress);
		m_ParsedDataDirectory_Opt.push_back(entry);
	}
}







void PEDisector::ParseSectionHeaders(PIMAGE_DOS_HEADER dos)
{
	if (m_b64bit)
		ParseSectionHeaders64(dos);
	else
		ParseSectionHeaders32(dos);
}

void PEDisector::ParseSectionHeaders32(PIMAGE_DOS_HEADER dos)
{
	auto nt = (PIMAGE_NT_HEADERS32)((unsigned char*)dos + dos->e_lfanew);
	auto numSections = nt->FileHeader.NumberOfSections;
	auto sectionAddr = ((uintptr_t)&nt->OptionalHeader + (uintptr_t)nt->FileHeader.SizeOfOptionalHeader);
	for (int i = 0; i < numSections; i++) {
		auto section = (PIMAGE_SECTION_HEADER)sectionAddr;
		fh_Section entry;
		entry.m_Name = std::string((char*)section->Name);
		fhInsertEntry((unsigned char*)&section->Misc.PhysicalAddress, sizeof(section->Misc.PhysicalAddress), "Misc.PhysicalAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Misc.VirtualSize, sizeof(section->Misc.VirtualSize), "Misc.VirtualSize", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->VirtualAddress, sizeof(section->VirtualAddress), "VirtualAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->SizeOfRawData, sizeof(section->SizeOfRawData), "SizeOfRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRawData, sizeof(section->PointerToRawData), "PointerToRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRelocations, sizeof(section->PointerToRelocations), "PointerToRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToLinenumbers, sizeof(section->PointerToLinenumbers), "PointerToLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfRelocations, sizeof(section->NumberOfRelocations), "NumberOfRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfLinenumbers, sizeof(section->NumberOfLinenumbers), "NumberOfLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Characteristics, sizeof(section->Characteristics), "Characteristics", entry.m_SectionData);
		m_ParsedSectionHeaders.push_back(entry);
		/*
			We will add a function here to get hash/entropy of each section,
			will need to update the fh_Section struct to hold this information as well
		*/
		//GetHashes(Base + section->PointerToRawData, section->SizeOfRawData, &newSection.HashInfo);
		sectionAddr = sectionAddr + sizeof(IMAGE_SECTION_HEADER);
	}
}

void PEDisector::ParseSectionHeaders64(PIMAGE_DOS_HEADER dos)
{
	auto nt = (PIMAGE_NT_HEADERS64)((unsigned char*)dos + dos->e_lfanew);
	auto numSections = nt->FileHeader.NumberOfSections;
	auto sectionAddr = ((uintptr_t)&nt->OptionalHeader + (uintptr_t)nt->FileHeader.SizeOfOptionalHeader);
	for (int i = 0; i < numSections; i++) {
		auto section = (PIMAGE_SECTION_HEADER)sectionAddr;
		fh_Section entry;
		entry.m_Name = std::string((char*)section->Name);
		fhInsertEntry((unsigned char*)&section->Misc.PhysicalAddress, sizeof(section->Misc.PhysicalAddress), "Misc.PhysicalAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Misc.VirtualSize, sizeof(section->Misc.VirtualSize), "Misc.VirtualSize", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->VirtualAddress, sizeof(section->VirtualAddress), "VirtualAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->SizeOfRawData, sizeof(section->SizeOfRawData), "SizeOfRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRawData, sizeof(section->PointerToRawData), "PointerToRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRelocations, sizeof(section->PointerToRelocations), "PointerToRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToLinenumbers, sizeof(section->PointerToLinenumbers), "PointerToLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfRelocations, sizeof(section->NumberOfRelocations), "NumberOfRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfLinenumbers, sizeof(section->NumberOfLinenumbers), "NumberOfLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Characteristics, sizeof(section->Characteristics), "Characteristics", entry.m_SectionData);
		m_ParsedSectionHeaders.push_back(entry);
		/*
			We will add a function here to get hash/entropy of each section,
			will need to update the fh_Section struct to hold this information as well
		*/
		//GetHashes(Base + section->PointerToRawData, section->SizeOfRawData, &newSection.HashInfo);
		sectionAddr = sectionAddr + sizeof(IMAGE_SECTION_HEADER);
	}
}



/*
	- Need to create a custom struct for holding import data so we can hold other fields besides just the names
	- Need to add support for ordinal imports
*/
void PEDisector::ParseImports(PIMAGE_DOS_HEADER dos, std::ifstream& file)
{
	if (m_b64bit)
		ParseImports64(dos, file);
	else
		ParseImports32(dos, file);
}

void PEDisector::ParseImports32(PIMAGE_DOS_HEADER dos, std::ifstream& file)
{
	unsigned char* Base = (unsigned char*)dos;
	PIMAGE_IMPORT_DESCRIPTOR iDescriptor = nullptr;
	auto nt = (PIMAGE_NT_HEADERS32)((unsigned char*)dos + dos->e_lfanew);
	auto pSectionHeader = IMAGE_FIRST_SECTION(nt);
	int numSections = nt->FileHeader.NumberOfSections;
	if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		iDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(Base + Rva2Offset32(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSectionHeader, nt));
		while (iDescriptor->Name != NULL)
		{
			std::string library(((char*)(Rva2Offset32(iDescriptor->Name, pSectionHeader, nt) + Base)));
			auto thunkILT = (PIMAGE_THUNK_DATA32)(Base + Rva2Offset32(iDescriptor->OriginalFirstThunk, pSectionHeader, nt));
			fh_LibraryImport libImport;
			libImport.m_Library = library;
			while (thunkILT->u1.AddressOfData != 0)
			{
				fh_FunctionImport funcImport;
				if (!(thunkILT->u1.Ordinal & IMAGE_ORDINAL_FLAG32))
				{
					PIMAGE_IMPORT_BY_NAME nameArray = (PIMAGE_IMPORT_BY_NAME)(Rva2Offset32(thunkILT->u1.AddressOfData, pSectionHeader, nt));
					std::string function((char*)(Base + (DWORD)nameArray->Name));
					funcImport.m_FunctionName = function;
					fhInsertEntry((unsigned char*)&iDescriptor->Characteristics, sizeof(iDescriptor->Characteristics), "Characteristics", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&iDescriptor->OriginalFirstThunk, sizeof(iDescriptor->OriginalFirstThunk), "OriginalFirstThunk", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&iDescriptor->TimeDateStamp, sizeof(iDescriptor->TimeDateStamp), "TimeDateStamp", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&iDescriptor->ForwarderChain, sizeof(iDescriptor->ForwarderChain), "ForwarderChain", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&iDescriptor->Name, sizeof(iDescriptor->Name), "Name", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&iDescriptor->FirstThunk, sizeof(iDescriptor->FirstThunk), "FirstThunk", funcImport.m_ImportDescriptorData);
					libImport.m_FunctionImports.push_back(funcImport); // move this later
				}
				thunkILT++;
			}
			m_ParsedImports.push_back(libImport);
			iDescriptor++;
		}
	}
}

void PEDisector::ParseImports64(PIMAGE_DOS_HEADER dos, std::ifstream& file)
{
	unsigned char* Base = (unsigned char*)dos;
	PIMAGE_IMPORT_DESCRIPTOR iDescriptor = nullptr;
	IMAGE_IMPORT_DESCRIPTOR descriptor;
	auto nt = (PIMAGE_NT_HEADERS)((unsigned char*)dos + dos->e_lfanew);
	auto pSectionHeader = IMAGE_FIRST_SECTION(nt);
	int numSections = nt->FileHeader.NumberOfSections;
	if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		/*
			We need to check to make sure we are not accessing out of bounds offsets, if it is out of bounds, we will use our
			utility function to read the data from the file there
		*/
		auto iDescriptorOffset = (size_t)Rva2Offset(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSectionHeader, nt);
		if (iDescriptorOffset > INITIAL_PE_LOAD)
			utils::readFromFileOffset(file, iDescriptorOffset, sizeof(IMAGE_IMPORT_DESCRIPTOR), &descriptor);
		else
		{
			iDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(Base + iDescriptorOffset);
			descriptor = *iDescriptor;
		}

		while (descriptor.Name != NULL)
		{
			
			//printf("0x%x\n", Rva2Offset(descriptor.Name, pSectionHeader, nt));
			auto libNameOffset = Rva2Offset(descriptor.Name, pSectionHeader, nt);
			std::string library;
			if (libNameOffset > INITIAL_PE_LOAD)
				utils::readStringFromFileOffset(file, libNameOffset, library);
			else
				library = std::string ((char*)(libNameOffset + Base));

		
			/*
		
			*/
			IMAGE_THUNK_DATA thunkData;
			auto thunkOffset = Rva2Offset(descriptor.OriginalFirstThunk, pSectionHeader, nt);


			if (thunkOffset > INITIAL_PE_LOAD)
				utils::readFromFileOffset(file, thunkOffset, sizeof(ULONGLONG), &thunkData);
			else
			{
				auto thunkILT = (PIMAGE_THUNK_DATA)(Base + thunkOffset);
				thunkData = *thunkILT;
			}
				
			fh_LibraryImport libImport;
			libImport.m_Library = library;
			while (thunkData.u1.AddressOfData != 0)
			{
				fh_FunctionImport funcImport;
				if (!(thunkData.u1.Ordinal & IMAGE_ORDINAL_FLAG))
				{

					// Messing up here, we arent reading in the function names correctly
					IMAGE_IMPORT_BY_NAME ibName;
					
					std::string function;

					auto nameArrayOffset = Rva2Offset(thunkData.u1.AddressOfData, pSectionHeader, nt);
					if (nameArrayOffset > INITIAL_PE_LOAD)
						utils::readStringFromFileOffset(file, nameArrayOffset + sizeof(WORD), function);
					else
					{
						PIMAGE_IMPORT_BY_NAME nameArray = (PIMAGE_IMPORT_BY_NAME)(Base + nameArrayOffset);
						function = std::string((char*)(Base + (DWORD)nameArray->Name));
					}
					
					funcImport.m_FunctionName = function;
					fhInsertEntry((unsigned char*)&descriptor.Characteristics, sizeof(descriptor.Characteristics), "Characteristics", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&descriptor.OriginalFirstThunk, sizeof(descriptor.OriginalFirstThunk), "OriginalFirstThunk", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&descriptor.TimeDateStamp, sizeof(descriptor.TimeDateStamp), "TimeDateStamp", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&descriptor.ForwarderChain, sizeof(descriptor.ForwarderChain), "ForwarderChain", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&descriptor.Name, sizeof(descriptor.Name), "Name", funcImport.m_ImportDescriptorData);
					fhInsertEntry((unsigned char*)&descriptor.FirstThunk, sizeof(descriptor.FirstThunk), "FirstThunk", funcImport.m_ImportDescriptorData);
					libImport.m_FunctionImports.push_back(funcImport); // move this later
				}
				
				thunkOffset += sizeof(PIMAGE_THUNK_DATA);
				utils::readFromFileOffset(file, thunkOffset, sizeof(ULONGLONG), &thunkData);
			}
			m_ParsedImports.push_back(libImport);
			
			iDescriptorOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
			if (iDescriptorOffset > INITIAL_PE_LOAD)
				utils::readFromFileOffset(file, iDescriptorOffset, sizeof(IMAGE_IMPORT_DESCRIPTOR), &descriptor);
			else
			{
				iDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(Base + iDescriptorOffset);
				descriptor = *iDescriptor;
			}


		}
	}
}


/*
	I was standing on the shoulders of giants for this method. See the following
	https://github.com/hasherezade/bearparser/tree/3816d7e7f441d97e594fc9f4a12b01bf71953c3c
	https://github.com/0xRick/PE-Parser
*/
void PEDisector::ParseRichHeader(PIMAGE_DOS_HEADER dos)
{
	// replace with utils::NewBuffer()
	utils::NewBuffer rB(dos->e_lfanew);
	auto richBuffer = rB.Get();


	memcpy(richBuffer, dos, dos->e_lfanew);

	int index = 0;

	// Scan until we get tagBegID. It will be before the NT Header
	for (int i = 0; i <= dos->e_lfanew; i++)
	{
		// 'Rich'
		if (richBuffer[i] == 0x52 && richBuffer[i + 1] == 0x69 && richBuffer[i + 2] == 0x63 && richBuffer[i + 3] == 0x68)
		{
			index = i;
			break;
		}
	}
	if (!index)
	{
		return;
	}

	/*
		points to start of padding see fig4 https://www.virusbulletin.com/virusbulletin/2020/01/vb2019-paper-rich-headers-leveraging-mysterious-artifact-pe-format/
	
	*/
	auto start_index = index;
	// Grab the decryption key while we're here
	char key[4];
	memcpy(&key, richBuffer + (index + 4), 4);
	int size = 0; 
	index -= 4;
	
	// Decrypt the rich header
	while (true)
	{
		for (int i = 0; i < 4; i++)
		{
			richBuffer[index + i] = richBuffer[index + i] ^ key[i];
		}
		size += 4;
		// 'Dans'
		if (richBuffer[index] == 0x44 && richBuffer[index + 1] == 0x61 && richBuffer[index+2] == 0x6e && richBuffer[index+3] == 0x53)
		{
			break;
		}
		index -= 4;
	}

	// allocate new buffer with only the rich header
	utils::NewBuffer dr(size);
	auto decrypted_rich = dr.Get();
	memcpy(decrypted_rich, richBuffer + (start_index - size), size);
	

	int num_entries = (size - 16) / 8;
	// allocate enough space so we can index into our entries vector
	m_ParsedRichHeader.m_Entries = std::vector<RichHeaderEntry>(num_entries);
	// parse the entries
	for (int i = 16; i < size; i += 8) {
		RichHeaderEntry entry;
		entry.m_prodID = (uint16_t)((unsigned char)decrypted_rich[ i + 3] << 8) | (unsigned char)decrypted_rich[i + 2];
		entry.m_buildID = (uint16_t)((unsigned char)decrypted_rich[i + 1] << 8) | (unsigned char)decrypted_rich[i];
	    entry.m_useCount = (uint32_t)((unsigned char)decrypted_rich[i + 7] << 24) |(unsigned char)decrypted_rich[i + 6] << 16 | (unsigned char)decrypted_rich[i + 5] << 8 | (unsigned char)decrypted_rich[i + 4];
		entry.m_prodIDMeaning = RichHdr_translateProdId(entry.m_prodID);
		entry.m_vsVersion = RichHdr_ProdIdToVSversion(entry.m_prodID);
		memcpy(entry.m_raw, &decrypted_rich[i], 8);
		m_ParsedRichHeader.m_Entries[(i / 8) - 2] = entry;
	}

}
