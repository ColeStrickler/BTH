#include "pe.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <fstream>



static void fhInsertEntry(unsigned char* bytes, int size, const std::string& name, std::vector<fh_Entry>& target, DWORD offset = 0)
{
	fh_Entry entry;
	entry.m_Name = name;
	entry.m_Offset = offset;
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


std::string RichHeaderMeaning(RichHeaderEntry& entry)
{
	char tmp_buf[256];
	sprintf_s(tmp_buf, "%d.%d.%d", entry.m_buildID, entry.m_prodID, entry.m_useCount);
	auto ret = std::string(tmp_buf);
	return ret;
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

PEDisector::PEDisector(const std::string& loadFile) : m_LoadFileName(loadFile)
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
	if (dos->e_magic != 0x5A4D)
		return;
	auto nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);
	auto fh = &nt->FileHeader;
	m_bValidPEFile = EqualBytes((unsigned char*)& nt->Signature, (unsigned char*)"PE\0\0", sizeof(nt->Signature));
	if (m_bValidPEFile)
	{
		ParseDosHeader(dos);
		ParseRichHeader(dos);
		ParseFileHeader(fh, dos);
		ParseOptionalHeader(dos);
		ParseSectionHeaders(dos);
		ParseImports(dos, file);
		m_bSuccessfulDisection = true;
	}
	file.close();
}







void PEDisector::ParseDosHeader(PIMAGE_DOS_HEADER dos)
{
	fhInsertEntry((unsigned char*)&dos->e_magic,		sizeof(dos->e_magic),			"e_magic", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_magic));
	fhInsertEntry((unsigned char*)&dos->e_cblp,		sizeof(dos->e_cblp),				"e_cblp", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_cblp));
	fhInsertEntry((unsigned char*)&dos->e_cp,		sizeof(dos->e_cp),					"e_cp", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_cp));
	fhInsertEntry((unsigned char*)&dos->e_crlc,		sizeof(dos->e_crlc),				"e_crlc", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_crlc));
	fhInsertEntry((unsigned char*)&dos->e_cparhdr,	sizeof(dos->e_cparhdr),				"e_cparhdr", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_cparhdr));
	fhInsertEntry((unsigned char*)&dos->e_minalloc,	sizeof(dos->e_minalloc),			"e_minalloc", m_ParsedDosHeader,	offsetof(IMAGE_DOS_HEADER,e_minalloc));
	fhInsertEntry((unsigned char*)&dos->e_maxalloc,	sizeof(dos->e_maxalloc),			"e_maxalloc", m_ParsedDosHeader,	offsetof(IMAGE_DOS_HEADER,e_maxalloc));
	fhInsertEntry((unsigned char*)&dos->e_ss,		sizeof(dos->e_ss),					"e_ss", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_ss));
	fhInsertEntry((unsigned char*)&dos->e_sp,		sizeof(dos->e_sp),					"e_sp", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_sp));
	fhInsertEntry((unsigned char*)&dos->e_csum,		sizeof(dos->e_csum),				"e_csum", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_csum));
	fhInsertEntry((unsigned char*)&dos->e_ip,		sizeof(dos->e_ip),					"e_ip", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_ip));
	fhInsertEntry((unsigned char*)&dos->e_cs,		sizeof(dos->e_cs),					"e_cs", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_cs));
	fhInsertEntry((unsigned char*)&dos->e_lfarlc,	sizeof(dos->e_lfarlc),				"e_lfarlc", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_lfarlc));
	fhInsertEntry((unsigned char*)&dos->e_ovno,		sizeof(dos->e_ovno),				"e_ovno", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_ovno ));
	fhInsertEntry((unsigned char*)&dos->e_res,		sizeof(dos->e_res),					"e_res", m_ParsedDosHeader,			offsetof(IMAGE_DOS_HEADER,e_res ));
	fhInsertEntry((unsigned char*)&dos->e_oemid,		sizeof(dos->e_oemid),			"e_oemid", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_oemid));
	fhInsertEntry((unsigned char*)&dos->e_oeminfo,	sizeof(dos->e_oeminfo),				"e_oeminfo", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_oeminfo));
	fhInsertEntry((unsigned char*)&dos->e_res2,		sizeof(dos->e_res2),				"e_res2", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_res2));
	fhInsertEntry((unsigned char*)&dos->e_lfanew,	sizeof(dos->e_lfanew),				"e_lfanew", m_ParsedDosHeader,		offsetof(IMAGE_DOS_HEADER,e_lfanew));
	memcpy(&m_CopiedDos, dos, sizeof(IMAGE_DOS_HEADER));
}

