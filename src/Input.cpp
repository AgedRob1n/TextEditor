#include "Input.hpp"
#include "Global.hpp"
#include "App.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <termios.h>
///

#define CTRL_KEY(key) ((key) & 0x1f)

//Needs to be defined here, because the 'atexit' function only accepts functions that aren't associated to a class(includes variables in functions)
struct termios originalSettings, m_settings;

void updateDesiredX() {
	if (state.desiredX == state.cursorX || state.row[state.cursorY + state.yOffset - 1].length == 0) {return;}
	
	if (state.cursorX == state.row[(state.cursorY - 1 + state.yOffset)].length + state.xOffset && state.desiredX > state.cursorX) return;
	state.desiredX = state.cursorX;
}

void setCursorToDesiredPos() {
	int rowLength = state.row[state.cursorY + state.yOffset - 1].length;
	if (state.desiredX > rowLength && rowLength != 3 && rowLength != 1) {
		state.cursorX = rowLength + state.xOffset;
		debugMessage = std::to_string(rowLength);
	} else {
		debugMessage = "huh";
		state.cursorX = state.desiredX;
	}
}

void Input::enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &originalSettings) == -1)
		App::exitProgram("tcgetattr");

	atexit([]() {tcsetattr(STDIN_FILENO, TCSAFLUSH, &originalSettings);});
	write(STDOUT_FILENO, "\x1b[?1049h", 9);

	m_settings = originalSettings;

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
			if (state.cursorY > 1 && state.fileOpen) {
				state.cursorY--;
			} else if (state.cursorY + state.yOffset > 1) {
				state.yOffset--;
			}

			setCursorToDesiredPos();
			break;
		case DOWN_KEY:
			if (state.row[0].text == "" && state.numRows == 1 || (state.cursorY + state.yOffset) == state.numRows) break;
			if (state.cursorY < state.screenSize.ws_row - 1 && state.fileOpen && state.numRows > 1) {
				state.cursorY++;
			} else if (state.numRows > state.screenSize.ws_row && (state.cursorY + state.yOffset) < state.numRows && state.numRows > 2) {
				state.yOffset++;
			} else if (state.numRows < state.screenSize.ws_col && state.cursorY < state.numRows - 1) {
				state.cursorY++;
			}

			setCursorToDesiredPos();
			break;
		case LEFT_KEY:
			if (state.cursorX > state.xOffset) state.cursorX--;
			break;
		case RIGHT_KEY:
			if ((state.cursorX < state.row[state.cursorY - 1 + state.yOffset].length + state.xOffset)) {
				state.cursorX++;
			}
			break;
		case PAGE_UP:
			state.cursorY = 1;
			setCursorToDesiredPos();
			break;
		case PAGE_DOWN:
			if (state.fileOpen && state.numRows >= state.screenSize.ws_row) state.cursorY = state.screenSize.ws_row - 1;
			if (state.numRows < state.screenSize.ws_col) state.cursorY = state.numRows;
			setCursorToDesiredPos();
			break;
		case HOME_KEY:
			state.cursorX = state.xOffset;
			break;
		case END_KEY:
			state.cursorX = state.row[state.cursorY + state.yOffset - 1].length + state.xOffset;
			break;
		case 'w':
		case CONTROL_RIGHT:
			moveCursorByWord(1);
			break; //WIP
		case 'b':
		case CONTROL_LEFT:
			moveCursorByWord(-1);
			break;
		case ':':
			state.editorMode = COMMAND;
			break;
		case ESCAPE_KEY:
			state.editorMode = NORMAL;
			break;
		case 'i':
			state.editorMode = INSERT;
			break;
		default: break;
	}
	updateDesiredX();
}

void Input::moveCursorByWord(int direction) {
	std::string *text = &state.row[state.cursorY + state.yOffset - 1].text;
	if (*text == "") return;
	int spaceCount = std::count(text->begin(), text->end(), ' ');
	if (direction == 1) {
		std::string textFromCursor = text->substr(state.cursorX - state.xOffset, text->length());
		size_t spaceIndex = textFromCursor.find_first_of(" ");

		if (spaceIndex == std::string::npos)  {
			state.cursorX = text->length() + state.xOffset;
		} else {
			if (spaceIndex + state.xOffset + 1 < state.cursorX) {
				state.cursorX += spaceIndex + 1;
				return;
			}
			state.cursorX = state.cursorX  - state.xOffset + spaceIndex + state.xOffset + 1;
			//debugMessage = text.substr(state.cursorX - state.xOffset + 1, text.length()) + "   " + std::to_std::string(text.length());
		}
	} else {
		if (state.cursorX == state.xOffset) return;
		std::string textFromCursor = text->substr(0, state.cursorX - state.xOffset);
		size_t spaceIndex = textFromCursor.find_last_of(" ");
		if (textFromCursor.find_first_not_of(" ") == std::string::npos) return;

		if (spaceIndex == std::string::npos)  {
			state.cursorX = state.xOffset;
			return;
		} else {
			if (state.cursorX == spaceIndex + state.xOffset + 1) {
				spaceIndex = text->substr(0, state.cursorX - state.xOffset - 2).find_last_of(" ");
				state.cursorX = spaceIndex + state.xOffset + 1;
			} else {
				state.cursorX = spaceIndex + state.xOffset + 1;
			}

			if (state.cursorX < state.xOffset) state.cursorX = state.xOffset;
			return;
			
		}

	}

}

