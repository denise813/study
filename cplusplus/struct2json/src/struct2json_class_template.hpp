#ifndef _STAUCT2JSON_CLASS_TEMPLATE_H
#define _STAUCT2JSON_CLASS_TEMPLATE_H


#include "struct2json_function_template.hpp"


using namespace std;


struct Person
{
    int item_a;
    std::string item_b;
    bool is_female = false;
    bool isFemale() { return is_female; }
};

template <typename T>
struct class_info;
template<>
struct class_info <Person> {
    using funcs = std::tuple<function_traits<decltype(&Person::isFemale)>>;
};


#endif
