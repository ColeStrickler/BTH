#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstddef>
#include <Windows.h>


namespace utils
{
	int stringToHex(const std::string& str);


    class NewBuffer
    {
    public:
        NewBuffer(size_t size);
        NewBuffer(unsigned char* buffer);
        unsigned char* Get();
        ~NewBuffer();
    private:
        unsigned char* buf;
    };

    void readFromFileOffset(std::ifstream& file, size_t offset, size_t toRead, void* out);
    void readStringFromFileOffset(std::ifstream& file, size_t offset, std::string& out);
    void readThunkData(std::ifstream& file, size_t offset, size_t toRead, void* out);
}