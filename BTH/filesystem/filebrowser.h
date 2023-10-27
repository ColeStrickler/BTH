#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <iostream>
#include "../utils/utils.h"


namespace fs = std::filesystem;

enum class FB_RETCODE : short
{
	NO_CHANGE_LOAD,
	OFFSET_CHANGE_LOAD,
	FILE_CHANGE_LOAD,
	FILE_ERROR_LOAD
};



class FileBrowser
{
public:
	// Max Load Size = 2mb
	FileBrowser(size_t MaxLoadSize = 2000);
	~FileBrowser();

	std::vector<fs::directory_entry> ListDirectory(const std::string& path);
	std::vector<std::string> PathToStringVec(const std::vector<fs::path>& pvec) const;
	std::vector<std::string> CurrentDirectoryToStringVec();
	std::vector<fs::directory_entry> DisplayFilter();
	FB_RETCODE LoadFile(const std::string& filepath, const size_t& offset = 0);
	std::vector<unsigned char> LoadBytes(unsigned long long offset, DWORD numToRead, DWORD* numRead);
	void SetInputPath(const std::string& new_inputpath);

	void DisplayCurrentDirectory();

	/*
	
		Will check if input data is a directory and only then update m_CurrentDirectory,
		and then from this, we can apply the filter to only the listings
	
	*/


	std::vector<fs::directory_entry> m_CurrentDirectory;
	std::vector<std::string> m_CurrentDirectoryListings;
	std::vector<unsigned char> m_FileLoadData;
	char m_InputPath[_MAX_PATH];
	std::string m_LoadedFileName;
	size_t m_LoadedFileSize;
	int m_CurrentBounds[2];
private:
	size_t m_MaxLoadSize;
	
	

};

