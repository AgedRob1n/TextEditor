
#include "Color.hpp"
#include <charconv>
#include <string>

using namespace std::string_literals;

std::string Color::setBackground(int colorId) {
	std::string message = "\x1b[48;5;"s + std::to_string(colorId) + "m"s;
	return message;
}

std::string Color::setForeground(int colorId) {
	std::string message = "\x1b[38;5;"s + std::to_string(colorId) + "m"s;
	return message;
}

