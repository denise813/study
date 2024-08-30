#ifndef _DEMO_STRUCT_H_
#define _DEMO_STRUCT_H_

#include <vector>

#include "struct2json_robject.h"


using namespace std;


class DemoStructA : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
};
REFLECT_STRUCT(DemoStructA, item_a);

class DemoStructB : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
    DemoStructA item_c;
};
REFLECT_STRUCT(DemoStructB, item_a, item_c);

class DemoStructC : public RelectionObject
{
public:
    int item_a = 0;
    int item_b = 0;
    DemoStructA item_c;
    std::vector<DemoStructB> item_d;
};
REFLECT_STRUCT(DemoStructC, item_a, item_c, item_d);


#endif
