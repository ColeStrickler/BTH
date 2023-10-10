#pragma once
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;


class FileBrowser
{
public:
	FileBrowser();
	~FileBrowser();

	std::vector<fs::directory_entry> ListDirectory(const std::string& path);
	std::vector<std::string> PathToStringVec(const std::vector<fs::path>& pvec) const;
	std::vector<std::string> CurrentDirectoryToStringVec();
	std::vector<std::string> DisplayFilter(std::vector<std::string>& unfiltered);
	unsigned char* LoadNewFile(const std::string& filepath);
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
	

};

