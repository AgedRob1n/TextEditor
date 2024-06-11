#pragma once
#include <termios.h>

class Input {
private:
	char readKey();
	struct termios m_originalSettings, m_settings;
	void moveCursorByWord(int direction);
public:
	void processKeys();
	void enableRawMode();
};

