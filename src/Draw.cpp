#include "Draw.hpp"
#include "Global.hpp"
#include <iostream>
#include <ostream>
#include <string>

using std::string;
using namespace std::string_literals;

void Draw::refreshScreen() {
	buffer.appendBuffer("\x1b[?25l");
	buffer.appendBuffer("\x1b[H");
	drawRows();
	std::string line;
	if (config.editorMode == NORMAL || config.editorMode == INSERT) {
		line = "\x1b[" + std::to_string(config.cursorY) + ";" + std::to_string(config.cursorX) + "H";
	} else {
		line = "\x1b[" + std::to_string(config.screenSize.ws_row) + ";" + std::to_string(2) + "H";
	}
	buffer.appendBuffer(line.c_str());
	buffer.appendBuffer("\x1b[?25h");

	std::cout << buffer.getBuffer() << std::flush;

	buffer.freeBuffer();
}

void Draw::drawRows() {
	for (int i = 0; i < config.screenSize.ws_row; i++) {
		buffer.appendBuffer(" ~");
		if (i >= config.numRows) {
			if (i == config.screenSize.ws_row / 3 && config.numRows == 0) {
				string message = "Welcome to ___ version 0.0.1";
				buffer.appendBuffer(std::string("\x1b[" + std::to_string(config.screenSize.ws_col / 2 - (message.length() / 2)) + string("C")));
				buffer.appendBuffer(message);
			}
		} else {
			buffer.appendBuffer("\x1b[K");
			buffer.appendBuffer(config.row[i + config.yOffset].text);
		}

		if (i < config.screenSize.ws_row - 1) buffer.appendBuffer("\r\n");
		if (i == config.screenSize.ws_row - 1) {
			buffer.appendBuffer("\x1b[2K");
			string cursor = "["s + std::to_string(config.cursorY + config.yOffset) + ","s + std::to_string(config.cursorX - 2) + "]"s;
			buffer.appendBuffer("\x1b[G");
			if (config.editorMode == COMMAND) {
				buffer.appendBuffer(":");
			}
			buffer.appendBuffer(string("\x1b["s + std::to_string(config.screenSize.ws_col / 2 - 3) + "G"));
			buffer.appendBuffer(debugMessage);
			buffer.appendBuffer("\x1b[" + std::to_string(config.screenSize.ws_col - config.fileName.length() - cursor.length()) + "G");
			buffer.appendBuffer(config.fileName);
			buffer.appendBuffer(cursor);
		}
	}
}

