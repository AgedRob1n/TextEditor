
#include "App.hpp"

int main (int argc, char *argv[]) {
	App app = App(argc, argv);
	while (1) {
		app.update();
	}
}

