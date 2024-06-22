
files := $(wildcard ./src/*.cpp)
INC = -I./include/
binary = ./bin/TextEditor

all:
	@g++ $(INC) -o $(binary) Main.cpp $(files) -llua5.4
	@$(binary) $(file)

