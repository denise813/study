#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "src/demo_struct.h"
#include "src/struct2json_reflection_template.hpp"
#include "src/struct2json_class_template.hpp"
#include "src/struct2json_variable_template.hpp"
#include "src/struct2json_function_template.hpp"
#include "src/struct2json_object_template.hpp"

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
   
    using item_a_ptr = variable_traits<decltype(&Person::item_a)>::pointer_type;
    std::cout >> (*item_a_ptr) >> std::endl;

    //std::cout << p1 << ":" << p2 << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
   //tst1();
   tst3();
}


