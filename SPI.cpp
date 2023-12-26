#include "SPI.h"
#include <cstring>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

SPI::SPI(const char* dev_path, uint8_t mode, uint8_t bits_per_word, uint32_t speed)
	: spi_fd(-1)
	, mode(mode)
	, bits_per_word(bits_per_word)
	, speed(speed) {
	strncpy(device_path, dev_path, sizeof(device_path));
}

SPI::~SPI() {
	closeSPI();
}

bool SPI::openSPI() {
	spi_fd = open(device_path, O_RDWR);
	if (spi_fd < 0) {
		std::cerr << "Error opening SPI device" << std::endl;
		return false;
	}

	if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) == -1 ||
	    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) == -1 ||
	    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
		std::cerr << "Error configuring SPI" << std::endl;
		closeSPI();
		return false;
	}

	return true;
}

void SPI::closeSPI() {
	if (spi_fd >= 0) {
		close(spi_fd);
		spi_fd = -1;
	}
}

bool SPI::transfer(const char* data, size_t length) {
	if (spi_fd < 0) {
		std::cerr << "SPI device not opened" << std::endl;
		return false;
	}

	struct spi_ioc_transfer spi_transfer = {
		.tx_buf = (unsigned long)data,
		.len = length,
		.speed_hz = speed,
		.bits_per_word = bits_per_word,
	};

	if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_transfer) == -1) {
		std::cerr << "Error during SPI transfer" << std::endl;
		return false;
	}

	return true;
}
