#pragma once
#include <vector>
#include <unordered_set>
#include <map>
#include <math.h>


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
	std::vector<long long int> scan_bytes(const std::vector<unsigned char>& pattern, const std::vector<unsigned char>& bytes, float* progress) ;
	std::vector<long long int> m_ByteMatches;
private:
	
};

