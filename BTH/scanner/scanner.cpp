#include "scanner.h"

#include "../manager/manager.h"

Scanner::Scanner()
{
}

Scanner::~Scanner()
{
}

std::vector<unsigned int> Scanner::simple_scan(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes)
{
	std::vector<unsigned int> ret;

	for (int i = 0; i < bytes.size() - pattern.size(); i++)
	{
		bool add = true;
		for (int j = pattern.size() - 1; j >= 0; j--)
		{
			if (bytes[i + j] != pattern[j])
			{
				add = false;
				break;
			}
		}
		if (add)
		{
			ret.push_back(i);
		}
	}
	return ret;

}

/*
	scan_bytes is a Boyer-Moore implementation


	We will need to set this up to work with the dynamic file offset loading

*/
std::vector<unsigned long long> Scanner::scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, float* progress)
{
	m_ByteMatches.clear();

	// Construct good byte and skip tables
	std::unordered_set<unsigned char> good_bytes;
	std::map<std::pair<unsigned char, unsigned char>, int> skip_table;
	for (int i = pattern.size() - 1; i >= 0; i--)
	{
		auto& byte = pattern[i];
		good_bytes.insert(byte);
		for (int j = i-1; j >= 0; j--)
		{
			auto pair = std::make_pair(pattern[i], pattern[j]);
			if (skip_table[pair] == 0)
			{
				skip_table[pair] = INT_MAX;
			}
			skip_table[pair] = min(skip_table[pair], i - j);
		}
	}

	// i is alignment

	for (long long int i = 0; i < bytes.size() - pattern.size() && bytes.size() != 0; i++) 
	{
		int end = pattern.size() - 1;
		if (good_bytes.count(bytes[i + end]) == 0)
		{
			i += end;
			continue;
		}
		bool add = true;
		while (end >= 0)
		{
			if (pattern[end] != bytes[i + end])
			{
				i += skip_table[std::make_pair(bytes[i+end], pattern[end])];
				add = false;
				break;
			}
			end--;
		}
		if (add)
		{
			m_ByteMatches.push_back(i);
		}
		*progress = float(i / bytes.size());
	}
	return m_ByteMatches;
}

std::vector<unsigned long long> Scanner::scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, std::vector<unsigned long long>& out, size_t offset)
{
	// Construct good byte and skip tables
	std::unordered_set<unsigned char> good_bytes;
	std::map<std::pair<unsigned char, unsigned char>, int> skip_table;
	for (int i = pattern.size() - 1; i >= 0; i--)
	{
		auto& byte = pattern[i];
		good_bytes.insert(byte);
		for (int j = i - 1; j >= 0; j--)
		{
			auto pair = std::make_pair(pattern[i], pattern[j]);
			if (skip_table[pair] == 0)
			{
				skip_table[pair] = INT_MAX;
			}
			skip_table[pair] = min(skip_table[pair], i - j);
		}
	}

	// i is alignment

	for (long long int i = 0; i < bytes.size() - pattern.size() && bytes.size() != 0; i++)
	{
		int end = pattern.size() - 1;
		if (good_bytes.count(bytes[i + end]) == 0)
		{
			i += end;
			continue;
		}
		bool add = true;
		while (end >= 0)
		{
			if (pattern[end] != bytes[i + end])
			{
				i += skip_table[std::make_pair(bytes[i + end], pattern[end])];
				add = false;
				break;
			}
			end--;
		}
		if (add)
		{
			out.push_back(i + offset);
		}
	}
	return out;
}

std::vector<unsigned long long> Scanner::byte_scan_file(FileBrowser* fb, const std::vector<unsigned char>& pattern, Manager* mgr)
{	
	auto start_time = std::chrono::high_resolution_clock::now();
	mgr->SetByteScannerProgress(0.0f);
	mgr->SetByteScannerFinished(false);
	
	auto& file_size = fb->m_LoadedFileSize;
	DWORD scanned = 0;
	std::vector<unsigned long long> ret;

	while (scanned < file_size)
	{
		DWORD read;
		auto bytes = fb->LoadBytes(scanned, SCANNER_LOAD_MAX, &read);
		scan_bytes(pattern, bytes, ret, scanned);
		/*
			we back up by the size of the pattern because we do not want to miss
			cases when the pattern lies on the boundary of our load limit
		*/ 
		scanned += (read > SCANNER_LOAD_MAX - pattern.size() ?  read - pattern.size() : read);
		auto progress = min((static_cast<float>(scanned) / file_size), 100.0f);
		mgr->SetByteScannerProgress(progress);
	}

	m_ByteMatches.clear();
	m_ByteMatches = ret;
	auto end_time = std::chrono::high_resolution_clock::now();
	mgr->SetByteScannerFinished(true);
	m_ByteScanTime = end_time - start_time;

	return ret;
}