void PEDisector::ParseFileHeader(PIMAGE_FILE_HEADER fh, PIMAGE_DOS_HEADER dos)
{
	fhInsertEntry((unsigned char*)&fh->Machine,				sizeof(fh->Machine),					"Machine", m_ParsedFileHeader,			dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, Machine));
	fhInsertEntry((unsigned char*)&fh->NumberOfSections, sizeof(fh->NumberOfSections), "NumberOfSections", m_ParsedFileHeader,				dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, NumberOfSections));
	fhInsertEntry((unsigned char*)&fh->TimeDateStamp, sizeof(fh->TimeDateStamp), "TimeDateStamp", m_ParsedFileHeader,						dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, TimeDateStamp));
	fhInsertEntry((unsigned char*)&fh->PointerToSymbolTable, sizeof(fh->PointerToSymbolTable), "PointerToSymbolTable", m_ParsedFileHeader,	dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, PointerToSymbolTable));
	fhInsertEntry((unsigned char*)&fh->NumberOfSymbols, sizeof(fh->NumberOfSymbols), "NumberOfSymbols", m_ParsedFileHeader,					dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, NumberOfSymbols));
	fhInsertEntry((unsigned char*)&fh->SizeOfOptionalHeader, sizeof(fh->SizeOfOptionalHeader), "SizeOfOptionalHeader", m_ParsedFileHeader,	dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, SizeOfOptionalHeader));
	fhInsertEntry((unsigned char*)&fh->Characteristics, sizeof(fh->Characteristics), "Characteristics", m_ParsedFileHeader,					dos->e_lfanew + sizeof(DWORD) + offsetof(IMAGE_FILE_HEADER, Characteristics));
	memcpy(&m_CopiedFileHeader, fh, sizeof(IMAGE_FILE_HEADER));
}

