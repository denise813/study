#include <iostream>
#include <string>
#include "./hello_so.h"


using namespace std;


int say_hello()
{
    std::string hello("hello");
    std::cout << hello << std::endl;
    return 0;
} 
