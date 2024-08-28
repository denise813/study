#ifndef _DEMO_STRUCT_H
#define _DEMO_STRUCT_H


#include "struct2json_reflection_object.h"


class DemoStructA : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
};

class DemoStructB : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
    DemoStructA item_c;
};

class DemoStructC : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
    std::vector<DemoStructA> item_c;
};

#endif