void PEDisector::ParseOptionalHeader32(PIMAGE_OPTIONAL_HEADER32 opt, PIMAGE_DOS_HEADER dos)
{
	fhInsertEntry((unsigned char*)&opt->Magic,						sizeof(opt->Magic),							"Magic", m_ParsedOptionalHeader,						dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, Magic));
	fhInsertEntry((unsigned char*)&opt->MajorLinkerVersion,			sizeof(opt->MajorLinkerVersion),			"MajorLinkerVersion", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MajorLinkerVersion));
	fhInsertEntry((unsigned char*)&opt->MinorLinkerVersion,			sizeof(opt->MinorLinkerVersion),			"MinorLinkerVersion", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MinorLinkerVersion));
	fhInsertEntry((unsigned char*)&opt->SizeOfCode,					sizeof(opt->SizeOfCode),					"SizeOfCode", m_ParsedOptionalHeader,					dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfCode));
	fhInsertEntry((unsigned char*)&opt->SizeOfInitializedData,		sizeof(opt->SizeOfInitializedData),			"SizeOfInitializedData", m_ParsedOptionalHeader	,		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfInitializedData));
	fhInsertEntry((unsigned char*)&opt->SizeOfUninitializedData,	sizeof(opt->SizeOfUninitializedData),			"SizeOfUninitializedData", m_ParsedOptionalHeader,	dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfUninitializedData));
	fhInsertEntry((unsigned char*)&opt->AddressOfEntryPoint,		sizeof(opt->AddressOfEntryPoint),				"AddressOfEntryPoint", m_ParsedOptionalHeader,		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, AddressOfEntryPoint));
	fhInsertEntry((unsigned char*)&opt->BaseOfCode,					sizeof(opt->BaseOfCode),					"BaseOfCode", m_ParsedOptionalHeader,					dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, BaseOfCode));
	fhInsertEntry((unsigned char*)&opt->BaseOfData,					sizeof(opt->BaseOfData),					"BaseOfData", m_ParsedOptionalHeader,					dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, BaseOfData));
	fhInsertEntry((unsigned char*)&opt->ImageBase,					sizeof(opt->ImageBase),						"ImageBase", m_ParsedOptionalHeader	,					dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, ImageBase));
	fhInsertEntry((unsigned char*)&opt->SectionAlignment,			sizeof(opt->SectionAlignment),				"SectionAlignment", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SectionAlignment));
	fhInsertEntry((unsigned char*)&opt->FileAlignment,				sizeof(opt->FileAlignment),					"FileAlignment", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, FileAlignment));
	fhInsertEntry((unsigned char*)&opt->MajorOperatingSystemVersion,sizeof(opt->MajorOperatingSystemVersion),	"MajorOperatingSystemVersion", m_ParsedOptionalHeader,	dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MajorOperatingSystemVersion));
	fhInsertEntry((unsigned char*)&opt->MinorOperatingSystemVersion,sizeof(opt->MinorOperatingSystemVersion),	"MinorOperatingSystemVersion", m_ParsedOptionalHeader,	dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MinorOperatingSystemVersion));
	fhInsertEntry((unsigned char*)&opt->MajorSubsystemVersion,		sizeof(opt->MajorSubsystemVersion),			"MajorSubsystemVersion", m_ParsedOptionalHeader	,		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MajorSubsystemVersion));
	fhInsertEntry((unsigned char*)&opt->MinorSubsystemVersion,		sizeof(opt->MinorSubsystemVersion),			"MinorSubsystemVersion", m_ParsedOptionalHeader,		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, MinorSubsystemVersion));
	fhInsertEntry((unsigned char*)&opt->Win32VersionValue,			sizeof(opt->Win32VersionValue),				"Win32VersionValue", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, Win32VersionValue));
	fhInsertEntry((unsigned char*)&opt->SizeOfImage,				sizeof(opt->SizeOfImage),						"SizeOfImage", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfImage));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeaders,				sizeof(opt->SizeOfHeaders),					"SizeOfHeaders", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfHeaders));
	fhInsertEntry((unsigned char*)&opt->CheckSum,					sizeof(opt->CheckSum),						"CheckSum", m_ParsedOptionalHeader,						dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, CheckSum));
	fhInsertEntry((unsigned char*)&opt->Subsystem,					sizeof(opt->Subsystem),						"Subsystem", m_ParsedOptionalHeader,					dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, Subsystem));
	fhInsertEntry((unsigned char*)&opt->DllCharacteristics,			sizeof(opt->DllCharacteristics),			"DllCharacteristics", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, DllCharacteristics));
	fhInsertEntry((unsigned char*)&opt->SizeOfStackReserve,			sizeof(opt->SizeOfStackReserve),			"SizeOfStackReserve", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfStackReserve));
	fhInsertEntry((unsigned char*)&opt->SizeOfStackCommit,			sizeof(opt->SizeOfStackCommit),				"SizeOfStackCommit", m_ParsedOptionalHeader	,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfStackCommit));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapReserve,			sizeof(opt->SizeOfHeapReserve),				"SizeOfHeapReserve", m_ParsedOptionalHeader,			dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfHeapReserve));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapCommit,			sizeof(opt->SizeOfHeapCommit),				"SizeOfHeapCommit", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfHeapCommit));
	fhInsertEntry((unsigned char*)&opt->LoaderFlags,				sizeof(opt->LoaderFlags),						"LoaderFlags", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, LoaderFlags));
	fhInsertEntry((unsigned char*)&opt->NumberOfRvaAndSizes,		sizeof(opt->NumberOfRvaAndSizes),				"NumberOfRvaAndSizes", m_ParsedOptionalHeader,		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS32,OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER32, NumberOfRvaAndSizes));
	memcpy(&m_CopiedOpt32, opt, sizeof(IMAGE_OPTIONAL_HEADER32));
	ParseDataDirectories(opt->DataDirectory, opt->NumberOfRvaAndSizes);
}

