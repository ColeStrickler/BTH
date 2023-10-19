#include "utils.h"
#include <iostream>

int utils::stringToHex(const std::string& str)
{
    if (!str.size())
    {
        return 0;
    }

	int ret;
	std::istringstream iss(str);
	iss >> std::hex >> ret;
    if (iss.fail()) {
        // The hex string was unsuccessfully parsed.
        return 0;
    }
	return ret;
}




utils::NewBuffer::NewBuffer(size_t size) {
    buf = new unsigned char[size];
    memset(buf, 0, size);
}


utils::NewBuffer::NewBuffer(unsigned char* buffer) {
    buf = buffer;
}

utils::NewBuffer::~NewBuffer() {
    delete buf;
}

unsigned char* utils::NewBuffer::Get() {
    return buf;
}