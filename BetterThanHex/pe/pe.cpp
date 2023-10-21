#include "pe.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unordered_map>
#include <fstream>



static std::string RichHdr_ProdIdToVSversion(WORD i)
{
	//list based on: https://github.com/kirschju/richheader + pnx's notes
	if (i >= 0x0106 && i < (0x010a + 1))
		return "Visual Studio 2017 14.01+";
	if (i >= 0x00fd && i < (0x0106))
		return "Visual Studio 2015 14.00";
	if (i >= 0x00eb && i < 0x00fd)
		return "Visual Studio 2013 12.10";
	if (i >= 0x00d9 && i < 0x00eb)
		return "Visual Studio 2013 12.00";
	if (i >= 0x00c7 && i < 0x00d9)
		return "Visual Studio 2012 11.00";
	if (i >= 0x00b5 && i < 0x00c7)
		return "Visual Studio 2010 10.10";
	if (i >= 0x0098 && i < 0x00b5)
		return "Visual Studio 2010 10.00";
	if (i >= 0x0083 && i < 0x0098)
		return "Visual Studio 2008 09.00";
	if (i >= 0x006d && i < 0x0083)
		return "Visual Studio 2005 08.00";
	if (i >= 0x005a && i < 0x006d)
		return "Visual Studio 2003 07.10";
	if (i >= 0x0019 && i < (0x0045 + 1))
		return "Visual Studio 2002 07.00";
	if (i == 0xA || i == 0xB || i == 0xD || i == 0x15 || i == 0x16)
		return "Visual Studio 6.0 06.00";
	if (i == 0x2 || i == 0x6 || i == 0xC || i == 0xE)
		return "Visual Studio 97 05.00";
	if (i == 1)
		return "Visual Studio";
	return "";
}

