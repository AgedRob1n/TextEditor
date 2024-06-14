#pragma once

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

class Input {
private:
	char readKey();
	void moveCursorByWord(int direction);
public:
	void processKeys();
	void enableRawMode();
};

