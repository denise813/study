#ifndef _DEMO_STRUCT_H
#define _DEMO_STRUCT_H


#include "struct2json_reflection_object.h"
#include "struct2json_reflection.h"


class DemoStructA : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
};

REGISTER_RELECTION(DemoStructA, item_a);

class DemoStructB : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
    DemoStructA item_c;
};


#endif
