
#include "struct2json_reflection_object.h"


#if 0
RelectionField::RelectionField()
{
}

RelectionField::RelectionField(
                const string & name,
                const string & type,
                size_t offset) : m_name(name),
                                m_type(type),
                                m_offset(offset);
{
}

RelectionField::~RelectionField()
{
}


RelectionObject::RelectionObject()
{
}

RelectionObject::~RelectionObject()
{
}

const string & RelectionObject::get_class_name() const
{

}

RelectionField * RelectionObject::get_field(const string & fieldName)
{

}


RelectionClass::RelectionClass()
{
}

RelectionClass::~RelectionClass()
{
}

RelectionField * RelectionClass::get_field(const string & fieldName)
{

}

void RelectionClass::register_class(const string & className, create_object_t method)
{
    return 0;
}

#endif
