#pragma once

class Config {
private:
	struct config {
		const char *backgroundColor;
		const char *foregroundColor;
		const char *lineNumberType;
		bool showLineNumbers;
	};
	void getFile();
	config c = config();
public:
	int initLua();
	config getVars();
};

