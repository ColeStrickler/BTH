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

void utils::readFromFileOffset(std::ifstream& file, size_t offset, size_t toRead, void* out)
{
    // Seek to offset and read toRead # of bytes to out
    file.seekg(offset, std::ios::beg);
    file.read((char*)out, toRead);
    if (file.gcount() != toRead)
    {
        if (file.eof()) {
            std::cout << "Reached end of file. Total bytes read: " << file.gcount() << std::endl;
        }

        else if (file.fail()) {
            std::cerr << "Read operation failed due to format error." << std::endl;
            char buf[1000];
            strerror_s(buf, errno);
            std::cout << "Error: " << buf;
        }
        else if (file.bad()) {
            std::cerr << "An unrecoverable I/O error occurred." << std::endl;
        }
        else {
            std::cerr << "No bytes were read from the file for an unknown reason." << std::endl;
        }
    }
  
    // reset the iterator to the beginning
    file.seekg(0, std::ios::beg);
}


void utils::readThunkData(std::ifstream& file, size_t offset, size_t toRead, void* out)
{
    // Seek to offset and read toRead # of bytes to out
    

    file.seekg(offset, std::ios::beg);
    if (!file.eof())
    {
        file.read((char*)out, toRead);
    }

    // reset the iterator to the beginning
    file.seekg(0, std::ios::beg);
}



void utils::readStringFromFileOffset(std::ifstream& file, size_t offset, std::string& out)
{
    // Seek to offset and read _MAX_PATH bytes into a char buf, use to make a new std::string
    file.seekg(offset, std::ios::beg);
    if (!file.eof())
    {
        char buf[_MAX_PATH];
        file.read(buf, _MAX_PATH);
        out = std::string(buf);
        if (file.gcount() != _MAX_PATH)
        {
            if (file.eof()) {
                std::cout << "Reached end of file. Total bytes read: " << file.gcount() << std::endl;
            }

            else if (file.fail()) {
                std::cerr << "Read operation failed due to format error." << std::endl;
                char buf[1000];
                strerror_s(buf, errno);
                std::cout << "Error: " << buf;
            }
            else if (file.bad()) {
                std::cerr << "An unrecoverable I/O error occurred." << std::endl;
            }
            else {
                std::cerr << "No bytes were read from the file for an unknown reason." << std::endl;
            }
        }





    }
    // reset the iterator to the beginning
    file.seekg(0, std::ios::beg);

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