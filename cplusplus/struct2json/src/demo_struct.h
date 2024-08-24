#ifndef _DEMO_STRUCT_H
#define _DEMO_STRUCT_H


#include "struct2json.h"



class DemoStructA : public Struct2Json
{
public:
    int item_a = 0;
    //ReflectionField(dsa_a);
    int item_b = 0;
    //ReflectionField(dsa_b);

    ReflectionObject(DemoStructA, item_a, item_b);
};

class DemoStructB : public Struct2Json
{
public:
    int item_a = 0;
    //ReflectionField(dsa_a);
    int item_b = 0;
    //ReflectionField(dsa_b);
    DemoStructA item_c;

    ReflectionObject(DemoStructB, item_a, item_c);
};


#endif
