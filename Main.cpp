
#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

//#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>#include <vector>

using std::string;
using namespace std::string_literals;

#define CTRL_KEY(key) ((key) & 0x1f)

string debugMessage = "";
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
	CONTROL_RIGHT
};

struct termios settings;

struct editorRow {
	string text;
	int length;
};

struct editorConfig {
	int cursorX, cursorY, xOffset, numRows, yOffset;
	int *desiredX = &cursorX;
	bool fileOpen;
	struct winsize screenSize;
	std::vector<editorRow> row;
	string fileName;
};

struct editorConfig config;

struct WriteBuffer {
	char *buffer;
	int length;
};

struct TestBuffer {
	string buffer;
	int length;
};

TestBuffer testBuffer = {"", 0};

/*void appendBuffer(const char *string) {
	char *newString = (char*)realloc(writeBuffer.buffer, strlen(string) + (writeBuffer.length));
	if (newString == NULL) return;
	memcpy(&newString[writeBuffer.length], string, strlen(string));
	writeBuffer.buffer = newString;
	writeBuffer.length += strlen(string);
}*/

void appendBuffer(string text) {
	if ((testBuffer.buffer += text) == testBuffer.buffer) return;
	testBuffer.buffer += text;
	testBuffer.length += text.length();
}

void freeBuffer() {
	testBuffer = {"", 0};
}

void exitProgram(const char *error) {
	write(STDOUT_FILENO, "\x1b[?1049l", 9);
	perror(error);
	exit(1);
}

void exitRawMode() {
	write(STDOUT_FILENO, "\x1b[?1049l", 9);
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &settings) == -1)
		exitProgram("tcgetattr");
}

void drawRows() {
	for (int i = 0; i < config.screenSize.ws_row; i++) {
		appendBuffer(" ~");
		if (i >= config.numRows) {
			if (i == config.screenSize.ws_row / 3 && config.numRows == 0) {
				string message = "Welcome to ___ version 0.0.1";
				appendBuffer(std::string("\x1b[" + std::to_string(config.screenSize.ws_col / 2 - (message.length() / 2)) + string("C")));
				appendBuffer(message);
			}
		} else {
			appendBuffer("\x1b[K");
			appendBuffer(config.row[i + config.yOffset].text);
		}

		if (i < config.screenSize.ws_row - 1) appendBuffer("\r\n");
		if (i == config.screenSize.ws_row - 1) {
			appendBuffer("\x1b[2K");
			string cursor = "["s + std::to_string(config.cursorY + config.yOffset) + ","s + std::to_string(config.cursorX - 2) + "]"s;
			appendBuffer(string("\x1b["s + std::to_string(config.screenSize.ws_col / 2 - 3) + "G"));
			appendBuffer(debugMessage);
			appendBuffer("\x1b[2G");
			appendBuffer(config.fileName);
			appendBuffer(string(string("\x1b[" + std::to_string(config.screenSize.ws_col - cursor.length()) + string("G"))));
			appendBuffer(cursor);
		}
	}
}

void refreshScreen() {
	appendBuffer("\x1b[?25l");
	appendBuffer("\x1b[H");
	drawRows();
	string line = "\x1b[" + std::to_string(config.cursorY) + ";" + std::to_string(config.cursorX) + "H";
	appendBuffer(line.c_str());
	appendBuffer("\x1b[?25h");

	std::cout << testBuffer.buffer << std::flush;

	freeBuffer();
}

void init() {
	config.numRows = 0;
	config.xOffset = 3;
	config.yOffset = 0;
	config.cursorY = 1;
	config.cursorX = config.xOffset;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &config.screenSize)) exitProgram("Get Window Size");
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &settings) == -1)
		exitProgram("tcgetattr");
	atexit(exitRawMode);
	
	write(STDOUT_FILENO, "\x1b[?1049h", 9);

	struct termios m_settings = settings;

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
		exitProgram("tcsetattr");
}

