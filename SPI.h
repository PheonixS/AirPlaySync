#pragma once
#ifndef AIRPLAY_SPI_H
#define AIRPLAY_SPI_H

#include <cstdint>
#include <iostream>

class SPI {
public:
	SPI(const char* dev_path, uint8_t mode = 0, uint8_t bits_per_word = 8, uint32_t speed = 500000);
	~SPI();

	bool openSPI();
	void closeSPI();
	bool transfer(const char* data, size_t length);

private:
	int spi_fd;
	char device_path[20];
	uint8_t mode;
	uint8_t bits_per_word;
	uint32_t speed;
};

#endif
