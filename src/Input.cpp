#include "Input.hpp"
#include "Global.hpp"
#include "App.hpp"
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

#define CTRL_KEY(key) ((key) & 0x1f)

struct termios originalSettings;

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

void Input::enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &originalSettings) == -1)
		App::exitProgram("tcgetattr");

	atexit([]() {tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings);});
	write(STDOUT_FILENO, "\x1b[?1049h", 9);

	m_settings = m_originalSettings;

	//ECHO stops ahowing the chararcter typed in, ICANON reads input by char, ISIG disables ctrl-z and ctrl-c from stopping the program
	//IEXTEN diables the ctrl-v command, last two are misc. flags
	m_settings.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN | BRKINT | ICRNL );
	//IXON disables the ctrl-s and ctrl-q commands, ICRNL seperates some keys like enter from ctrl-j
	m_settings.c_iflag &= ~(IXON | ICRNL);
	//Get rid of output processing
	m_settings.c_oflag &= ~(OPOST);
	m_settings.c_cflag |= (CS8);

	m_settings.c_cc[VMIN] = 0;
	m_settings.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &m_settings) == -1)
		App::exitProgram("tcsetattr");
}

char Input::readKey() {
	char key;
	int status;
	while ((status = read(STDIN_FILENO, &key, 1)) != 1) {
		if (status == -1 && errno != EAGAIN) App::exitProgram("read");
	}
	if (key == 27) {
		char seq[3];

		if (read(STDOUT_FILENO, &seq[0], 1) == -1) return ESCAPE_KEY;
		if (read(STDOUT_FILENO, &seq[1], 1) == -1) return ESCAPE_KEY;
		if (seq[1] >= '0' && seq[1] <= '9') {
			if (read(STDOUT_FILENO, &seq[2], 1) == -1) return ESCAPE_KEY;
			if (seq[2] == '~') {
				switch (seq[1]) {
					case '5': return PAGE_UP;
					case '6': return PAGE_DOWN;
					default: break;
				}
			}
		}

		if (seq[0] == '[') {
			switch (seq[1]) {
				case 'C':
					return RIGHT_KEY;
				case 'D':
					return LEFT_KEY;
				case 'A':
					return UP_KEY;
				case 'B':
					return DOWN_KEY;
				case 'H': 
					return HOME_KEY;
				case 'F':
					return END_KEY;
				case '1':
					char chars[3];
					read(STDOUT_FILENO, &chars[0], 1);
					read(STDOUT_FILENO, &chars[0], 1);
					switch (chars[0]) {
						case 'A':
							return CONTROL_UP;
						case 'B':
							return CONTROL_DOWN;
						case 'C':
							return CONTROL_RIGHT;
						case 'D':
							return CONTROL_LEFT;
					}
				break;
				default:
					break;
			}
		}

	}
	return key;
}

