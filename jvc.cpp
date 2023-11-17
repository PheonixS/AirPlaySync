#include "jvc.h"

std::string getPressed(int val)
{
	std::string button = "";

	if (val <= 10)
	{
		button = "on";
	}
	else if (val <= 260)
	{
		button = "open/close";
	}else if (val <= 440)
	{
		button = "play";
	}else if (val <= 640)
	{
		button = "stop";
	}else if (val <= 820)
	{
		button = "back";
	}else if (val <= 990)
	{
		button = "forward";
	}

	return button;
}
