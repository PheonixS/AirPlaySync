// inspired by https://github.com/jcj83429/jvc_compulink
// and http://www.johnwillis.com/2018/09/av-compu-link-unmodulated-ir-daisy.html

#include "compulink.h"

void wordEnd()
{
	digitalWrite(COMPULINK, HIGH);
	delay(5);
	digitalWrite(COMPULINK, LOW);
}

void writeHigh()
{
	digitalWrite(COMPULINK, HIGH);
	delay(5);
	digitalWrite(COMPULINK, LOW);
	delay(15);
}

void writeLow()
{
	digitalWrite(COMPULINK, HIGH);
	delay(5);
	digitalWrite(COMPULINK, LOW);
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
