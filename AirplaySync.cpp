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

	int spiHandle = wiringPiSPISetup(CHANNEL, SPEED);
	if (spiHandle == -1) {
		// Handle SPI setup error
		return 1;
	}
	
	std::cout << "Hello from airplay sidecar";

	for (;;)
	{
		
		unsigned long currentTime = millis();

		int sensorValue = analogRead(ADC_PIN_ARRAY);
		std::cout << sensorValue << std::endl;
		float voltage = sensorValue * (3.3 / 1023.0);
		std::string button = getPressed(voltage);

		if (button == "keon")
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

		if (poweredOn)
		{
			if (currentTime - lastMoveTime >= scrollDelayMs)
			{
				scrollString("OLGA IS GOOD WIFE");

				lastMoveTime = currentTime;
			}
		}

		delay(100);
	}
}
