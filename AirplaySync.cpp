#include "AirplaySync.h"

int main()
{
	if (wiringPiSetup() == -1) {
		std::cerr << "Error initializing WiringPi." << std::endl;
		return 1;
	}
	
	pinMode(ADC_PIN_ARRAY, INPUT);
	
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)

	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LED_STANDBY);

	pinMode(POWER_SOURCE_PIN, OUTPUT);
	digitalWrite(POWER_SOURCE_PIN, LOW);
	pinMode(V3_3_CTRL, OUTPUT);
	digitalWrite(V3_3_CTRL, LOW);

	pinMode(VFD_STB, OUTPUT);
	digitalWrite(VFD_STB, HIGH);
	
	// Use Arduino pro mini as ADC :D

	int serialHandle = serialOpen("/dev/serial0", 115200);
	std::cout << "open serial handle at: " << serialHandle << std::endl;
	if (serialHandle == -1) {
		std::cout << "failed to configure serial port" << std::endl;
		// Handle SPI setup error
		return 1;
	}

	int spiHandle = wiringPiSPISetup(CHANNEL, SPEED);
	if (spiHandle == -1) {
		std::cout << "failed to configure SPI" << std::endl;
		// Handle SPI setup error
		return 1;
	}
	
	std::cout << "Hello from airplay sidecar" << std::endl;

	static int dataIndex = 0;
	unsigned long lastMoveTime = 0;
	
	for (;;)
	{
		unsigned long currentTime = millis();
	
		if (poweredOn)
		{
			if (currentTime - lastMoveTime >= scrollDelayMs)
			{
				scrollString("OLGA IS GOOD WIFE");

				lastMoveTime = currentTime;
			}
		}
		
		char receivedString[50]; // Buffer to store received characters
		int charIndex = 0;
		int receivedInt = -1;

		// Check if data is available in the serial buffer
		while (serialDataAvail(serialHandle) > 0) {
			// Read a character from the serial port
			char incomingChar = serialGetchar(serialHandle);

			// Check if the received character is a newline character
			if (incomingChar == '\n') {
				// Null-terminate the string
				receivedString[charIndex] = '\0';
				receivedInt = atoi(receivedString);
				
				// Reset the buffer index
				charIndex = 0;
			}
			else {
				// Store the character in the buffer
				receivedString[charIndex++] = incomingChar;

				// Check if the buffer is full to avoid overflow
				if (charIndex >= sizeof(receivedString) - 1) {
					charIndex = 0; // Reset the buffer index
				}
			}
		}
		
				
		if (receivedInt == -1)
		{
			delay(100);
			continue;
		}
		
		std::string button = getPressed(receivedInt);
		if (button == "")
		{		
			delay(100);
			continue;
		}

		if (button == "on")
		{
			delay(50);
			if (button != "on")
			{
				continue;
			}

			if (!poweredOn)
			{
				poweredOn = true;
				digitalWrite(POWER_SOURCE_PIN, HIGH);
				digitalWrite(V3_3_CTRL, HIGH);
				digitalWrite(LED_PIN, LED_READY);
				std::cout << "Powering on power source\n";
				// waiting for VFD driver startup
				delay(200);

				// clear RAM
				for (size_t i = 0; i < 0x23; i++)
				{
					// command 3 - clear RAM
					digitalWrite(VFD_STB, LOW);
					vfdSend(0b11000000);
					vfdSend(0x00);
					vfdSend(0x00);
					digitalWrite(VFD_STB, HIGH);
				}

				digitalWrite(VFD_STB, LOW);
				// command 1 - 10 digits 18 segments
				vfdSend(0b00000110);
				digitalWrite(VFD_STB, HIGH);

				delay(1);
				// command 2 - normal mode
				digitalWrite(VFD_STB, LOW);
				vfdSend(0b01000000);
				digitalWrite(VFD_STB, HIGH);
				delay(1);

				// command 4 turn on display
				digitalWrite(VFD_STB, LOW);
				vfdSend(0b10001111);
				digitalWrite(VFD_STB, HIGH);

				std::cout << "powering on VFD\n";
			}
			else if (poweredOn) // shutdown if already powered on
			{
				poweredOn = false;
				digitalWrite(LED_PIN, LED_STANDBY);
				digitalWrite(POWER_SOURCE_PIN, LOW);
				digitalWrite(V3_3_CTRL, LOW);
				lastMoveIdx = 0;
				std::cout << "powering off power source and VFD\n";
			}
		}

		delay(100);
	}
}
