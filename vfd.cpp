#include "vfd.h"

// higher bit - *sec* on first digit
//  0b10000000 - MIN on 3rd digit| 0b01000000 - colon
//  0b01000000 - colon on 5th digit

// Create a std::map for the key-value pairs
// Head of segment stored in first byte
// Tail of the element stored in second byte
std::map<char, TwoBytes> asciiMap = {
	{' ', {0b00000000, 0b00000000}},
	{'A', {0b00110001, 0b11000110}},
	{'B', {0b00100101, 0b10010101}},
	{'C', {0b00110000, 0b00000011}},
	{'D', {0b00100101, 0b00010101}},
	{'E', {0b00110000, 0b11000011}},
	{'F', {0b00110000, 0b11000010}},
	{'G', {0b00110000, 0b10000111}},
	{'H', {0b00010001, 0b11000110}},
	{'I', {0b00100100, 0b00010001}},
	{'J', {0b00000001, 0b00000111}},
	{'K', {0b00010010, 0b01100010}},
	{'L', {0b00010000, 0b00000011}},
	{'M', {0b00011011, 0b00000110}},
	{'N', {0b00011001, 0b00100110}},
	{'O', {0b00110001, 0b00000111}},
	{'P', {0b00110001, 0b11000010}},
	{'Q', {0b00110001, 0b00100111}},
	{'R', {0b00110001, 0b11100010}},
	{'S', {0b00110000, 0b11000101}},
	{'T', {0b00110101, 0b00010001}},
	{'U', {0b00010001, 0b00000111}},
	{'V', {0b00010010, 0b00001010}},
	{'W', {0b00010001, 0b00101110}},
	{'X', {0b00001010, 0b00101000}},
	{'Y', {0b00001010, 0b00010000}},
	{'Z', {0b00100010, 0b11001001}},

	{'1', {0b00000011, 0b00000100}},
	{'2', {0b00100001, 0b11000011}},
	{'3', {0b00100001, 0b11000101}},
	{'4', {0b00010001, 0b11000100}},
	{'5', {0b00110000, 0b11000101}},
	{'6', {0b00110000, 0b11000111}},
	{'7', {0b00100001, 0b00000100}},
	{'8', {0b00110001, 0b11000111}},
	{'9', {0b00110001, 0b11000101}},
	{'0', {0b00110011, 0b00001111}},

	{'+', {0b00000100, 0b11010000}},
	{'-', {0b00000000, 0b11000000}},
	{'/', {0b00000010, 0b00001000}},
	{'*', {0b00001110, 0b00111000}},
	{'\'', {0b00000100, 0b00000000}},
	{'"',  {0b00000100, 0b00000000}},
	{'.', {0b00000000, 0b01000000}},
	{'(', {0b00001000, 0b00000000}},
	{')', {0b00000010, 0b00000000}},
	{':', {0b00000010, 0b00001000}},
};


uint8_t sendLSB(uint8_t orig)
{
	uint8_t reversedValue = 0;

	// Reverse the bits
	for (int i = 0; i < 8; i++)
	{
		if (orig & (1 << i))
		{
			reversedValue |= (1 << (7 - i));
		}
	}
	return reversedValue;
}

void vfdSend(SPI spi, uint8_t cmd)
{
	if (spi.openSPI()) {
		const char data[] = { cmd };
		spi.transfer(data, sizeof(data));
		spi.closeSPI();
	}
}

void writeBuffered(SPI spi, std::string buf)
{
	char outputBuffer[VFD_MAXLENGTH * 3];
	u_int8_t i = buf.length() * 3 - 3;
	
	for (char c : buf) {
		outputBuffer[i] = sendLSB(0b11000000);
		outputBuffer[i + 1] = sendLSB(asciiMap[c].second);
		outputBuffer[i + 2] = sendLSB(asciiMap[c].first);
		i-=3;
	}
	
	if (spi.openSPI()) {
		spi.transfer(outputBuffer, buf.length()*3);
		spi.closeSPI();
	}
}

void copySegments(const std::string& inputString, std::string& outputString, size_t start, size_t end) {
	if (start >= inputString.length() || end > inputString.length() || start >= end) {
		std::cerr << "Invalid start or end indices" << std::endl;
		return;
	}

	outputString = inputString.substr(start, end - start);
}

int lastMoveIdx = 0;

// it only displays uppercase, please see mapping above
void scrollString(SPI spi, std::string s)
{	
	// restart with cycle
	if (lastMoveIdx + VFD_MAXLENGTH > s.length())
	{
		lastMoveIdx = 0;
	}
	
	size_t start = lastMoveIdx;	
	size_t end = lastMoveIdx + VFD_MAXLENGTH;	
	std::string output;
	
	if (s.length() > VFD_MAXLENGTH)
	{
		copySegments(s, output, start, end);
	}
	else
	{
		output = s; //TODO: Optimize to assign it once?
	
		// add padding left with spaces
		size_t paddingSize = VFD_MAXLENGTH - output.length();
		if (paddingSize > 0) {
			output.insert(0, paddingSize, ' '); // Insert spaces at the beginning
		}
	}
	
	writeBuffered(spi, output);
	
	lastMoveIdx += 1;
}
