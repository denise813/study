#ifndef __LUA_CALL_HELLO_SO_H__
#define __LUA_CALL_HELLO_SO_H__


#include <stdio.h>


extern "C" {

#include<lua.h>
#include<lauxlib.h>
#include<lualib.h>
};


extern "C" {
int luaopen_libhello(lua_State* L);
};


#endif