static std::string RichHdr_translateProdId(WORD prodId)
{
	//list from: https://github.com/kirschju/richheader
	switch (prodId) {
	case 0x0000: return "Unknown";
	case 0x0001: return "Import0";
	case 0x0002: return "Linker510";
	case 0x0003: return "Cvtomf510";
	case 0x0004: return "Linker600";
	case 0x0005: return "Cvtomf600";
	case 0x0006: return "Cvtres500";
	case 0x0007: return "Utc11_Basic";
	case 0x0008: return "Utc11_C";
	case 0x0009: return "Utc12_Basic";
	case 0x000a: return "Utc12_C";
	case 0x000b: return "Utc12_CPP";
	case 0x000c: return "AliasObj60";
	case 0x000d: return "VisualBasic60";
	case 0x000e: return "Masm613";
	case 0x000f: return "Masm710";
	case 0x0010: return "Linker511";
	case 0x0011: return "Cvtomf511";
	case 0x0012: return "Masm614";
	case 0x0013: return "Linker512";
	case 0x0014: return "Cvtomf512";
	case 0x0015: return "Utc12_C_Std";
	case 0x0016: return "Utc12_CPP_Std";
	case 0x0017: return "Utc12_C_Book";
	case 0x0018: return "Utc12_CPP_Book";
	case 0x0019: return "Implib700";
	case 0x001a: return "Cvtomf700";
	case 0x001b: return "Utc13_Basic";
	case 0x001c: return "Utc13_C";
	case 0x001d: return "Utc13_CPP";
	case 0x001e: return "Linker610";
	case 0x001f: return "Cvtomf610";
	case 0x0020: return "Linker601";
	case 0x0021: return "Cvtomf601";
	case 0x0022: return "Utc12_1_Basic";
	case 0x0023: return "Utc12_1_C";
	case 0x0024: return "Utc12_1_CPP";
	case 0x0025: return "Linker620";
	case 0x0026: return "Cvtomf620";
	case 0x0027: return "AliasObj70";
	case 0x0028: return "Linker621";
	case 0x0029: return "Cvtomf621";
	case 0x002a: return "Masm615";
	case 0x002b: return "Utc13_LTCG_C";
	case 0x002c: return "Utc13_LTCG_CPP";
	case 0x002d: return "Masm620";
	case 0x002e: return "ILAsm100";
	case 0x002f: return "Utc12_2_Basic";
	case 0x0030: return "Utc12_2_C";
	case 0x0031: return "Utc12_2_CPP";
	case 0x0032: return "Utc12_2_C_Std";
	case 0x0033: return "Utc12_2_CPP_Std";
	case 0x0034: return "Utc12_2_C_Book";
	case 0x0035: return "Utc12_2_CPP_Book";
	case 0x0036: return "Implib622";
	case 0x0037: return "Cvtomf622";
	case 0x0038: return "Cvtres501";
	case 0x0039: return "Utc13_C_Std";
	case 0x003a: return "Utc13_CPP_Std";
	case 0x003b: return "Cvtpgd1300";
	case 0x003c: return "Linker622";
	case 0x003d: return "Linker700";
	case 0x003e: return "Export622";
	case 0x003f: return "Export700";
	case 0x0040: return "Masm700";
	case 0x0041: return "Utc13_POGO_I_C";
	case 0x0042: return "Utc13_POGO_I_CPP";
	case 0x0043: return "Utc13_POGO_O_C";
	case 0x0044: return "Utc13_POGO_O_CPP";
	case 0x0045: return "Cvtres700";
	case 0x0046: return "Cvtres710p";
	case 0x0047: return "Linker710p";
	case 0x0048: return "Cvtomf710p";
	case 0x0049: return "Export710p";
	case 0x004a: return "Implib710p";
	case 0x004b: return "Masm710p";
	case 0x004c: return "Utc1310p_C";
	case 0x004d: return "Utc1310p_CPP";
	case 0x004e: return "Utc1310p_C_Std";
	case 0x004f: return "Utc1310p_CPP_Std";
	case 0x0050: return "Utc1310p_LTCG_C";
	case 0x0051: return "Utc1310p_LTCG_CPP";
	case 0x0052: return "Utc1310p_POGO_I_C";
	case 0x0053: return "Utc1310p_POGO_I_CPP";
	case 0x0054: return "Utc1310p_POGO_O_C";
	case 0x0055: return "Utc1310p_POGO_O_CPP";
	case 0x0056: return "Linker624";
	case 0x0057: return "Cvtomf624";
	case 0x0058: return "Export624";
	case 0x0059: return "Implib624";
	case 0x005a: return "Linker710";
	case 0x005b: return "Cvtomf710";
	case 0x005c: return "Export710";
	case 0x005d: return "Implib710";
	case 0x005e: return "Cvtres710";
	case 0x005f: return "Utc1310_C";
	case 0x0060: return "Utc1310_CPP";
	case 0x0061: return "Utc1310_C_Std";
	case 0x0062: return "Utc1310_CPP_Std";
	case 0x0063: return "Utc1310_LTCG_C";
	case 0x0064: return "Utc1310_LTCG_CPP";
	case 0x0065: return "Utc1310_POGO_I_C";
	case 0x0066: return "Utc1310_POGO_I_CPP";
	case 0x0067: return "Utc1310_POGO_O_C";
	case 0x0068: return "Utc1310_POGO_O_CPP";
	case 0x0069: return "AliasObj710";
	case 0x006a: return "AliasObj710p";
	case 0x006b: return "Cvtpgd1310";
	case 0x006c: return "Cvtpgd1310p";
	case 0x006d: return "Utc1400_C";
	case 0x006e: return "Utc1400_CPP";
	case 0x006f: return "Utc1400_C_Std";
	case 0x0070: return "Utc1400_CPP_Std";
	case 0x0071: return "Utc1400_LTCG_C";
	case 0x0072: return "Utc1400_LTCG_CPP";
	case 0x0073: return "Utc1400_POGO_I_C";
	case 0x0074: return "Utc1400_POGO_I_CPP";
	case 0x0075: return "Utc1400_POGO_O_C";
	case 0x0076: return "Utc1400_POGO_O_CPP";
	case 0x0077: return "Cvtpgd1400";
	case 0x0078: return "Linker800";
	case 0x0079: return "Cvtomf800";
	case 0x007a: return "Export800";
	case 0x007b: return "Implib800";
	case 0x007c: return "Cvtres800";
	case 0x007d: return "Masm800";
	case 0x007e: return "AliasObj800";
	case 0x007f: return "PhoenixPrerelease";
	case 0x0080: return "Utc1400_CVTCIL_C";
	case 0x0081: return "Utc1400_CVTCIL_CPP";
	case 0x0082: return "Utc1400_LTCG_MSIL";
	case 0x0083: return "Utc1500_C";
	case 0x0084: return "Utc1500_CPP";
	case 0x0085: return "Utc1500_C_Std";
	case 0x0086: return "Utc1500_CPP_Std";
	case 0x0087: return "Utc1500_CVTCIL_C";
	case 0x0088: return "Utc1500_CVTCIL_CPP";
	case 0x0089: return "Utc1500_LTCG_C";
	case 0x008a: return "Utc1500_LTCG_CPP";
	case 0x008b: return "Utc1500_LTCG_MSIL";
	case 0x008c: return "Utc1500_POGO_I_C";
	case 0x008d: return "Utc1500_POGO_I_CPP";
	case 0x008e: return "Utc1500_POGO_O_C";
	case 0x008f: return "Utc1500_POGO_O_CPP";
	case 0x0090: return "Cvtpgd1500";
	case 0x0091: return "Linker900";
	case 0x0092: return "Export900";
	case 0x0093: return "Implib900";
	case 0x0094: return "Cvtres900";
	case 0x0095: return "Masm900";
	case 0x0096: return "AliasObj900";
	case 0x0097: return "Resource";
	case 0x0098: return "AliasObj1000";
	case 0x0099: return "Cvtpgd1600";
	case 0x009a: return "Cvtres1000";
	case 0x009b: return "Export1000";
	case 0x009c: return "Implib1000";
	case 0x009d: return "Linker1000";
	case 0x009e: return "Masm1000";
	case 0x009f: return "Phx1600_C";
	case 0x00a0: return "Phx1600_CPP";
	case 0x00a1: return "Phx1600_CVTCIL_C";
	case 0x00a2: return "Phx1600_CVTCIL_CPP";
	case 0x00a3: return "Phx1600_LTCG_C";
	case 0x00a4: return "Phx1600_LTCG_CPP";
	case 0x00a5: return "Phx1600_LTCG_MSIL";
	case 0x00a6: return "Phx1600_POGO_I_C";
	case 0x00a7: return "Phx1600_POGO_I_CPP";
	case 0x00a8: return "Phx1600_POGO_O_C";
	case 0x00a9: return "Phx1600_POGO_O_CPP";
	case 0x00aa: return "Utc1600_C";
	case 0x00ab: return "Utc1600_CPP";
	case 0x00ac: return "Utc1600_CVTCIL_C";
	case 0x00ad: return "Utc1600_CVTCIL_CPP";
	case 0x00ae: return "Utc1600_LTCG_C";
	case 0x00af: return "Utc1600_LTCG_CPP";
	case 0x00b0: return "Utc1600_LTCG_MSIL";
	case 0x00b1: return "Utc1600_POGO_I_C";
	case 0x00b2: return "Utc1600_POGO_I_CPP";
	case 0x00b3: return "Utc1600_POGO_O_C";
	case 0x00b4: return "Utc1600_POGO_O_CPP";
	case 0x00b5: return "AliasObj1010";
	case 0x00b6: return "Cvtpgd1610";
	case 0x00b7: return "Cvtres1010";
	case 0x00b8: return "Export1010";
	case 0x00b9: return "Implib1010";
	case 0x00ba: return "Linker1010";
	case 0x00bb: return "Masm1010";
	case 0x00bc: return "Utc1610_C";
	case 0x00bd: return "Utc1610_CPP";
	case 0x00be: return "Utc1610_CVTCIL_C";
	case 0x00bf: return "Utc1610_CVTCIL_CPP";
	case 0x00c0: return "Utc1610_LTCG_C";
	case 0x00c1: return "Utc1610_LTCG_CPP";
	case 0x00c2: return "Utc1610_LTCG_MSIL";
	case 0x00c3: return "Utc1610_POGO_I_C";
	case 0x00c4: return "Utc1610_POGO_I_CPP";
	case 0x00c5: return "Utc1610_POGO_O_C";
	case 0x00c6: return "Utc1610_POGO_O_CPP";
	case 0x00c7: return "AliasObj1100";
	case 0x00c8: return "Cvtpgd1700";
	case 0x00c9: return "Cvtres1100";
	case 0x00ca: return "Export1100";
	case 0x00cb: return "Implib1100";
	case 0x00cc: return "Linker1100";
	case 0x00cd: return "Masm1100";
	case 0x00ce: return "Utc1700_C";
	case 0x00cf: return "Utc1700_CPP";
	case 0x00d0: return "Utc1700_CVTCIL_C";
	case 0x00d1: return "Utc1700_CVTCIL_CPP";
	case 0x00d2: return "Utc1700_LTCG_C";
	case 0x00d3: return "Utc1700_LTCG_CPP";
	case 0x00d4: return "Utc1700_LTCG_MSIL";
	case 0x00d5: return "Utc1700_POGO_I_C";
	case 0x00d6: return "Utc1700_POGO_I_CPP";
	case 0x00d7: return "Utc1700_POGO_O_C";
	case 0x00d8: return "Utc1700_POGO_O_CPP";
	case 0x00d9: return "AliasObj1200";
	case 0x00da: return "Cvtpgd1800";
	case 0x00db: return "Cvtres1200";
	case 0x00dc: return "Export1200";
	case 0x00dd: return "Implib1200";
	case 0x00de: return "Linker1200";
	case 0x00df: return "Masm1200";
	case 0x00e0: return "Utc1800_C";
	case 0x00e1: return "Utc1800_CPP";
	case 0x00e2: return "Utc1800_CVTCIL_C";
	case 0x00e3: return "Utc1800_CVTCIL_CPP";
	case 0x00e4: return "Utc1800_LTCG_C";
	case 0x00e5: return "Utc1800_LTCG_CPP";
	case 0x00e6: return "Utc1800_LTCG_MSIL";
	case 0x00e7: return "Utc1800_POGO_I_C";
	case 0x00e8: return "Utc1800_POGO_I_CPP";
	case 0x00e9: return "Utc1800_POGO_O_C";
	case 0x00ea: return "Utc1800_POGO_O_CPP";
	case 0x00eb: return "AliasObj1210";
	case 0x00ec: return "Cvtpgd1810";
	case 0x00ed: return "Cvtres1210";
	case 0x00ee: return "Export1210";
	case 0x00ef: return "Implib1210";
	case 0x00f0: return "Linker1210";
	case 0x00f1: return "Masm1210";
	case 0x00f2: return "Utc1810_C";
	case 0x00f3: return "Utc1810_CPP";
	case 0x00f4: return "Utc1810_CVTCIL_C";
	case 0x00f5: return "Utc1810_CVTCIL_CPP";
	case 0x00f6: return "Utc1810_LTCG_C";
	case 0x00f7: return "Utc1810_LTCG_CPP";
	case 0x00f8: return "Utc1810_LTCG_MSIL";
	case 0x00f9: return "Utc1810_POGO_I_C";
	case 0x00fa: return "Utc1810_POGO_I_CPP";
	case 0x00fb: return "Utc1810_POGO_O_C";
	case 0x00fc: return "Utc1810_POGO_O_CPP";
	case 0x00fd: return "AliasObj1400";
	case 0x00fe: return "Cvtpgd1900";
	case 0x00ff: return "Cvtres1400";
	case 0x0100: return "Export1400";
	case 0x0101: return "Implib1400";
	case 0x0102: return "Linker1400";
	case 0x0103: return "Masm1400";
	case 0x0104: return "Utc1900_C";
	case 0x0105: return "Utc1900_CPP";
	case 0x0106: return "Utc1900_CVTCIL_C";
	case 0x0107: return "Utc1900_CVTCIL_CPP";
	case 0x0108: return "Utc1900_LTCG_C";
	case 0x0109: return "Utc1900_LTCG_CPP";
	case 0x010a: return "Utc1900_LTCG_MSIL";
	case 0x010b: return ": 'Utc1900_POGO_I_C";
	case 0x010c: return "Utc1900_POGO_I_CPP";
	case 0x010d: return "Utc1900_POGO_O_C";
	case 0x010e: return "Utc1900_POGO_O_CPP";
	}
	return "?";
}



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

