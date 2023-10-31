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
	BOOL
};

std::string DumpDisplayType2String(MEMDUMPDISPLAY type);
size_t DumpDisplaySize(MEMDUMPDISPLAY type);

// For each member of a use defined struct we will have a corresponding
// MemDumpStructEntry that will dictate how the data is displayed
struct MemDumpStructEntry
{
	int m_Size;
	MEMDUMPDISPLAY m_Display;
	std::string m_GivenName;
};

struct MemDumpDisplayEntry
{
	MemDumpStructEntry m_SE;
	std::string m_Display;
};



class MemDumpStructure
{
public:
	MemDumpStructure(const std::string& structure_name);
	~MemDumpStructure();
	

	inline void AddEntry(const MemDumpStructEntry& entry) { m_StructEntries.push_back(entry); };
	inline void RemoveEntry() { if (m_StructEntries.size()) { m_StructEntries.pop_back(); } };
	std::vector<MemDumpDisplayEntry> GetDisplayData(std::vector<unsigned char> bytes, size_t offset);
	std::vector<MemDumpDisplayEntry> GetDisplayData();
	MemDumpStructEntry& GetSelectedEntry(int entry);
	std::vector<MemDumpStructEntry> GetAllEntries() const { return m_StructEntries; };
	void EditFieldEntry(int entry, const std::string& new_fieldname);
	void EditDisplayEntry(int entry, MEMDUMPDISPLAY new_display);
	void EditSizeEntry(int entry, size_t new_size);
	std::string m_Name;



	
private:
	
	std::vector<MemDumpStructEntry> m_StructEntries;
	std::vector<MemDumpDisplayEntry> m_DisplayEntries;
};

