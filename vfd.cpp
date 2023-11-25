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

void printBinary(int value, int numBits)
{
	for (int bit = numBits - 1; bit >= 0; bit--)
	{
		std::cout << ((value & (1 << bit)) ? '1' : '0');
	}
	std::cout << "\n";
}

void vfdSend(uint8_t cmd)
{
	delay(1);
	uint8_t lsb = sendLSB(cmd);
	wiringPiSPIDataRW(CHANNEL, &lsb, 1);
	delay(1);
}

void writeString(const char *s)
{
	for (int i = strlen(s); i >= 0; i--)
	{
		if (s[i] == '\0')
		{
			continue;
		}
		writeChar(s[i]);
	}
}

void writeChar(char c)
{
	vfdSend(0b11000000 + 0x00);
	vfdSend(asciiMap[c].second);
	vfdSend(asciiMap[c].first);
}

int lastMoveIdx = 0;

void addSpacesToRight(char* s, int targetLength) {
	int currentLength = static_cast<int>(std::strlen(s));

	if (currentLength < targetLength) {
		int spacesToAdd = targetLength - currentLength;
		std::memset(s + currentLength, ' ', spacesToAdd);
		s[targetLength] = '\0';  // Null-terminate the modified string
	}
}

void scrollLargeString(const char *s, size_t origLen, int maxIdx)
{
	char windowOutput[VFD_MAXLENGTH + 1];
	snprintf(windowOutput, sizeof(windowOutput), "%.*s", VFD_MAXLENGTH, s + lastMoveIdx);
	digitalWrite(VFD_STB, LOW);
	writeString(windowOutput);
	digitalWrite(VFD_STB, HIGH);

	lastMoveIdx++;

	// restarting rotation if max index reached
	if (lastMoveIdx >= maxIdx + 1)
	{
		lastMoveIdx = 0;
	}
}

void scrollShortString(const char *s)
{
	char mS[VFD_MAXLENGTH + 1];  // Make sure it's large enough to accommodate the content
	// Copy the content to the mutable string
	std::strcpy(mS, s);
		
	addSpacesToRight(mS, VFD_MAXLENGTH);
		
	digitalWrite(VFD_STB, LOW);
	writeString(mS);
	digitalWrite(VFD_STB, HIGH);
}

// it only displays uppercase, please see mapping above
// it will also add padding to MAXLENGTH characters with spaces
void scrollString(const char *s)
{
	size_t origLen = strlen(s);
	int maxIdx = origLen - VFD_MAXLENGTH;
	
	if (origLen > 10)
	{
		scrollLargeString(s, origLen, maxIdx);
	}
	else
	{
		scrollShortString(s);
	}
}
