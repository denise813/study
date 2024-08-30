#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <typeinfo>
#include <vector>
#include <memory>

#include "src/demo_struct.h"
#include "src/struct2json_json_serialize.h"

using namespace std;

int tst1()
{
    Struct2Json tools;

    DemoStructA a1;
    a1.item_a = 4;
    a1.item_b = 5;
    std::string a1_doc = tools.toString(a1);

    DemoStructB b1;
    b1.item_a = 14;
    b1.item_b = 15;
    b1.item_c = a1;
    std::string b1_doc = tools.toString(b1);

    DemoStructC c1;
    c1.item_a = 14;
    c1.item_b = 15;
    c1.item_c = a1;
    std::vector<DemoStructB> tmp;
    tmp.push_back(b1);
    c1.item_d = tmp;
    std::string c1_doc = tools.toString(c1);

    std::cout << 
                    "----------------serialize----------------------" <<
                    std::endl;
    std::cout << "doc a1= " << std::endl <<
                    a1_doc << std::endl;
    std::cout << "doc b1= " << std::endl <<
                    b1_doc << std::endl;
    std::cout << "doc c1= " << std::endl <<
                    c1_doc << std::endl;

    std::cout << 
                    "----------------deserialize----------------------" <<
                    std::endl;

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
   tst1();
   tst2();
   //tst3();
}


