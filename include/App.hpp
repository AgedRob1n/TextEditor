
class App {
private:
	bool m_running = true;
	void init(int argc, char *argv[]);
public:
	App(int argc, char *argv[]);
	bool isRunning();
	void update();
	static void exitProgram(const char *message);
	void exitRawMode();
};

