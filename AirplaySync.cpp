#include "AirplaySync.h"

std::string title = "";
bool displayCleared = false;
icu::Transliterator* t;

void powerOffVFD()
{
	poweredOn = false;
	digitalWrite(LED_PIN, LED_STANDBY);
	digitalWrite(POWER_SOURCE_PIN, LOW);
	digitalWrite(V3_3_CTRL, LOW);
	lastMoveIdx = 0;
}

void powerOnVFD()
{
	poweredOn = true;
	digitalWrite(POWER_SOURCE_PIN, HIGH);
	digitalWrite(V3_3_CTRL, HIGH);
	digitalWrite(LED_PIN, LED_READY);
	std::cout << "Powering on power source\n";
	// waiting for VFD driver startup
	delay(200);
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

void clearDisplay()
{
	digitalWrite(VFD_STB, LOW);
	writeString("        ");
	digitalWrite(VFD_STB, HIGH);
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
	
	t->transliterate(input);
	
	// Convert back to std::string if needed
	std::string resultStr;
	input.toUTF8String(resultStr);
	
	return resultStr;
}

void readFromPipe(const std::string& pipePath) {
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
							if (title != filteredData.c_str())
							{
								title = filteredData.c_str();
								displayCleared = false;
								std::cout << "title: " << decodedData << std::endl;
							}
						}else if (decodedcode == "conn")
						{	
							// if connection was requested
							//powering on VFD
							powerOnVFD();
							std::cout << "powering on VFD\n";
						}else if (decodedcode == "disc")
						{	
							// connection was terminated
							// powering off VFD
							powerOffVFD();
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
	title = "WAITING";
	
	//prepare transliterator
	icu::UnicodeString rules = "Any-Latin; Latin-ASCII";
	UErrorCode status = U_ZERO_ERROR;
	t = icu::Transliterator::createInstance(rules, UTRANS_FORWARD, status);

	if (U_FAILURE(status) || !t) {
		std::cerr << "Error creating transliterator. Error code: " << u_errorName(status) << std::endl;
		return 1;
	}

	
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGINT, signalHandler);
	
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
	
	const std::string pipePath = "/tmp/shairport-sync-metadata";
	std::thread readerThread(readFromPipe, pipePath);
	
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
				if (!displayCleared)
				{
					clearDisplay();
					displayCleared = true;
				}
				scrollString(title.c_str());
				
				lastMoveTime = currentTime;
			}
		}
		
		int receivedInt = parseButton(serialHandle);
				
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
				powerOnVFD();
				std::cout << "powering on VFD\n";
			}
			else if (poweredOn) // shutdown if already powered on
			{
				powerOffVFD();
				std::cout << "powering off power source and VFD\n";
			}
		}

		delay(100);
	}
	
	readerThread.join();
}
