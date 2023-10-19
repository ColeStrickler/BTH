#include "scanner.h"

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
std::vector<long long int> Scanner::scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, float* progress)
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
			skip_table[pair] = std::min(skip_table[pair], i - j);
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
