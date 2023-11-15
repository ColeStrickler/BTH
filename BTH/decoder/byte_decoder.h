#pragma once

#include <vector>
#include <iostream>
#include <Zydis/Zydis.h>
#include "../utils/utils.h"
#include "../pe/pe.h"
#include <map>

struct DecodedInst
{
	int m_Offset;
	int m_OpcodeSize;
	std::string m_DecodedInstruction;
};


class Decoder
{
public:
	Decoder();
	~Decoder();


	void DecodeBytes(std::vector<unsigned char> bytes, PEDisector* pe);
	// offset : instruction
	std::vector<DecodedInst> m_DecodedBytes;
	std::map<int, int> m_OffsetToInstIndex;
private:
		
	
};

