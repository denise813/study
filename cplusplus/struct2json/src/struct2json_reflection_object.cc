#include "struct2json_reflection_object.h"


using namespace std;


void RelectionObject::set_class_name(const std::string & class_name)
{
    m_class_name = class_name;
}

std::string RelectionObject::get_class_name()
{
    return m_class_name;
}

void * RelectionClassFactory::call_shared_construction(const std::string & class_name)
{
    return nullptr;
}

RelectionClassFactory * RelectionClassFactory::instance()
{
    return nullptr;
}

void RelectionClassFactory::register_shared_construction(
        const std::string & class_name,
        construction_object_t construction)
{
    return;
}