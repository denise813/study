#ifndef _STAUCT2JSON_REFLECTION_OBJECT_H
#define _STAUCT2JSON_REFLECTION_OBJECT_H


#include <string>


#if 0
class RelectionField
{
public:
    RelectionField();
    RelectionField(const string & name, const string & type, size_t offset);
    ~RelectionField();
    const string & name(){ return m_name; }
    const string & type() {return m_type; }
    size_t offset() { return m_offset; }
private:
    string m_name;
    string m_type;
    size_t m_offset = 0;
};


class RelectionObject
{
public:
    RelectionObject();
    virtual ~RelectionObject();
    void set_class_name(const string & className);
    const string & get_class_name() const;
    RelectionField * get_field(const string & fieldName);
    template <typename T>
    void get(const string & fieldName, T & value);
    template <typename T>
    void set(const string & fieldName, const T & value);
private:
    string m_className;
};


typedef RelectionObject * (*create_object_t)(void);


class RelectionClass
{
public:
    RelectionClass();
    virtual ~RelectionClass();
    //int get_class_field_count(const string & className);
    void register_class(const string & className, create_object_t method);
    void register_field(const string & className,
                    const string & fieldName,
                    const string & fieldType,
                    size_t offset);
    RelectionField * get_field(
                    const string & className,
                    const string & fieldName);
private:
    static create_object_t m_constr;
    std::map<string, std::vector<ClassField *>> m_Fields;
};


class RelectionClassFactory
{
public:
    // reflect class
    ClassFactory() {}
    ~ClassFactory() {}
    void register_class(const string & className, create_object_t method);
    RelectionObject * create_class(const string & className);
    RelectionClass * get_class(const string & className);
    

private:
    std::map<string, RelectionClass> m_classMap;
};


template <typename T>
int Object::get(const string & fieldName, T & value)
{
    int rc = 0;
    RelectionClass * cls =  Singleton<ClassFactory>::instance()->get_class(m_className);
    if (!cls) {
        return -1;
    }
    RelectionField * field = cls->get_class_field(m_className, fieldName);
    size_t offset = field->offset();
    value = *((T *)((unsigned char *)(this) + offset));
}

template <typename T>
void Object::set(const string & fieldName, const T & value)
{
    ClassField * cls = Singleton<ClassFactory>::instance()->get_class(m_className);
    RelectionField * field = cls->get_class_field(m_className, fieldName);
    size_t offset = field->offset();
    *((T *)((unsigned char *)(this) + offset)) = value;
}


#define REGISTER_RELECTION(className, ...) \
do { \
    RelectionClassFactory->instance()->register_class(#className); \
    RelectionClass * cls =  RelectionClassFactory->instance()->get_class(#className); \
    REGISTER_CLASS_FIELD(cls, ##args); \
while(0)


#define IMPReflection_OBJECT_FIELD(cls, x) \
do{ \
    size_t offset = offsetof(cls, x); \
     cls->register_field(#x, typeid(x), offset); \
}while(0)

#define REGISTER_OBJECT_FIELD(cls);
#define REGISTER_OBJECT_FIELD_1(cls, _1) \
    IMPReflection_OBJECT_FIELD(cls, _1)
#define REGISTER_OBJECT_FIELD_2(cls, _1, _2) \
    REGISTER_OBJECT_FIELD_1(cls, _1) \
    IMPReflection_OBJECT_FIELD(cls, _2)
#define REGISTER_OBJECT_FIELD_3(cls, _1, _2, _3) \
    REGISTER_OBJECT_FIELD_2(cls, _1, _2) \
    IMPReflection_OBJECT_FIELD(cls, _3)
#define REGISTER_OBJECT_FIELD_4(cls, _1, _2, _3, _4) \
    REGISTER_OBJECT_FIELD_3(cls, _1, _2, _3) \
    IMPReflection_OBJECT_FIELD(cls, _4)
#define REGISTER_OBJECT_FIELD_5(cls, _1, _2, _3, _4, _5) \
    REGISTER_OBJECT_FIELD_4(cls, _1, _2, _3, _4) \
    IMPReflection_OBJECT_FIELD(cls, _5)
#define REGISTER_OBJECT_FIELD_6(s, _1, _2, _3, _4, _5, _6) \
    REGISTER_OBJECT_FIELD_5(cls, _1, _2, _3, _4, _5) \
    IMPReflection_OBJECT_FIELD(cls, _6)
#define REGISTER_OBJECT_FIELD_7(cls, _1, _2, _3, _4, _5, _6, _7) \
   REGISTER_OBJECT_FIELD_6(cls, _1, _2, _3, _4, _5, _6) \
    IMPReflection_OBJECT_FIELD(cls, _7)
#define REGISTER_OBJECT_FIELD_8(s, _1, _2, _3, _4, _5, _6, _7, _8) \
    REGISTER_OBJECT_FIELD_7(cls, _1, _2, _3, _4, _5, _6, _7) \
    IMPReflection_OBJECT_FIELD(cls, _8)
#define REGISTER_OBJECT_FIELD_9(cls, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    REGISTER_OBJECT_FIELD_8(cls, _1, _2, _3, _4, _5, _6, _7, _8) \
    IMPReflection_OBJECT_FIELD(cls, _9)
#define REGISTER_OBJECT_FIELD_10(cls, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    REGISTER_OBJECT_FIELD_9(cls, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    IMPReflection_OBJECT_FIELD(cls, _10)
#define REGISTER_OBJECT_FIELD_LIST(cls, args_n, ...) \
     Reflection_MACRO_CONCAT(REGISTER_OBJECT_FIELD, args_n)(cls, __VA_ARGS__)



#define REGISTER_CLASS_FIELD(cls, ...) \
    REGISTER_OBJECT_FIELD_LIST(cls, Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \

#endif

#endif