void openEditor(string fileLocation) {

	string text;
	std::ifstream file(fileLocation);

	while (std::getline(file, text)) {
		//Replace tabs with spaces
		string find = "\t", replace = "   ";
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
}

char readKey() {
	char key;
	int status;
	while ((status = read(STDIN_FILENO, &key, 1)) != 1) {
		if (status == -1 && errno != EAGAIN) exitProgram("read");
	}
	if (key == '\x1b') {
		char seq[3];

		if (read(STDOUT_FILENO, &seq[0], 1) == -1) return '\x1b';
		if (read(STDOUT_FILENO, &seq[1], 1) == -1) return '\x1b';
		if (seq[1] >= '0' && seq[1] <= '9') {
			if (read(STDOUT_FILENO, &seq[2], 1) == -1) return '\x1b';
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

/**
* Moves the cursor eaither backwards or forward by one word depending on the direction
* If direction is equal to -1 then it will go backwards, if it is one then it will move forwards.
*/
void moveCursorByWord(int direction) {
	string text = config.row[config.cursorY + config.yOffset - 1].text;
	if (text == "") return;
	int spaceCount = std::count(text.begin(), text.end(), ' ');
	if (direction == 1) {
		string textFromCursor = text.substr(config.cursorX - config.xOffset, text.length());
		size_t spaceIndex = textFromCursor.find_first_of(" ");

		if (spaceIndex == string::npos)  {
			config.cursorX = text.length() + config.xOffset;
		} else {
			if (spaceIndex + config.xOffset + 1 < config.cursorX) {
				config.cursorX += spaceIndex + 1;
			return;
			}
			config.cursorX = config.cursorX  - config.xOffset + spaceIndex + config.xOffset + 1;
			//debugMessage = text.substr(config.cursorX - config.xOffset + 1, text.length()) + "   " + std::to_string(text.length());
			debugMessage = std::to_string(text.substr(config.cursorX - config.xOffset, text.length()).find(" "));
		}
	} else {
		if (config.cursorX == config.xOffset) return;
		string textFromCursor = text.substr(0, config.cursorX - config.xOffset);
		size_t spaceIndex = textFromCursor.find_last_of(" ");
		if (textFromCursor.find_first_not_of(" ") == string::npos) return;

		if (spaceIndex == string::npos)  {
			config.cursorX = config.xOffset;
			debugMessage = "rare";
			return;
		} else {
			debugMessage = "";
			if (config.cursorX == spaceIndex + config.xOffset + 1) {
				spaceIndex = text.substr(0, config.cursorX - config.xOffset - 2).find_last_of(" ");
				config.cursorX = spaceIndex + config.xOffset + 1;
			} else {
				config.cursorX = spaceIndex + config.xOffset + 1;
			}
				if (config.cursorX < config.xOffset) config.cursorX = config.xOffset;
				return;
			
		}

	}

}

void processKey() {
	char key = readKey();
	switch (key) {
		case CTRL_KEY('q'):
			exit(0);
			break;
		case UP_KEY:
			//Move cursor up.
			if (config.cursorY > 1 && config.fileOpen) {
				config.cursorY--;
				if (*config.desiredX > config.row[(config.cursorY + config.yOffset) - 1].length) {
					config.cursorX = config.row[(config.cursorY + config.yOffset) - 1].length + config.xOffset;
				} else {
					config.cursorX = *config.desiredX;
				}
			} else if (config.cursorY + config.yOffset > 1) {
				config.yOffset--;
			}
			break;
		case DOWN_KEY:
			//Move cursor down.
			if (config.cursorY < config.screenSize.ws_row - 1 && config.fileOpen && config.numRows > 1) {
				config.cursorY++;
			} else if ((config.cursorY + config.yOffset) < config.numRows && config.numRows > 2) {
				config.yOffset++;
			}
			//Store desired position
			if (*config.desiredX > config.row[(config.cursorY - 1 + config.yOffset)].length) {
				config.cursorX = config.row[(config.cursorY - 1 + config.yOffset)].length + config.xOffset;
			} else {
				config.cursorX = *config.desiredX;
			}
			break;
		case LEFT_KEY:
			if (config.cursorX > config.xOffset) config.cursorX--;
			break;
		case RIGHT_KEY:
			if ((config.cursorX < config.row[config.cursorY - 1 + config.yOffset].length + config.xOffset)) {
				config.cursorX++;
			}
			break;
		case PAGE_UP:
			config.cursorY = 1;
			break;
		case PAGE_DOWN:
			if (config.fileOpen) config.cursorY = config.screenSize.ws_row - 1;
			break;
		case HOME_KEY:
			config.cursorX = config.xOffset;
			break;
		case END_KEY:
			config.cursorX = config.row[config.cursorY + config.yOffset - 1].length + config.xOffset;
			break;
		case 'w':
		case CONTROL_RIGHT:
			moveCursorByWord(1);
			break; //WIP
		case 'b':
		case CONTROL_LEFT:
			moveCursorByWord(-1);
			break;
		default: break;
	}
}

int main (int argc, char *argv[]) {
	init();
	if (argc > 1) {
		config.fileName = argv[1];
		config.fileOpen = true;
		openEditor(argv[1]);
	} else {
		config.row.push_back({"", 0});

	}
	enableRawMode();
	
	//printf("bruh");
	while (1) {
		refreshScreen();
		processKey();
	}
	return 0;
}

