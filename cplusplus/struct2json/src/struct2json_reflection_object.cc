#include "struct2json_reflection_object.h"


using namespace std;


RelectionVariable::RelectionVariable()
{
}

RelectionVariable::RelectionVariable(
                const std::string & name,
                const std::string & type,
                size_t offset) : m_name(name),
                                m_type(type),
                                m_offset(offset)
{
}

RelectionVariable::~RelectionVariable()
{
}


RelectionObject::RelectionObject()
{
}

RelectionObject::~RelectionObject()
{
}

void RelectionObject::set_class_name(const std::string & class_name)
{
    m_class_name = class_name;
}

const string & RelectionObject::get_class_name() const
{
    return m_class_name;
}

RelectionClass::RelectionClass(std::string class_name)
{
    m_class_name = class_name;
}

RelectionClass::~RelectionClass()
{
}

RelectionVariable * RelectionClass::get_variable(const std::string & variable_name)
{
    auto itor = m_variable_maps.find(variable_name);
    if (itor == m_variable_maps.end()) {
        return nullptr;
    }
    return &(itor->second);
}

void RelectionClass::list_variable(std::vector<RelectionVariable> &variable_list)
{
    for(auto itor : m_variable_maps) {
        variable_list.push_back(itor.second);
    }
}

void RelectionClass::register_variable(
                const std::string & variable_name,
                const std::string & variable_type,
                size_t offset)
{
    RelectionVariable variable = RelectionVariable(variable_name, variable_type, offset);
    m_variable_maps.insert(std::make_pair(variable_name, variable));
    return;
}

RelectionClassFactory g_relectionClassFactory;
RelectionClassFactory * RelectionClassFactory::instance()
{
    RelectionClassFactory * factory = &g_relectionClassFactory;
    return factory;
}

void RelectionClassFactory::register_class(const std::string & class_name)
{
    RelectionClass * cls  = new RelectionClass(class_name);
    m_class_maps.insert(std::make_pair(class_name, *cls));
    return;
}

RelectionClass* RelectionClassFactory::get_class(const std::string & class_name)
{
    auto itor = m_class_maps.find(class_name);
    if (itor == m_class_maps.end()) {
        return nullptr;
    }
    return &(itor->second);
}