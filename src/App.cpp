#include "App.hpp"
#include "Config.hpp"
#include "Input.hpp"
#include "Draw.hpp"
#include "Global.hpp"
#include <fstream>

using std::string;
Config conf;

void App::exitProgram(const char *error) {
	write(STDOUT_FILENO, "\x1b[?1049l", 9);
	perror(error);
	exit(1);
}

void App::openEditor(const char *location) {

	string text;
	std::ifstream file(location);
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
		if (text.length() > state.screenSize.ws_col) text = text.substr(0, state.screenSize.ws_col - 3);
		state.row.push_back({text, static_cast<int>(text.size())});
		state.numRows++;
	}

	file.close();
}

App::App(int argc, char *argv[]) {
	m_input = new Input;
	m_draw = new Draw;
	conf = Config();
	conf.initLua();
	conf.getVars();
	
  if (argc > 1) {
    openEditor(argv[1]);
		state.xOffset = Draw::getDigits(state.numRows) + 2;
		state.cursorX = state.xOffset;
		state.desiredX = state.cursorX;
    state.fileName = argv[1];
    state.fileOpen = true;
  } else {
    state.row.push_back({"", 0});
  }
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &state.screenSize))
    App::exitProgram("Get Window Size");
	m_input->enableRawMode();


	while (1) {
		update();
	}
}

void App::update() {
  m_draw->refreshScreen();
	m_input->processKeys();
}

