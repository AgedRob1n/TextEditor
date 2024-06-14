#pragma once
class Input;
class Draw;

class App {
private:
	bool m_running = true;
	void init(int argc, char *argv[]);
	void openEditor(const char *location);
	Input *m_input;
	Draw *m_draw;
public:
	App(int argc, char *argv[]);
	bool isRunning();
	void update();
	static void exitProgram(const char *message);
	void exitRawMode();
};

