
#include "Config.hpp"
#include "Global.hpp"
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>

lua_State *m_state;

const char *getVarFromConfig(const char *variableName, const char *defaultVal) {
	lua_getglobal(m_state, variableName);
	const char *value = defaultVal;
	if (lua_isstring(m_state, -1)) {
		value = lua_tostring(m_state, -1);
	}
	debugMessage += " ";
	debugMessage += value;
	lua_pop(m_state, 1);
	return value;
}

bool getVarFromConfig(const char *variableName, bool defaultVal) {
	lua_getglobal(m_state, variableName);
	bool value = defaultVal;
	if (lua_isboolean(m_state, -1)) {
		value = lua_toboolean(m_state, -1);
		//debugMessage += " " + std::to_string(value);
	}
	debugMessage += " ";
	debugMessage += value ? "true" : "false";
	lua_pop(m_state, 1);
	return value;
}

int getVarFromConfig(const char *variableName, int defaultVal) {
	lua_getglobal(m_state, variableName);
	int value = defaultVal;
	if (lua_isstring(m_state, -1)) {
		value = lua_tointeger(m_state, -1);
	}
	debugMessage += " " + std::to_string(value);
	lua_pop(m_state, 1);
	return value;
}



int Config::initLua() {
	m_state = luaL_newstate();
	luaL_openlibs(m_state);
	if (luaL_dofile(m_state, "config/config.lua") != LUA_OK) return -1;
	return 0;
}

Config::config Config::getVars() {
	c.foregroundColor = getVarFromConfig("foregroundColor", "black");
	c.backgroundColor = getVarFromConfig("backgroundColor", "white");
	c.showLineNumbers = getVarFromConfig("showLineNumbers", false);
	c.lineNumberType = getVarFromConfig("lineNumberType", "absolute");
	return c;
}

