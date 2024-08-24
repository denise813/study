#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>

#include "src/demo_struct.h"


int tst1()
{
    DemoStructA a;
   std::string a_structStr;

   a.item_a = 1;
   a.item_b = 2;

   a_structStr = a.ToString();

   std::cout << "ToString: json=" << std::endl <<
            a_structStr <<  std::endl <<
            "----" << std::endl;

    DemoStructA b;

    b.FormString(a_structStr);

    std::cout << "formString: b.item_a" << std::endl <<
            b.item_a <<  std::endl <<
            "----" << std::endl;
    return 0;
}

int tst2()
{
   DemoStructB a;
   std::string a_structStr;

   a.item_a = 1;
   a.item_b = 2;

   a_structStr = a.ToString();

   std::cout << "ToString: json=" << std::endl <<
            a_structStr <<  std::endl <<
            "----" << std::endl;

    DemoStructB b;
    b.FormString(a_structStr);

    std::cout << "formString: b.item_a" << std::endl <<
            b.item_a <<  std::endl <<
            "----" << std::endl;
    return 0;
}


int main(int argc, char* argv[])
{
   //tst1();
   tst2();
}


