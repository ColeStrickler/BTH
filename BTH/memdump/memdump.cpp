#include "memdump.h"


static std::string ucharToHexString(unsigned char value) {
	std::stringstream stream;
	stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
	return stream.str();
}



static std::string Wstring2String(const std::wstring& wstr)
{
	std::string ret(wstr.begin(), wstr.end());
	return ret;
}

template <typename T> 
static std::string DataToString(std::vector<unsigned char>& bytes, size_t offset, MemDumpStructEntry& se)
{
	std::string ret;
	if (se.m_Size == sizeof(T))
	{
		T* data = (T*)(bytes.data() + offset);
		ret = std::to_string(*data);
		return ret;
	}
	else
	{
		// protect from over indexing vector
		if (offset + se.m_Size > bytes.size())
			return "OVERFLOW";
		auto array_entries = se.m_Size / sizeof(T);
		for (int i = 0; i < array_entries; i++)
		{
			T* data = (T*)(bytes.data() + offset);
			ret += std::to_string(*data);
			ret += " ";
			offset += sizeof(T);

		}
		return ret;
	}
}

std::string DumpDisplayType2String(MEMDUMPDISPLAY type)
{
	switch (type)
	{
		case MEMDUMPDISPLAY::INT:
			return "INT";
		case MEMDUMPDISPLAY::LONG_INT:
			return "LONG_INT";
		case MEMDUMPDISPLAY::UNSIGNED_INT:
			return "UINT";
		case MEMDUMPDISPLAY::UNSIGNED_LONGLONG:
			return "ULONGLONG";
		case MEMDUMPDISPLAY::ASCII:
			return "ASCII";
		case MEMDUMPDISPLAY::UNICODE:
			return "UNICODE";
		case MEMDUMPDISPLAY::HEX:
			return "HEX";
		case MEMDUMPDISPLAY::BOOL:
			return "BOOL";
		default:
			return "";
	}
}

size_t DumpDisplaySize(MEMDUMPDISPLAY type)
{
	switch (type)
	{
	case MEMDUMPDISPLAY::INT:
		return sizeof(int);
	case MEMDUMPDISPLAY::LONG_INT:
		return sizeof(int64_t);
	case MEMDUMPDISPLAY::UNSIGNED_INT:
		return sizeof(unsigned int);
	case MEMDUMPDISPLAY::UNSIGNED_LONGLONG:
		return sizeof(unsigned long long);
	case MEMDUMPDISPLAY::ASCII:
		return sizeof(char);
	case MEMDUMPDISPLAY::UNICODE:
		return sizeof(wchar_t);
	case MEMDUMPDISPLAY::HEX:
		return sizeof(unsigned char);
	case MEMDUMPDISPLAY::BOOL:
		return sizeof(bool);
	default:
		return 0;
	}
}








static std::string HexDataToString(std::vector<unsigned char>& bytes, size_t offset, MemDumpStructEntry& se)
{
	std::string ret;
	if (se.m_Size == sizeof(unsigned char))
	{
		unsigned char* data = (bytes.data() + offset);
		ret = std::to_string(*data);
		return ret;
	}
	else
	{
		// protect from over indexing vector
		if (offset + se.m_Size > bytes.size())
			return "OVERFLOW";
		auto array_entries = se.m_Size / sizeof(unsigned char);
		for (int i = 0; i < array_entries; i++)
		{
			unsigned char* data = (bytes.data() + offset);
			ret += ucharToHexString(*data);
			ret += " ";
			offset += sizeof(unsigned char);

		}
		return ret;
	}
}




static std::string DataStringToString(std::vector<unsigned char>& bytes, size_t offset, MemDumpStructEntry& se)
{
	if (offset + se.m_Size > bytes.size())
		return "OVERFLOW";
	std::string ret;
	char* data = (char*)(bytes.data() + offset);
	for (int i = 0; i < se.m_Size; i += sizeof(char))
	{
		ret += *data;
		data++;
	}
	return ret;
}

static std::string DataWStringToString(std::vector<unsigned char>& bytes, size_t offset, MemDumpStructEntry& se)
{
	if (offset + se.m_Size > bytes.size())
		return "OVERFLOW";
	std::string ret;
	std::wstring tmp;
	wchar_t* data = (wchar_t*)(bytes.data() + offset);
	for (int i = 0; i < se.m_Size; i += sizeof(wchar_t))
	{
		tmp += *data;
		data++;
	}
	ret = Wstring2String(tmp);
	return ret;
}





static std::string GetDisplayString(std::vector<unsigned char>& bytes, size_t offset, MemDumpStructEntry& se)
{
	switch (se.m_Display)
	{
		case MEMDUMPDISPLAY::INT:
		{
			return DataToString<int>(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::LONG_INT:
		{
			return DataToString<int64_t>(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::UNSIGNED_INT:
		{
			return DataToString<unsigned int>(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::UNSIGNED_LONGLONG:
		{
			return DataToString<unsigned long long>(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::ASCII:
		{
			return DataStringToString(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::UNICODE:
		{
			return DataWStringToString(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::HEX:
		{
			return HexDataToString(bytes, offset, se);
		}
		case MEMDUMPDISPLAY::BOOL:
		{
			return DataToString<bool>(bytes, offset, se);
		}
		default:
			return "";
	}

}




MemDumpStructure::MemDumpStructure(const std::string& structure_name)
{
	m_Name = structure_name;
}

MemDumpStructure::~MemDumpStructure()
{

}

std::vector<MemDumpDisplayEntry> MemDumpStructure::GetDisplayData(std::vector<unsigned char> bytes, size_t offset)
{
	if (offset >= bytes.size())
		return {};
	unsigned char* struct_data = bytes.data() + offset;

	size_t curr_offset = offset;
	std::vector<MemDumpDisplayEntry> ret;

	for (auto& se : m_StructEntries)
	{
		MemDumpDisplayEntry entry;
		entry.m_SE = se;
		entry.m_Display = GetDisplayString(bytes, curr_offset, se);
		ret.push_back(entry);
		curr_offset += se.m_Size;
	}

	m_DisplayEntries = ret;
	return ret;
}

std::vector<MemDumpDisplayEntry> MemDumpStructure::GetDisplayData()
{
	std::vector<MemDumpDisplayEntry> ret;
	for (auto& se : m_StructEntries)
	{
		MemDumpDisplayEntry entry;
		entry.m_SE = se;
		ret.push_back(entry);
	}
	return ret;
}

MemDumpStructEntry& MemDumpStructure::GetSelectedEntry(int entry)
{
	MemDumpStructEntry ret;
	if (entry >= m_StructEntries.size())
		return ret;

	auto& x = m_StructEntries[entry];
	return x;
}

void MemDumpStructure::EditFieldEntry(int entry, const std::string& new_fieldname)
{
	if (entry >= m_StructEntries.size())
		return;

	auto& selected = m_StructEntries[entry];
	selected.m_GivenName = new_fieldname;

}

void MemDumpStructure::EditDisplayEntry(int entry, MEMDUMPDISPLAY new_display)
{
	if (entry >= m_StructEntries.size())
		return;

	auto& selected = m_StructEntries[entry];
	selected.m_Display = new_display;
}

void MemDumpStructure::EditSizeEntry(int entry, size_t new_size)
{
	if (entry >= m_StructEntries.size())
		return;

	auto& selected = m_StructEntries[entry];
	auto& display_type = selected.m_Display;
	auto dtype_size = DumpDisplaySize(display_type);

	if (new_size % dtype_size == 0)
		selected.m_Size = new_size;
}

