#include <string>
#include "forker/forker.h"
#include "demo_so/demo_so.h"
#include "lua_call_hello_so.h"


using namespace std;



static int averageFunc(lua_State *L)
{
    int n = lua_gettop(L);
    double sum = 0;
    int i;
    for (i = 1; i <= n; i++) sum += lua_tonumber(L, i);
    lua_pushnumber(L, sum / n);
    lua_pushnumber(L, sum);
    return 2;
}

static int sayHelloFunc(lua_State* L)
{
    Forker forker("");
    std::string execDir;
    forker.getCurrentExecDir(execDir);
    forker.setLDPath(execDir);
    void * ldInst = forker.loadLD("demo");
    say_hello();
    forker.unloadLD(ldInst);
    return 0;
}

static struct luaL_Reg  mylib[] = {
        {"sayHello", sayHelloFunc},
        {NULL, NULL}
 };


int luaopen_libhello(lua_State* L)
{
    //lua_register(L, "ss", sayHelloFunc);
    luaL_register(L, "demo", mylib);
	return 1;
}


