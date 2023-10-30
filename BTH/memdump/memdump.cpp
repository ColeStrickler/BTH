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
		default:
			return "";
	}

}




MemDump::MemDump(const std::vector<MemDumpStructEntry>& struct_entries)
{
	m_StructEntries = struct_entries;
}

std::vector<MemDumpDisplayEntry> MemDump::GetDisplayData(std::vector<unsigned char> bytes, size_t offset)
{
	ASSERT(offset < bytes.size());
	unsigned char* struct_data = bytes.data() + offset;

	size_t curr_offset = offset;
	std::vector<MemDumpDisplayEntry> ret;

	for (auto& se : m_StructEntries)
	{
		MemDumpDisplayEntry entry;
		entry.m_Size = se.m_Size;
		entry.m_Display = GetDisplayString(bytes, curr_offset, se);
		ret.push_back(entry);
		curr_offset += se.m_Size;
	}

	m_DisplayEntries = ret;
	return ret;
}
