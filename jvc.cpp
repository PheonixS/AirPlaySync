#include "jvc.h"

std::string getPressed(float voltage)
{
	std::string button = "";

	if (voltage <= 0.05)
	{
		button = "on";
	}

	if (voltage >= 1.1 && voltage <= 1.15)
	{
		button = "play";
	}

	if (voltage >= 3.22 && voltage <= 3.27)
	{
		button = "pause";
	}

	if (voltage >= 1.55 && voltage <= 1.7)
	{
		button = "stop";
	}

	if (voltage >= 2 && voltage <= 2.1)
	{
		button = "back";
	}

	if (voltage >= 2.5 && voltage <= 2.6)
	{
		button = "forward";
	}

	if (voltage >= 2.9 && voltage <= 3.1)
	{
		button = "dimmer";
	}

	if (voltage >= 0.55 && voltage <= 0.75)
	{
		button = "open close";
	}

	if (voltage >= 3.299)
	{
		button = "none";
	}

	return button;
}
