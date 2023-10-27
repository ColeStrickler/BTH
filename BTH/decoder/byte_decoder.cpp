#include "byte_decoder.h"

static std::string ucharToHexString(unsigned char value) {
	std::stringstream stream;
	stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(value);
	return stream.str();
}




Decoder::Decoder()
{
	
	
}

Decoder::~Decoder()
{
	
}

void Decoder::DecodeBytes(std::vector<unsigned char> bytes)
{
	m_DecodedBytes.clear();
	m_OffsetToInstIndex.clear();
	ZydisDecoder decoder;
	ZydisDecoderInit(&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
	ZydisFormatter formatter;
	ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

	int offset = 0x0;
	while(offset < bytes.size())
	{
		ZydisDecodedInstruction instruction;
		ZydisDecoderDecodeBuffer(&decoder, bytes.data() + offset, bytes.size() - offset, &instruction);

		char instructionString[256] = "";  // Adjust the buffer size as needed
		ZydisFormatterFormatInstruction(&formatter, &instruction, instructionString, sizeof(instructionString), offset);

		std::string inst(instructionString);
		DecodedInst decoded;


		for (int i = 0; i < instruction.length; i++)
		{
			m_OffsetToInstIndex[offset + i] = m_DecodedBytes.size();
		}

		if (inst != std::string("invalid"))
		{
			decoded.m_Offset = offset;
			decoded.m_DecodedInstruction = inst;
			m_DecodedBytes.push_back(decoded);
			
		}
		else
		{
			std::string invalid_bytes;
			for (int i = 0; i < instruction.length; i++)
			{
				invalid_bytes += ucharToHexString(bytes[offset + i]);
			}
			decoded.m_Offset = offset;
			decoded.m_DecodedInstruction = invalid_bytes;
			m_DecodedBytes.push_back(decoded);
			
		}
		


		// Increment the offset by the instruction length
		offset += instruction.length;
	}

}