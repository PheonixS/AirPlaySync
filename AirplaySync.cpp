#include "AirplaySync.h"
#include <wiringPiI2C.h>

std::string title = "";
icu::Transliterator* icuTrans;

void syncDisplay(SPI spi)
{
	// command 4 turn off display
	vfdSend(spi, sendLSB(0b10000111));
	
	// clear RAM
	for (size_t i = 0; i < 0xFF; i++)
	{
		if (spi.openSPI()) {
			const char data[] = { sendLSB(0b11000000 + i), 0x00, 0x00 };
			spi.transfer(data, sizeof(data));
			spi.closeSPI();
		}
	}
	
	// command 1 - 10 digits 18 segments
	vfdSend(spi, sendLSB(0b00000110));
	
	// command 2 - normal mode
	vfdSend(spi, sendLSB(0b01000000));
	
	// command 4 turn on display
	vfdSend(spi, sendLSB(0b10001111));
}


void powerOffVFD()
{
	poweredOn = false;
	digitalWrite(LED_PIN, LED_STANDBY);
	digitalWrite(POWER_SOURCE_PIN, LOW);
	digitalWrite(V3_3_CTRL, LOW);
	lastMoveIdx = 0;
	std::cout << "Powering off power source" << std::endl;
}

void powerOnVFD()
{
	digitalWrite(POWER_SOURCE_PIN, HIGH);
	digitalWrite(V3_3_CTRL, HIGH);
	digitalWrite(LED_PIN, LED_READY);
	std::cout << "Powering on power source" << std::endl;
	// waiting for VFD driver startup
	delay(200);
	poweredOn = true;
}

char* toUpperCase(const char* str) {
	size_t length = std::strlen(str);
	char* result = new char[length + 1];  // +1 for the null terminator

	for (size_t i = 0; i < length; ++i) {
		result[i] = std::toupper(str[i]);
	}

	result[length] = '\0';  // Null-terminate the new string
	return result;
}

std::string filterString(const char* input, const std::map<char, TwoBytes>& asciiMap) {
	std::string result;

	for (; *input != '\0'; ++input) {
		char currentChar = *input;

		// Check if the character is in the map
		if (asciiMap.find(currentChar) != asciiMap.end()) {
			result += currentChar;
		}
	}

	return result;
}


std::string hexToString(const std::string& hex) {
	std::string result;
	for (size_t i = 0; i < hex.length(); i += 2) {
		std::string byteString = hex.substr(i, 2);
		char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
		result += byte;
	}
	return result;
}

std::string transliterate(const std::string& text) {	
	// Convert std::string to icu::UnicodeString
	icu::UnicodeString input;
	input.setTo(text.c_str());
	
	icuTrans->transliterate(input);
	
	// Convert back to std::string if needed
	std::string resultStr;
	input.toUTF8String(resultStr);
	
	return resultStr;
}

const int device_address = 0x49;
const double samplingInterval = 1.0 / 1600.0; // Sampling interval in seconds
const char* commandStop = "docker exec shairport dbus-send --system --type=method_call --dest=org.gnome.ShairportSync '/org/gnome/ShairportSync' org.gnome.ShairportSync.RemoteControl.Pause";

void readButtonArray()
{
	int fd = wiringPiI2CSetup(device_address);

	if (fd == -1) {
		std::cout << "Failed to init I2C communication.\n";
		exit(1);
	}

	std::cout << "I2C communication successfully setup.\n";

	int config_value = 0x80E0; // Continuous conversion mode
	// Write to configuration register
	wiringPiI2CWriteReg16(fd, 0x01, config_value);

	useconds_t delay = static_cast<useconds_t>(samplingInterval * 1000000);
	
	// The device is now set to continuous conversion mode
	// You can now read the conversion result in a loop
	while (true) {
		uint16_t result = wiringPiI2CReadReg16(fd, 0x00);
		if (result == 0)
		{
			usleep(delay);
			continue;
		}
		
		if (result < 10)
		{
			int exitCode = system(commandStop);
			std::cout << "Conversion result: " << result << "; ex=" << exitCode << std::endl;
		}
		
		usleep(delay);
	}
}

