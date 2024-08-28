#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <typeinfo>
#include <vector>
#include <memory>

#include "src/struct2json_traits_template.hpp"
#include "struct2json_variable_traits_template.hpp"
#include "src/demo_struct.h"

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
    using type = variable_traits<decltype(DemoStructA::item_a)>::type;
    //using item_a_ptr = variable_traits<decltype(&DemoStructA::item_a)>::type;
    auto flag = is_reflection_object<DemoStructA>::value;
    std::cout << flag << std::endl;
    auto ok1 = std::is_base_of<RelectionObject, DemoStructA>::value;
    std::cout << ok1 << std::endl;
    using typeA = variable_traits<decltype(DemoStructC::item_c)>::type;
    std::cout << typeid(typeA).name() << std::endl;
    auto ok2 = std::is_base_of<std::vector<RelectionObject>, typeA>::value;
    std::cout << ok2 << std::endl;
    auto ok3 = is_std_vector<decltype(DemoStructC::item_c)>::value;
    std::cout << "ok3"<< ok3 << std::endl;
    std::shared_ptr<DemoStructC> x = std::make_shared<DemoStructC>();
    RelectionObject * object = x.get();
    using object_type = variable_traits<decltype(*object)>::type;
    //object_type a;
    std::cout << "object_type1" <<typeid(object_type).name() << std::endl;
    std::cout << "object_type2" <<typeid(decltype(*object)).name() << std::endl;
    std::cout << "object_type3" <<typeid(object).name() << std::endl;
    std::cout << "object_type4" <<typeid(*object).name() << std::endl;

    DemoStructC * y = new DemoStructC();
    RelectionObject * object_y = y;
    using object_type_y = variable_traits<decltype(*object_y)>::type;
    //object_type a;
    std::cout << "object_type1" <<typeid(object_type_y).name() << std::endl;
    std::cout << "object_type2" <<typeid(decltype(*object_y)).name() << std::endl;
    std::cout << "object_type3" <<typeid(object_y).name() << std::endl;
    std::cout << "object_type4" <<typeid(*object_y).name() << std::endl;

    using element_type = element_type_traits<std::vector<long>>::type; 
     std::cout << "element_type" <<typeid(element_type).name() << std::endl;
    return 0;
}

int main(int argc, char* argv[])
{
   //tst1();
   tst3();
}


