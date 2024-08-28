#include "struct2json_reflection_object.h"


using namespace std;


RelectionTypeUtils::RelectionTypeUtils()
{
}

RelectionTypeUtils::~RelectionTypeUtils()
{
}

bool RelectionTypeUtils::is_bool(const std::string & type)
{ 
    return (type) == typeid(bool).name();
}

bool RelectionTypeUtils::is_uint64(const std::string & type)
{
    return (type) == typeid(uint64_t).name();
}

bool RelectionTypeUtils::is_double(const std::string & type)
{
    return (type) == typeid(double).name();
}

bool RelectionTypeUtils::is_int(const std::string & type)
{
    return (type) == typeid(int).name();
}

bool RelectionTypeUtils::is_string(const std::string & type)
{ 
    return (type) == typeid(std::string).name();
}


RelectionVariable::RelectionVariable()
{
}

RelectionVariable::RelectionVariable(
                const std::string & name,
                const std::string & type,
                size_t offset,
                bool is_object,
                bool is_array) : m_name(name),
                                m_type(type),
                                m_offset(offset),
                                m_is_object(is_object),
                                m_is_array(is_array)
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

RelectionClass::RelectionClass(std::string class_name, mk_shared_object_func_t alloc)
{
    m_class_name = class_name;
    m_alloc = alloc;
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

std::shared_ptr<RelectionObject> RelectionClass::mk_shared_object()
{
    std::shared_ptr<RelectionObject> object = m_alloc();
    return object;
}

void RelectionClass::register_variable(
                RelectionVariable &info)
{
    m_variable_maps.insert(std::make_pair(info.name(), info));
    return;
}

RelectionClassFactory g_relectionClassFactory;
RelectionClassFactory * RelectionClassFactory::instance()
{
    RelectionClassFactory * factory = &g_relectionClassFactory;
    return factory;
}

void RelectionClassFactory::register_class(const std::string & class_name, mk_shared_object_func_t alloc)
{
    RelectionClass * cls  = new RelectionClass(class_name, alloc);
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

RelectionObject * create_object(const std::string & class_name)
{
    return nullptr;
}