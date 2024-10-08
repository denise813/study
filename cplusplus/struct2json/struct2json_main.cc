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
    DemoStructA a1;
    a1.item_a = 4;
    a1.item_b = 5;
    std::string a1_doc = Struct2Json::toString(a1);

    DemoStructB b1;
    b1.item_a = 14;
    b1.item_b = 15;
    b1.item_c = a1;
    std::string b1_doc = Struct2Json::toString(b1);

    DemoStructC c1;
    c1.item_a = 14;
    c1.item_b = 15;
    c1.item_c = a1;
    std::vector<DemoStructB> tmp;
    tmp.push_back(b1);
    c1.item_d = tmp;
    std::string c1_doc = Struct2Json::toString(c1);

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
    
    DemoStructA a2;
    Struct2Json::fromString(a1_doc, a2);
    std::cout << "doc a2.item_a= " << std::endl <<
                    a2.item_a << std::endl;

    DemoStructB b2;
    Struct2Json::fromString(b1_doc, b2);
    std::cout << "doc b2.item_a,b2.item_c.item_a = " << std::endl <<
                    b2.item_a << 
                    "," <<
                    b2.item_c.item_a << std::endl;
    
    DemoStructC c2;
    Struct2Json::fromString(c1_doc, c2);
    std::cout << "doc c2.item_a,c2.item_c.item_a = " << std::endl <<
                    c2.item_a << 
                    "," <<
                    c2.item_c.item_a << std::endl;
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


