#include "App.hpp"
#include "Global.hpp"
#include "Input.hpp"
#include <fstream>
#include <iostream>
#include <string>

using std::string;
using namespace std::string_literals;

#define CTRL_KEY(key) ((key) & 0x1f)

enum keys {
	UP_KEY = 'k',
	DOWN_KEY = 'j',
	LEFT_KEY = 'h',
	RIGHT_KEY = 'l',
	PAGE_UP,
	PAGE_DOWN,
	HOME_KEY,
	END_KEY,
	TEST_KEY,
	CONTROL_UP,
	CONTROL_DOWN,
	CONTROL_LEFT,
	CONTROL_RIGHT,
	ESCAPE_KEY = 27
};

struct WriteBuffer {
	string buffer;
	int length;
};

void App::exitProgram(const char *error) {
	write(STDOUT_FILENO, "\x1b[?1049l", 9);
	perror(error);
	exit(1);
}

void drawRows() {
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

void refreshScreen() {
	buffer.appendBuffer("\x1b[?25l");
	buffer.appendBuffer("\x1b[H");
	drawRows();
	string line;
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

void openEditor(const string& fileLocation) {

	string text;
	std::ifstream file(fileLocation);
	if (file.fail()) App::exitProgram("File does not exist");

	while (std::getline(file, text)) {
		//Replace tabs with spaces
		const char *find = "\t";
		string replace = "   ";
		if (text.find("\t") != string::npos) {
			while (text.find("\t") != string::npos) {
				text.replace(text.find(find), 1, replace);
			}
		}

		//Temporarily just clip the line until either scrolling or nvim style handling is implemented.
		if (text.length() > config.screenSize.ws_col) text = text.substr(0, config.screenSize.ws_col - 3);
		config.row.push_back({text, static_cast<int>(text.size())});
		config.numRows++;
	}
	file.close();
	debugMessage = std::to_string(sizeof(config.row) * config.row.capacity());
}

/**
* Moves the cursor eaither backwards or forward by one word depending on the direction
* If direction is equal to -1 then it will go backwards, if it is one then it will move forwards.
*/
App::App(int argc, char *argv[]) {init(argc, argv);}
Input input;

void App::init(const int argc, char *argv[]) {
  if (argc > 1) {
    openEditor(argv[1]);
    config.fileName = argv[1];
    config.fileOpen = true;
  } else {
    config.row.push_back({"", 0});
  }
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &config.screenSize))
    App::exitProgram("Get Window Size");
	input.enableRawMode();
}


void App::update() {
  refreshScreen();
	input.processKeys();
}

