#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <typeinfo>
#include <vector>
#include <memory>

#include "src/struct2json_type_traits.hpp"
#include "src/demo_struct.h"
#include "src/struct2json_json_object.h"

using namespace std;

int tst1()
{
    DemoStructA a;
    a.item_a = 4;
    a.item_b = 5;
    Struct2Json tools;
    tools.toString(a);
    return 0;
}

int tst2()
{
    return 0;
}


bool foo(int) { return false; }

int tst3()
{
    return 0;
}

int main(int argc, char* argv[])
{
   //tst1();
   tst2();
   //tst3();
}


