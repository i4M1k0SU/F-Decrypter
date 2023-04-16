#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include "key.h"

uint8_t* decrypt(uint8_t* destBuffer, uint8_t* srcBuffer, uint16_t keyOffset, size_t srcSize, uint8_t fileFormat)
{
	if (srcSize == 0 || fileFormat > 3)
	{
		return 0;
	}

	keyOffset %= 0x400;

	for (size_t i = 0; i < srcSize; i++)
	{
		destBuffer[i] = srcBuffer[i] - keyTable2[keyOffset];
		keyOffset++;
		if (keyOffset >= 0x400)
		{
			keyOffset = 0;
		}
	}

	return destBuffer;
}

int main(int argc, char* argv[])
{
	std::string inputFilePath, outputFilePath;

	if (argc == 3)
	{
		inputFilePath = argv[argc - 2];
		outputFilePath = argv[argc - 1];
	}
	if (argc == 2)
	{
		inputFilePath = argv[argc - 1];
	}

	if (inputFilePath.empty())
	{
		std::cerr << "Invalid arguments.\nUsage: " << argv[0] << " INPUT_FILE [OUTPUT_FILE]" << std::endl;
		return 1;
	}

	if (outputFilePath.empty())
	{
		outputFilePath = inputFilePath + "_dec";
	}

	std::cout << "Open input file: " << inputFilePath << std::endl;
	std::ifstream inputFile(inputFilePath, std::ios::binary);

	if (!inputFile)
	{
		std::cerr << "Could not open input file: " << inputFilePath << std::endl;
		return 1;
	}

	std::vector<uint8_t> srcBuffer{ std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>() };
	inputFile.close();

	if (srcBuffer.empty())
	{
		std::cerr << "Error: unable to read input file." << std::endl;
		return 1;
	}

	size_t fileSize = srcBuffer.size();
	size_t payloadSize = fileSize - 4;
	std::vector<uint8_t> destBuffer(payloadSize, 0x00);
	uint8_t* result = decrypt(
		destBuffer.data(),
		srcBuffer.data() + 4,
		((payloadSize >> 24 & 0xff) + (payloadSize >> 16 & 0xff) + (payloadSize >> 8 & 0xff) + (payloadSize & 0xff)) * 2,
		fileSize,
		srcBuffer[3]
	);

	if (result != destBuffer.data())
	{
		std::cerr << "Decryption failed." << std::endl;
		return 1;
	}
	std::cout << "Successfully decrypted." << std::endl;

	std::ofstream outputFile(outputFilePath, std::ios::binary);

	if (!outputFile)
	{
		std::cerr << "Could not open output file: " << outputFilePath << std::endl;
		return 1;
	}

	outputFile.write(reinterpret_cast<const char*>(destBuffer.data()), destBuffer.size());
	outputFile.close();

	std::cout << "Processed data successfully written to: " << outputFilePath << std::endl;

	return 0;
}
