#pragma once
#include <string>
#include <sstream>
#include <iomanip>


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
	
}