#pragma once
#include <vector>
#include <string>
#include <locale>
#include <codecvt>
#include "..\utils\utils.h"


enum class MEMDUMPDISPLAY : short
{
	INT,
	LONG_INT,
	UNSIGNED_INT,
	UNSIGNED_LONGLONG,
	ASCII,
	UNICODE,
	HEX,
};



// For each member of a use defined struct we will have a corresponding
// MemDumpStructEntry that will dictate how the data is displayed
struct MemDumpStructEntry
{
	int m_Size;
	MEMDUMPDISPLAY m_Display;
};

struct MemDumpDisplayEntry
{
	size_t m_Size;
	std::string m_Display;
};



class MemDump
{
public:
	MemDump(const std::vector<MemDumpStructEntry>& struct_entries);
	~MemDump();
	
	std::vector<MemDumpDisplayEntry> GetDisplayData(std::vector<unsigned char> bytes, size_t offset);

private:
	std::vector<MemDumpStructEntry> m_StructEntries;
	std::vector<MemDumpDisplayEntry> m_DisplayEntries;
};

