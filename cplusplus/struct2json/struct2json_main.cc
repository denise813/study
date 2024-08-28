#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "src/struct2json_traits_template.hpp"
#include "src/demo_struct.h"

using namespace std;

int tst1()
{
    return 0;
}

int tst2()
{
    return 0;
}


bool foo(int) { return false; }

int tst3()
{
    //using type = remote_pointer<int*>::type;
     using type = variable_traits<decltype(DemoStructA::item_a)>::type;
    //using item_a_ptr = variable_traits<decltype(&DemoStructA::item_a)>::type;
    //auto flag = has_member<DemoStructA>::value;
    //std::cout << flag << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
   //tst1();
   tst3();
}


