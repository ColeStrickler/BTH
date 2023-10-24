#pragma once
#include <vector>
#include <unordered_set>
#include <map>
#include <math.h>
#include "../filesystem/filebrowser.h"


#include "../Dependencies/imgui/imgui.h"
#include "../Dependencies/imgui/imgui_impl_glfw.h"
#include "../Dependencies/imgui/imgui_impl_opengl3.h"


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
	std::vector<unsigned long long> scan_file(FileBrowser* fb, const std::vector<unsigned char>& pattern,  float& progress);

	std::vector<unsigned long long> m_ByteMatches;
private:
	
};