// no BaseOfData in IMAGE_OPTIONAL_HEADER64
void PEDisector::ParseOptionalHeader64(PIMAGE_OPTIONAL_HEADER64 opt, PIMAGE_DOS_HEADER dos)
{
	fhInsertEntry((unsigned char*)&opt->Magic, sizeof(opt->Magic), "Magic", m_ParsedOptionalHeader,																		dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, Magic));
	fhInsertEntry((unsigned char*)&opt->MajorLinkerVersion, sizeof(opt->MajorLinkerVersion), "MajorLinkerVersion", m_ParsedOptionalHeader,								dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MajorLinkerVersion));
	fhInsertEntry((unsigned char*)&opt->MinorLinkerVersion, sizeof(opt->MinorLinkerVersion), "MinorLinkerVersion", m_ParsedOptionalHeader,								dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MinorLinkerVersion));
	fhInsertEntry((unsigned char*)&opt->SizeOfCode, sizeof(opt->SizeOfCode), "SizeOfCode", m_ParsedOptionalHeader,														dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfCode));
	fhInsertEntry((unsigned char*)&opt->SizeOfInitializedData, sizeof(opt->SizeOfInitializedData), "SizeOfInitializedData", m_ParsedOptionalHeader,						dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfInitializedData));
	fhInsertEntry((unsigned char*)&opt->SizeOfUninitializedData, sizeof(opt->SizeOfUninitializedData), "SizeOfUninitializedData", m_ParsedOptionalHeader,				dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfUninitializedData));
	fhInsertEntry((unsigned char*)&opt->AddressOfEntryPoint, sizeof(opt->AddressOfEntryPoint), "AddressOfEntryPoint", m_ParsedOptionalHeader,							dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, AddressOfEntryPoint));
	fhInsertEntry((unsigned char*)&opt->BaseOfCode, sizeof(opt->BaseOfCode), "BaseOfCode", m_ParsedOptionalHeader,														dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, BaseOfCode));
	fhInsertEntry((unsigned char*)&opt->ImageBase, sizeof(opt->ImageBase), "ImageBase", m_ParsedOptionalHeader,															dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, ImageBase));
	fhInsertEntry((unsigned char*)&opt->SectionAlignment, sizeof(opt->SectionAlignment), "SectionAlignment", m_ParsedOptionalHeader,									dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SectionAlignment));
	fhInsertEntry((unsigned char*)&opt->FileAlignment, sizeof(opt->FileAlignment), "FileAlignment", m_ParsedOptionalHeader,												dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, FileAlignment));
	fhInsertEntry((unsigned char*)&opt->MajorOperatingSystemVersion, sizeof(opt->MajorOperatingSystemVersion), "MajorOperatingSystemVersion", m_ParsedOptionalHeader,	dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MajorOperatingSystemVersion));
	fhInsertEntry((unsigned char*)&opt->MinorOperatingSystemVersion, sizeof(opt->MinorOperatingSystemVersion), "MinorOperatingSystemVersion", m_ParsedOptionalHeader,	dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MinorOperatingSystemVersion));
	fhInsertEntry((unsigned char*)&opt->MajorSubsystemVersion, sizeof(opt->MajorSubsystemVersion), "MajorSubsystemVersion", m_ParsedOptionalHeader,						dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MajorSubsystemVersion));
	fhInsertEntry((unsigned char*)&opt->MinorSubsystemVersion, sizeof(opt->MinorSubsystemVersion), "MinorSubsystemVersion", m_ParsedOptionalHeader,						dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, MinorSubsystemVersion));
	fhInsertEntry((unsigned char*)&opt->Win32VersionValue, sizeof(opt->Win32VersionValue), "Win32VersionValue", m_ParsedOptionalHeader,									dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, Win32VersionValue));
	fhInsertEntry((unsigned char*)&opt->SizeOfImage, sizeof(opt->SizeOfImage), "SizeOfImage", m_ParsedOptionalHeader,													dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfImage));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeaders, sizeof(opt->SizeOfHeaders), "SizeOfHeaders", m_ParsedOptionalHeader,												dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfHeaders));
	fhInsertEntry((unsigned char*)&opt->CheckSum, sizeof(opt->CheckSum), "CheckSum", m_ParsedOptionalHeader,															dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, CheckSum));
	fhInsertEntry((unsigned char*)&opt->Subsystem, sizeof(opt->Subsystem), "Subsystem", m_ParsedOptionalHeader,															dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, Subsystem));
	fhInsertEntry((unsigned char*)&opt->DllCharacteristics, sizeof(opt->DllCharacteristics), "DllCharacteristics", m_ParsedOptionalHeader,								dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, DllCharacteristics));
	fhInsertEntry((unsigned char*)&opt->SizeOfStackReserve, sizeof(opt->SizeOfStackReserve), "SizeOfStackReserve", m_ParsedOptionalHeader,								dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfStackReserve));
	fhInsertEntry((unsigned char*)&opt->SizeOfStackCommit, sizeof(opt->SizeOfStackCommit), "SizeOfStackCommit", m_ParsedOptionalHeader,									dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfStackCommit));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapReserve, sizeof(opt->SizeOfHeapReserve), "SizeOfHeapReserve", m_ParsedOptionalHeader,									dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfHeapReserve));
	fhInsertEntry((unsigned char*)&opt->SizeOfHeapCommit, sizeof(opt->SizeOfHeapCommit), "SizeOfHeapCommit", m_ParsedOptionalHeader,									dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, SizeOfHeapCommit));
	fhInsertEntry((unsigned char*)&opt->LoaderFlags, sizeof(opt->LoaderFlags), "LoaderFlags", m_ParsedOptionalHeader,													dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, LoaderFlags));
	fhInsertEntry((unsigned char*)&opt->NumberOfRvaAndSizes, sizeof(opt->NumberOfRvaAndSizes), "NumberOfRvaAndSizes", m_ParsedOptionalHeader,							dos->e_lfanew + offsetof(IMAGE_NT_HEADERS, OptionalHeader) + offsetof(IMAGE_OPTIONAL_HEADER, NumberOfRvaAndSizes));
	memcpy(&m_CopiedOpt64, opt, sizeof(IMAGE_OPTIONAL_HEADER64));
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
		ParseOptionalHeader32(opt, dos);
	}
	else if (data == IMAGE_FILE_MACHINE_AMD64 || data == IMAGE_FILE_MACHINE_IA64)
	{
		m_b64bit = true;
		auto nt = (PIMAGE_NT_HEADERS64)((unsigned char*)dos + dos->e_lfanew);
		auto opt = &nt->OptionalHeader;
		ParseOptionalHeader64(opt, dos);
	}
}

