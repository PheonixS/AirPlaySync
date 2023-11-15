// inspired by https://github.com/jcj83429/jvc_compulink
// and http://www.johnwillis.com/2018/09/av-compu-link-unmodulated-ir-daisy.html

#include "compulink.h"

void wordEnd()
{
	std::cout << "END\n";
	digitalWrite(D1, HIGH);
	delay(5);
	digitalWrite(D1, LOW);
}

void writeHigh()
{
	std::cout << "1";
	digitalWrite(D1, HIGH);
	delay(5);
	digitalWrite(D1, LOW);
	delay(15);
}

void writeLow()
{
	std::cout << "0";
	digitalWrite(D1, HIGH);
	delay(5);
	digitalWrite(D1, LOW);
	delay(5);
}

void sendCommand(unsigned char hexCode) {
	std::printf("0x%x\n", hexCode);
	for (int bitPosition = 7; bitPosition >= 0; bitPosition--)
	{
		int bitValue = (hexCode >> bitPosition) & 1;

		if (bitValue == 0)
		{
			writeLow();
		}
		else
		{
			writeHigh();
		}
	}
	wordEnd();
}