void Input::processKeys() {
	char key = readKey();
	switch (key) {
		case CTRL_KEY('q'):
			tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings);
			std::cout << "\x1b[?25h" << std::flush;
			std::cout << "\x1b[?1049l" << std::flush;
			exit(0);
			break;
		case UP_KEY:
			//Move cursor up//Move cursor up.
			if (config.cursorY > 1 && config.fileOpen) {
				config.cursorY--;
			} else if (config.cursorY + config.yOffset > 1) {
				config.yOffset--;
			}
			if (config.desiredX > config.row[(config.cursorY + config.yOffset) - 1].length) {
				config.cursorX = config.row[(config.cursorY + config.yOffset) - 1].length + config.xOffset;
			} else {
				config.cursorX = config.desiredX;
			}
			break;
		case DOWN_KEY:
			if (config.row[0].text == "" && config.numRows == 1) break;
			if (config.numRows > config.screenSize.ws_row && config.cursorY < config.screenSize.ws_row - 1 && config.fileOpen && config.numRows > 1) {
				config.cursorY++;
			} else if (config.numRows > config.screenSize.ws_row && (config.cursorY + config.yOffset) < config.numRows && config.numRows > 2) {
				config.yOffset++;
			} else if (config.numRows < config.screenSize.ws_col && config.cursorY < config.numRows - 1) {
				config.cursorY++;
			}
			//Store desired position
			if (config.desiredX > config.row[(config.cursorY - 1 + config.yOffset)].length) {
				config.cursorX = config.row[(config.cursorY - 1 + config.yOffset)].length + config.xOffset;
			} else {
				config.cursorX = config.desiredX;
			}
			break;
		case LEFT_KEY:
			if (config.cursorX > config.xOffset) config.cursorX--;
			if (config.row[config.cursorY + config.yOffset - 1].text != "") config.desiredX = config.cursorX;
			break;
		case RIGHT_KEY:
			if ((config.cursorX < config.row[config.cursorY - 1 + config.yOffset].length + config.xOffset)) {
				config.cursorX++;
			}
			if (config.row[config.cursorY + config.yOffset - 1].length != 0) config.desiredX = config.cursorX;
			break;
		case PAGE_UP:
			config.cursorY = 1;
			if (config.desiredX < config.row[config.cursorY + config.yOffset - 1].length) {
				config.cursorX = config.desiredX;
			} else {
				config.cursorX = config.row[config.cursorY + config.yOffset - 1].length + config.xOffset, config.desiredX = config.cursorX;
			}
			break;
		case PAGE_DOWN:
			if (config.fileOpen) config.cursorY = config.screenSize.ws_row - 1;
			if (config.desiredX < config.row[config.cursorY + config.yOffset - 1].length) {
				config.cursorX = config.desiredX;
			} else {
				config.cursorX = config.row[config.cursorY + config.yOffset - 1].length + config.xOffset, config.desiredX = config.cursorX;
			}
			break;
		case HOME_KEY:
			config.cursorX = config.xOffset, config.desiredX = config.cursorX;
			break;
		case END_KEY:
			config.cursorX = config.row[config.cursorY + config.yOffset - 1].length + config.xOffset, config.desiredX = config.cursorX;
			break;
		case 'w':
		case CONTROL_RIGHT:
			moveCursorByWord(1);
			config.desiredX = config.cursorX;
			break; //WIP
		case 'b':
		case CONTROL_LEFT:
			moveCursorByWord(-1);
			config.desiredX = config.cursorX;
			break;
		case ':':
			config.editorMode = COMMAND;
			break;
		case ESCAPE_KEY:
			config.editorMode = NORMAL;
			break;
		case 'i':
			config.editorMode = INSERT;
			break;
		default: break;
	}
}

void Input::moveCursorByWord(int direction) {
	std::string *text = &config.row[config.cursorY + config.yOffset - 1].text;
	if (*text == "") return;
	int spaceCount = std::count(text->begin(), text->end(), ' ');
	if (direction == 1) {
		std::string textFromCursor = text->substr(config.cursorX - config.xOffset, text->length());
		size_t spaceIndex = textFromCursor.find_first_of(" ");

		if (spaceIndex == std::string::npos)  {
			config.cursorX = text->length() + config.xOffset;
		} else {
			if (spaceIndex + config.xOffset + 1 < config.cursorX) {
				config.cursorX += spaceIndex + 1;
			return;
			}
			config.cursorX = config.cursorX  - config.xOffset + spaceIndex + config.xOffset + 1;
			//debugMessage = text.substr(config.cursorX - config.xOffset + 1, text.length()) + "   " + std::to_std::string(text.length());
		}
	} else {
		if (config.cursorX == config.xOffset) return;
		std::string textFromCursor = text->substr(0, config.cursorX - config.xOffset);
		size_t spaceIndex = textFromCursor.find_last_of(" ");
		if (textFromCursor.find_first_not_of(" ") == std::string::npos) return;

		if (spaceIndex == std::string::npos)  {
			config.cursorX = config.xOffset;
			debugMessage = "rare";
			return;
		} else {
			debugMessage = "";
			if (config.cursorX == spaceIndex + config.xOffset + 1) {
				spaceIndex = text->substr(0, config.cursorX - config.xOffset - 2).find_last_of(" ");
				config.cursorX = spaceIndex + config.xOffset + 1;
			} else {
				config.cursorX = spaceIndex + config.xOffset + 1;
			}
				if (config.cursorX < config.xOffset) config.cursorX = config.xOffset;
				return;
			
		}

	}

}