void PEDisector::ParseDataDirectories(PIMAGE_DATA_DIRECTORY dd, DWORD num_dd)
{
	for (int i = 0; i < num_dd; i++)
	{
		dd_Entry entry;
		auto& dd_data = dd[i];
		entry.m_Name = DataDirIndexToString(i);
		toByteVector((unsigned char*)&dd_data.Size, sizeof(dd_data.Size), entry.m_Size);
		toByteVector((unsigned char*)&dd_data.VirtualAddress, sizeof(dd_data.VirtualAddress), entry.m_VirtualAddress);
		entry.m_VirtualAddressRaw = dd_data.VirtualAddress;
		entry.m_SizeRaw = dd_data.Size;
		m_ParsedDataDirectory_Opt.push_back(entry);
	}
}





void PEDisector::ParseSectionHeaders(PIMAGE_DOS_HEADER dos)
{
	m_CopiedSectionHeaders.clear();
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
		m_CopiedSectionHeaders.push_back(*section);
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
		m_CopiedSectionHeaders.push_back(*section);
		fh_Section entry;
		entry.m_Name = std::string((char*)section->Name);
		fhInsertEntry((unsigned char*)&section->Misc.PhysicalAddress, sizeof(section->Misc.PhysicalAddress),	"Misc.PhysicalAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Misc.VirtualSize, sizeof(section->Misc.VirtualSize),			"Misc.VirtualSize", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->VirtualAddress, sizeof(section->VirtualAddress),				"VirtualAddress", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->SizeOfRawData, sizeof(section->SizeOfRawData),					"SizeOfRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRawData, sizeof(section->PointerToRawData),			"PointerToRawData", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToRelocations, sizeof(section->PointerToRelocations),	"PointerToRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->PointerToLinenumbers, sizeof(section->PointerToLinenumbers),	"PointerToLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfRelocations, sizeof(section->NumberOfRelocations),		"NumberOfRelocations", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->NumberOfLinenumbers, sizeof(section->NumberOfLinenumbers),		"NumberOfLineNumbers", entry.m_SectionData);
		fhInsertEntry((unsigned char*)&section->Characteristics, sizeof(section->Characteristics),				"Characteristics", entry.m_SectionData);
		m_ParsedSectionHeaders.push_back(entry);
		/*
			We will add a function here to get hash/entropy of each section,
			will need to update the fh_Section struct to hold this information as well
		*/
		//GetHashes(Base + section->PointerToRawData, section->SizeOfRawData, &newSection.HashInfo);
		sectionAddr = sectionAddr + sizeof(IMAGE_SECTION_HEADER);
	}
}

