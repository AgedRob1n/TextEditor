#pragma once
#include <string>
enum colors {
	RED = 196,
	BLACK = 16,
	WHITE = 15,
	YELLOW = 3,
	LIGHT_RED = 1,
	LIGHT_GREEN = 2,
	PINK = 5,
	LIGHT_GREY = 7,
	LIGHT_GRAY = 7,
	GREY = 8,
	GRAY = 8,
	CYAN = 51,
	PURPLE = 57,
	BLUE = 21
};

class Color {
public:
	static std::string setForeground(int colorId);
	static std::string setBackground(int colorId);
};