void readFromPipe(SPI spi, const std::string& pipePath) {
	std::ifstream pipe(pipePath);
	if (!pipe.is_open()) {
		std::cerr << "Error opening pipe." << std::endl;
		return;
	}

	std::string xmlString;
	std::string line;

	while (std::getline(pipe, line)) {
		xmlString += line;

		// Assume each XML message ends with </item>
		size_t pos = xmlString.find("</item>");

		// Check if </item> is found, indicating the end of a message
		if (pos != std::string::npos) {
			// Extract the XML message
			std::string message = xmlString.substr(0, pos + 7); // +7 to include </item>
            
			// Process the XML message
			pugi::xml_document doc;
			pugi::xml_parse_result result = doc.load_string(message.c_str());

			if (result) {
				// Successfully parsed XML
				pugi::xml_node itemNode = doc.child("item");
				if (itemNode) {
					std::string type = itemNode.child("type").text().as_string();
					std::string code = itemNode.child("code").text().as_string();
					std::string data = itemNode.child("data").text().as_string();							
					
					std::string decodedtype = hexToString(type);
					std::string decodedcode = hexToString(code);
									
					if (data != "")
					{
						// Title
						if (decodedcode == "minm")
						{
							std::string decodedData = transliterate(base64::from_base64(data));							
							const char* upperCaseData = toUpperCase(decodedData.c_str());
							std::string filteredData = filterString(upperCaseData, asciiMap);
							
							if (title != filteredData)
							{
								title = filteredData;
								lastMoveIdx = 0;
								std::cout << "title: " << decodedData << std::endl;
							}
						}else if (decodedcode == "conn")
						{	
							// if connection was requested
							//powering on VFD
							powerOnVFD();
							syncDisplay(spi);
							poweredOn = true;
							std::cout << "powering on VFD\n";
						}else if (decodedcode == "disc")
						{	
							// connection was terminated
							// powering off VFD
							powerOffVFD();
							poweredOn = false;
							std::cout << "powering off VFD\n";
						}
					}
				}
				else {
					std::cerr << "Error: 'item' node not found." << std::endl;
				}
			}
			else {
				std::cerr << "Error parsing XML: " << result.description() << std::endl;
			}

			// Remove the processed XML message from xmlString
			xmlString.erase(0, pos + 7);
		}
	}
	
	pipe.close();
}

// Signal handler function
void signalHandler(int signum) {
	std::cout << "Received signal: " << signum << ". Cleaning up and exiting..." << std::endl;

	powerOffVFD();

	// Terminate the program
	exit(signum);
}

int parseButton(int serialHandle)
{
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
	return receivedInt;
}

int main()
{
	title = "...WAITING...";
	const std::string pipePath = "/tmp/shairport-sync-metadata";
	
	SPI spi("/dev/spidev0.0");
	
	//prepare transliterator
	icu::UnicodeString rules = "Any-Latin; Latin-ASCII";
	UErrorCode status = U_ZERO_ERROR;
	icuTrans = icu::Transliterator::createInstance(rules, UTRANS_FORWARD, status);

	if (U_FAILURE(status) || !icuTrans) {
		std::cerr << "Error creating transliterator. Error code: " << u_errorName(status) << std::endl;
		return 1;
	}
	
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGINT, signalHandler);
	
	if (wiringPiSetup() == -1) {
		std::cerr << "Error initializing WiringPi." << std::endl;
		return 1;
	}
	
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, LED_STANDBY);

	pinMode(POWER_SOURCE_PIN, OUTPUT);
	digitalWrite(POWER_SOURCE_PIN, LOW);
	pinMode(V3_3_CTRL, OUTPUT);
	digitalWrite(V3_3_CTRL, LOW);
	
	std::thread readerThread(readFromPipe, spi, pipePath);
	std::thread buttonArray(readButtonArray);
	
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
				
				scrollString(spi, title);
				lastMoveTime = currentTime;
			}
		}
		
	}
	
	readerThread.join();
	buttonArray.join();
}