DWORD PEDisector::RVA_ToRaw(DWORD rva)
{
	if (!m_bValidPEFile)
		return 0;

	// We have to reload the file so we can do all this jazz
	// as we do not save the bytes at the beginning of the file anywhere
	std::ifstream file(m_LoadFileName, std::ios::binary);
	if (!file.is_open())
		return 0;
	file.seekg(0, std::ios::end);
	std::streampos fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	auto fs = static_cast<int>(fileSize);
	utils::NewBuffer filebuf(INITIAL_PE_LOAD);
	auto load = min(INITIAL_PE_LOAD, fs);
	file.read((char*)filebuf.Get(), load);

	auto base = filebuf.Get();
	auto dos = (PIMAGE_DOS_HEADER)filebuf.Get();

	if (m_b64bit)
	{
		auto nt = (PIMAGE_NT_HEADERS64)((BYTE*)dos + dos->e_lfanew);
		return Rva2Offset(rva, IMAGE_FIRST_SECTION(nt), nt);
	}
	else
	{
		auto nt = (PIMAGE_NT_HEADERS32)((BYTE*)dos + dos->e_lfanew);
		return Rva2Offset32(rva, IMAGE_FIRST_SECTION(nt), nt);
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
	IMAGE_IMPORT_DESCRIPTOR descriptor;
	auto nt = (PIMAGE_NT_HEADERS32)((unsigned char*)dos + dos->e_lfanew);
	auto pSectionHeader = IMAGE_FIRST_SECTION(nt);
	int numSections = nt->FileHeader.NumberOfSections;
	if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		/*
			We need to check to make sure we are not accessing out of bounds offsets, if it is out of bounds, we will use our
			utility function to read the data from the file there
		*/
		auto iDescriptorOffset = (size_t)Rva2Offset32(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSectionHeader, nt);
		if (iDescriptorOffset > INITIAL_PE_LOAD)
			utils::readFromFileOffset(file, iDescriptorOffset, sizeof(IMAGE_IMPORT_DESCRIPTOR), &descriptor);
		else
		{
			iDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(Base + iDescriptorOffset);
			descriptor = *iDescriptor;
		}

		while (descriptor.Name != NULL)
		{
			fh_LibraryImport libImport;
			fhInsertEntry((unsigned char*)&descriptor.Characteristics, sizeof(descriptor.Characteristics), "Characteristics", libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.OriginalFirstThunk, sizeof(descriptor.OriginalFirstThunk), "OriginalFirstThunk", libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.TimeDateStamp, sizeof(descriptor.TimeDateStamp), "TimeDateStamp", libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.ForwarderChain, sizeof(descriptor.ForwarderChain), "ForwarderChain", libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.Name, sizeof(descriptor.Name), "Name", libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.FirstThunk, sizeof(descriptor.FirstThunk), "FirstThunk", libImport.m_ImportDescriptorData);

			auto libNameOffset = Rva2Offset32(descriptor.Name, pSectionHeader, nt);
			std::string library;
			if (libNameOffset > INITIAL_PE_LOAD)
				utils::readStringFromFileOffset(file, libNameOffset, library);
			else
				library = std::string((char*)(libNameOffset + Base));

			IMAGE_THUNK_DATA32 thunkData;
			auto thunkOffset = Rva2Offset32(descriptor.OriginalFirstThunk, pSectionHeader, nt);

			if (thunkOffset > INITIAL_PE_LOAD)
				utils::readFromFileOffset(file, thunkOffset, sizeof(IMAGE_THUNK_DATA32), &thunkData);
			else
			{
				auto thunkILT = (PIMAGE_THUNK_DATA32)(Base + thunkOffset);
				thunkData = *thunkILT;
			}

			libImport.m_Library = library;
			while (thunkData.u1.AddressOfData != 0)
			{
				fh_FunctionImport funcImport;
				if (!(thunkData.u1.Ordinal & IMAGE_ORDINAL_FLAG32))
				{

					// Messing up here, we arent reading in the function names correctly
					WORD hint;
					std::string function;

					auto nameArrayOffset = Rva2Offset32(thunkData.u1.AddressOfData, pSectionHeader, nt);
					if (nameArrayOffset > INITIAL_PE_LOAD)
					{
						utils::readStringFromFileOffset(file, nameArrayOffset + sizeof(WORD), function);
						utils::readFromFileOffset(file, nameArrayOffset, sizeof(WORD), &hint);
					}
					else
					{
						PIMAGE_IMPORT_BY_NAME nameArray = (PIMAGE_IMPORT_BY_NAME)(Base + nameArrayOffset);
						hint = nameArray->Hint;
						function = std::string((char*)(nameArray->Name));
					}

					funcImport.m_FunctionName = function;
					funcImport.m_RawThunk = thunkData.u1.AddressOfData;
					fhInsertEntry((unsigned char*)&thunkData.u1.AddressOfData, sizeof(IMAGE_THUNK_DATA32), "Thunk", funcImport.m_ImportInfo);
					fhInsertEntry((unsigned char*)&hint, sizeof(WORD), "Hint", funcImport.m_ImportInfo);
					libImport.m_FunctionImports.push_back(funcImport);
				}

				thunkOffset += sizeof(DWORD);
				utils::readFromFileOffset(file, thunkOffset, sizeof(IMAGE_THUNK_DATA32), &thunkData);
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
			fh_LibraryImport libImport;
			fhInsertEntry((unsigned char*)&descriptor.Characteristics, sizeof(descriptor.Characteristics), "Characteristics",			libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.OriginalFirstThunk, sizeof(descriptor.OriginalFirstThunk), "OriginalFirstThunk",	libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.TimeDateStamp, sizeof(descriptor.TimeDateStamp), "TimeDateStamp",					libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.ForwarderChain, sizeof(descriptor.ForwarderChain), "ForwarderChain",				libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.Name, sizeof(descriptor.Name), "Name",											libImport.m_ImportDescriptorData);
			fhInsertEntry((unsigned char*)&descriptor.FirstThunk, sizeof(descriptor.FirstThunk), "FirstThunk",							libImport.m_ImportDescriptorData);

			auto libNameOffset = Rva2Offset(descriptor.Name, pSectionHeader, nt);
			std::string library;
			if (libNameOffset > INITIAL_PE_LOAD)
				utils::readStringFromFileOffset(file, libNameOffset, library);
			else
				library = std::string ((char*)(libNameOffset + Base));

			IMAGE_THUNK_DATA thunkData;
			auto thunkOffset = Rva2Offset(descriptor.OriginalFirstThunk, pSectionHeader, nt);

			if (thunkOffset > INITIAL_PE_LOAD)
				utils::readFromFileOffset(file, thunkOffset, sizeof(ULONGLONG), &thunkData);
			else
			{
				auto thunkILT = (PIMAGE_THUNK_DATA)(Base + thunkOffset);
				thunkData = *thunkILT;
			}
					
			libImport.m_Library = library;
			while (thunkData.u1.AddressOfData != 0)
			{
				fh_FunctionImport funcImport;
				if (!(thunkData.u1.Ordinal & IMAGE_ORDINAL_FLAG))
				{

					// Messing up here, we arent reading in the function names correctly
					WORD hint;
					std::string function;

					auto nameArrayOffset = Rva2Offset(thunkData.u1.AddressOfData, pSectionHeader, nt);
					if (nameArrayOffset > INITIAL_PE_LOAD)
					{
						utils::readStringFromFileOffset(file, nameArrayOffset + sizeof(WORD), function);
						utils::readFromFileOffset(file, nameArrayOffset, sizeof(WORD), &hint);
					}	
					else
					{
						PIMAGE_IMPORT_BY_NAME nameArray = (PIMAGE_IMPORT_BY_NAME)(Base + nameArrayOffset);
						hint = nameArray->Hint;
						function = std::string((char*)(nameArray->Name));
					}
					
					funcImport.m_FunctionName = function;
					funcImport.m_RawThunk = thunkData.u1.AddressOfData;
					fhInsertEntry((unsigned char*)&thunkData.u1.AddressOfData, sizeof(ULONGLONG), "Thunk", funcImport.m_ImportInfo);
					fhInsertEntry((unsigned char*)&hint, sizeof(WORD), "Hint", funcImport.m_ImportInfo);
					libImport.m_FunctionImports.push_back(funcImport); 
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
	memcpy(m_ParsedRichHeader.key, richBuffer + (index + 4), 4);
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
		for (int j = 0; j < 8; j++)
		{
			entry.m_Raw.push_back(decrypted_rich[i + j]);
		}
		m_ParsedRichHeader.m_Entries[(i / 8) - 2] = entry;
	}

}