StringMatches Scanner::string_scan_file(FileBrowser* fb, Manager* mgr, int min_string_length)
{	
	auto start_time = std::chrono::high_resolution_clock::now();
	mgr->SetStringScannerProgress(0.0f);
	mgr->SetStringScannerFinished(false);
	auto& file_size = fb->m_LoadedFileSize;
	DWORD scanned = 0;
	StringMatches ret;

	while (scanned < file_size)
	{
		DWORD read;
		auto bytes = fb->LoadBytes(scanned, SCANNER_LOAD_MAX, &read);
		string_scan(bytes, ret, scanned, min_string_length);
		
		/*
			we back up by the size of the pattern because we do not want to miss
			cases when the pattern lies on the boundary of our load limit
		*/
		scanned += (read > SCANNER_LOAD_MAX - min_string_length ? read - min_string_length : read);
		auto progress = min((static_cast<float>(scanned) / file_size), 100.0f);
		mgr->SetStringScannerProgress(progress);
	}


	m_StringMatches.m_StandardStrings.clear();
	m_StringMatches.m_UnicodeStrings.clear();
	m_StringMatches = ret;
	auto end_time = std::chrono::high_resolution_clock::now();
	m_StringScanTime = end_time - start_time;
	mgr->SetStringScannerFinished(true);
	return m_StringMatches;
}



StringMatches Scanner::string_scan(const std::vector<unsigned char>& bytes, StringMatches& out, size_t offset, int min_string_length)
{
	m_StringMatches.m_StandardStrings.clear();
	m_StringMatches.m_UnicodeStrings.clear();
	char asciiLowBound = '!';
	char asciiHighBound = '~';

	wchar_t unicodeLowBound = L'!';
	wchar_t unicodeHighBound = L'~';

	StringMatch<std::string> currAsciiMatch;
	StringMatch<std::wstring> currUnicodeMatch;
	StringMatch<std::wstring> currUnicodeMatch2;
	for (int i = 0; i < bytes.size() - min_string_length; i++)
	{
		if (bytes[i] >= asciiLowBound && bytes[i] <= asciiHighBound)
		{
			currAsciiMatch.m_StringVal += bytes[i];
		}
		else
		{
			if (currAsciiMatch.m_StringVal.size() >= min_string_length)
			{
				currAsciiMatch.m_Offset = i - currAsciiMatch.m_StringVal.size() + offset;
				out.m_StandardStrings.push_back(currAsciiMatch);
			}
			currAsciiMatch.m_Offset = 0x0;
			currAsciiMatch.m_StringVal.clear();
		}

		if (i % 2 == 0)	// only mess with unicode every two bytes because thats how much wide chars use
		{
			wchar_t* wc = (wchar_t*)(bytes.data() + i);
			wchar_t wide_c = *wc;
			if (wide_c >= unicodeLowBound && wide_c <= unicodeHighBound)
			{
				currUnicodeMatch.m_StringVal += wide_c;
			}
			else
			{
				if (currUnicodeMatch.m_StringVal.size() >= min_string_length)
				{
					currUnicodeMatch.m_Offset = i - currUnicodeMatch.m_StringVal.size() * 2 + offset;
					out.m_UnicodeStrings.push_back(currUnicodeMatch);
				}
				currUnicodeMatch.m_Offset = 0x0;
				currUnicodeMatch.m_StringVal.clear();
			}
		}
		else // we also check to see if the unicode string starts on an odd offset 
		{
			wchar_t* wc = (wchar_t*)(bytes.data() + i);
			wchar_t wide_c = *wc;
			if (wide_c >= unicodeLowBound && wide_c <= unicodeHighBound)
			{
				currUnicodeMatch2.m_StringVal += wide_c;
			}
			else
			{
				if (currUnicodeMatch2.m_StringVal.size() >= min_string_length)
				{
					currUnicodeMatch2.m_Offset = i - currUnicodeMatch2.m_StringVal.size() * 2 + offset;
					out.m_UnicodeStrings.push_back(currUnicodeMatch2);
				}
				currUnicodeMatch2.m_Offset = 0x0;
				currUnicodeMatch2.m_StringVal.clear();
			}
		}
	}


	return out;
}
