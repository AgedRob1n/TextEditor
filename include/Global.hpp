#pragma once
#include <Buffer.hpp>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

#define NORMAL 0
#define INSERT 1
#define COMMAND 2

struct editorRow {
	std::string text;
	int length;
};

inline std::string debugMessage = "";

struct editorConfig {
	int cursorY = 1, xOffset = 3, numRows = 0, yOffset = 0, editorMode = NORMAL, cursorX = xOffset, desiredX = cursorX;
	bool fileOpen;
	struct winsize screenSize;
	std::vector<editorRow> row;
	std::string fileName;
};

inline editorConfig state;
inline Buffer buffer;

