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

std::vector<unsigned long long> Scanner::scan_file(FileBrowser* fb, const std::vector<unsigned char>& pattern, Manager* mgr)
{
	mgr->SetByteScannerProgress(0.0f);
	auto start_time = std::chrono::high_resolution_clock::now();
	mgr->SetByteScannerFinished(false);
	
	auto& file_size = fb->m_LoadedFileSize;
	DWORD scanned = 0;
	std::vector<unsigned long long> ret;

	while (scanned < file_size)
	{
		DWORD read;
		auto bytes = fb->LoadBytes(scanned, 200000, &read);
		scan_bytes(pattern, bytes, ret, scanned);
		scanned += read;
		auto progress = min((static_cast<float>(scanned) / file_size), 100.0f);
		mgr->SetByteScannerProgress(progress);
	}

	m_ByteMatches.clear();
	m_ByteMatches = ret;
	auto end_time = std::chrono::high_resolution_clock::now();
	mgr->SetByteScannerFinished(true);
	m_ScanTime = end_time - start_time;

	return ret;
}
