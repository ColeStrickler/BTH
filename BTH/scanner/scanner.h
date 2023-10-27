#pragma once
#include <vector>
#include <unordered_set>
#include <map>
#include <math.h>
#include "../filesystem/filebrowser.h"


#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"

#define SCANNER_LOAD_MAX 200000
class Manager;


template <typename T>
struct StringMatch
{
	T m_StringVal;
	unsigned long long m_Offset;
};


struct StringMatches
{
	std::vector<StringMatch<std::string>> m_StandardStrings;
	std::vector<StringMatch<std::wstring>> m_UnicodeStrings;
};



class Scanner
{
public:
	Scanner();
	~Scanner();
	


	/*
		simple_scan is naive pattern matching and is very slow
	*/
	std::vector<unsigned int> simple_scan(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes);

	/*
		scan_bytes is a boyer-moore implementation
	*/
	std::vector<unsigned long long> scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, float* progress);
	std::vector<unsigned long long> scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, std::vector<unsigned long long>& out, size_t offset);
	std::vector<unsigned long long> byte_scan_file(FileBrowser* fb, const std::vector<unsigned char>& pattern, Manager* mgr);
	std::vector<unsigned long long> m_ByteMatches;
	

	// string_scan_file is a wrapper function that will call string_scan over and over
	StringMatches string_scan_file(FileBrowser* fb, Manager* mgr, int min_string_length);
	// string_scan is a single pass O(n) string scan that will extract both ascii and Unicode 
	StringMatches string_scan(const std::vector<unsigned char>& bytes, StringMatches& out, size_t offset, int min_string_length);
	StringMatches m_StringMatches;


	
	
	std::chrono::duration<double> m_ByteScanTime;
	std::chrono::duration<double> m_StringScanTime;
private:
	
};

