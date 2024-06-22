#include "Draw.hpp"
#include "Color.hpp"
#include "Global.hpp"
#include <iostream>
#include <ostream>
#include <string>

using std::string;
using namespace std::string_literals;

void Draw::refreshScreen() {
	buffer.appendBuffer("\x1b[?25l");
	buffer.appendBuffer("\x1b[H");
	drawRows(true);
	std::string line;
	if (state.editorMode == NORMAL || state.editorMode == INSERT) {
		line = "\x1b[" + std::to_string(state.cursorY) + ";" + std::to_string(state.cursorX) + "H";
	} else {
		line = "\x1b[" + std::to_string(state.screenSize.ws_row) + ";" + std::to_string(2) + "H";
	}
	buffer.appendBuffer(line.c_str());
	buffer.appendBuffer("\x1b[?25h");

	std::cout << buffer.getBuffer() << std::flush;

	buffer.freeBuffer();
}

int Draw::getDigits(int input) {
	int numDigits = 0;
	if (input == 0) return 0;
	while (input /= 10) {
		numDigits++;
	}
	return numDigits;
}

string drawLineNumbers(int i) {
	string lineNum;
	int spaces = Draw::getDigits(state.numRows) - Draw::getDigits(i + state.yOffset + 1);
	for (int j = 0; j < spaces; j++) lineNum += " ";
	lineNum += std::to_string(i + state.yOffset + 1);
	return lineNum;
}

void Draw::drawRows(bool showLineNumbers) {
	for (int i = 0; i < state.screenSize.ws_row; i++) {
		buffer.appendBuffer(Color::setForeground(CYAN) + Color::setBackground(GRAY));
		if (showLineNumbers && state.fileOpen && i < state.numRows) {
			buffer.appendBuffer(drawLineNumbers(i));
			buffer.appendBuffer(Color::setForeground(WHITE) + Color::setBackground(BLACK));
		} else {
			buffer.appendBuffer("~");
			buffer.appendBuffer(Color::setBackground(BLACK));
			for (int j = 0; j < state.screenSize.ws_col - 1; j++) buffer.appendBuffer(" "); 
		}
		if (i >= state.numRows) {
			if (i == state.screenSize.ws_row / 3 && state.numRows == 0) {
				string message = "Welcome to ___ version 0.0.1";
				buffer.appendBuffer(std::string("\x1b[" + std::to_string(state.screenSize.ws_col / 2 - (message.length() / 2)) + string("C")));
				buffer.appendBuffer(message);
			}
		} else {
			buffer.appendBuffer("\x1b[K");
			buffer.appendBuffer(state.row[i + state.yOffset].text);
		}

		if (i < state.screenSize.ws_row - 1) buffer.appendBuffer("\r\n");
		if (i == state.screenSize.ws_row - 1) {
			buffer.appendBuffer("\x1b[2K");
			string cursor = "["s + std::to_string(state.cursorY + state.yOffset) + ","s + std::to_string(state.cursorX - 2) + "]"s;
			buffer.appendBuffer("\x1b[G");
			if (state.editorMode == COMMAND) {
				buffer.appendBuffer(":");
			}
			buffer.appendBuffer(string("\x1b["s + std::to_string(state.screenSize.ws_col / 2 - 3) + "G"));
			buffer.appendBuffer(debugMessage);
			buffer.appendBuffer("\x1b[" + std::to_string(state.screenSize.ws_col - state.fileName.length() - cursor.length()) + "G");
			buffer.appendBuffer(state.fileName);
			buffer.appendBuffer(cursor);
		}
	}
}