PEDisector::PEDisector(std::vector<unsigned char>& bytes)
{
	auto base = bytes.data();
	auto dos = (PIMAGE_DOS_HEADER)bytes.data();
	auto nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew); 
	m_bValidPEFile = EqualBytes((unsigned char*)& nt->Signature, (unsigned char*)"PE\0\0", sizeof(nt->Signature));
	printf("Valid Pe: %d\n", m_bValidPEFile);
	auto fh = &nt->FileHeader;
	auto opt = &nt->OptionalHeader;
	ParseDosHeader(dos);
	ParseRichHeader(dos);
	ParseFileHeader(fh);
	ParseOptionalHeader(dos);
	ParseSectionHeaders(dos);
	ParseImports(dos);
	

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
void PEDisector::ParseImports(PIMAGE_DOS_HEADER dos)
{
	if (m_b64bit)
		ParseImports64(dos);
	else
		ParseImports32(dos);
}

void PEDisector::ParseImports32(PIMAGE_DOS_HEADER dos)
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

void PEDisector::ParseImports64(PIMAGE_DOS_HEADER dos)
{
	unsigned char* Base = (unsigned char*)dos;
	PIMAGE_IMPORT_DESCRIPTOR iDescriptor = nullptr;
	auto nt = (PIMAGE_NT_HEADERS64)((unsigned char*)dos + dos->e_lfanew);
	auto pSectionHeader = IMAGE_FIRST_SECTION(nt);
	int numSections = nt->FileHeader.NumberOfSections;
	if (nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		iDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)(Base + Rva2Offset(nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, pSectionHeader, nt));
		while (iDescriptor->Name != NULL)
		{
			std::string library(((char*)(Rva2Offset(iDescriptor->Name, pSectionHeader, nt) + Base)));
			auto thunkILT = (PIMAGE_THUNK_DATA)(Base + Rva2Offset(iDescriptor->OriginalFirstThunk, pSectionHeader, nt));
			fh_LibraryImport libImport;
			libImport.m_Library = library;
			while (thunkILT->u1.AddressOfData != 0)
			{
				fh_FunctionImport funcImport;
				if (!(thunkILT->u1.Ordinal & IMAGE_ORDINAL_FLAG))
				{
					PIMAGE_IMPORT_BY_NAME nameArray = (PIMAGE_IMPORT_BY_NAME)(Rva2Offset(thunkILT->u1.AddressOfData, pSectionHeader, nt));
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


/*
	I was standing on the shoulders of giants for this method. See the following
	https://github.com/hasherezade/bearparser/tree/3816d7e7f441d97e594fc9f4a12b01bf71953c3c
	https://github.com/0xRick/PE-Parser
*/
void PEDisector::ParseRichHeader(PIMAGE_DOS_HEADER dos)
{
	// replace with utils::NewBuffer()
	auto richBuffer = new unsigned char[dos->e_lfanew];
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
	auto decrypted_rich = new unsigned char[size];
	memcpy(decrypted_rich, richBuffer + (start_index - size), size);
	delete richBuffer;

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

	delete decrypted_rich;
	//PrintRichHeader(m_ParsedRichHeader);
}
