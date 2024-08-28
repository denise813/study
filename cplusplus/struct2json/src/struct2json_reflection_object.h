#ifndef _STAUCT2JSON_REFLECTION_OBJECT_H
#define _STAUCT2JSON_REFLECTION_OBJECT_H


#include <string>
#include <map>
#include <vector>
#include <typeinfo>


#include "struct2json_traits_template.hpp"
#include "struct2json_variable_traits_template.hpp"


using namespace std;


class RelectionVariable
{
public:
    RelectionVariable();
    RelectionVariable(const std::string & name, const std::string & type, size_t offset);
    ~RelectionVariable();
    const std::string & name(){ return m_name; }
    const std::string & type() {return m_type; }
    size_t offset() { return m_offset; }
private:
    std::string m_name;
    std::string m_type;
    size_t m_offset = 0;
};


class RelectionObject
{
public:
    RelectionObject();
    virtual ~RelectionObject();
    void set_class_name(const std::string & class_name);
    const string & get_class_name() const;
    template <typename T>
    void get(const std::string & variable_name, T & value);
    template <typename T>
    void set(const std::string & variable_name, const T & value);
    template <typename T>
    void register_for_each_variable();
private:
    std::string m_class_name;
    bool is_load_relection = false;
    bool is_reflection_object = true;
};


typedef RelectionObject * (*create_object_t)(void);


class RelectionClass
{
public:
    RelectionClass(std::string class_name);
    virtual ~RelectionClass();
    //int get_class_Variable_count(const string & class_name);
    void register_variable(const std::string & variable_name,
                    const std::string & VariableType,
                    size_t offset);
    RelectionVariable* get_variable(
                    const std::string & variable_name);
    void list_variable(std::vector<RelectionVariable> &variable_list);
private:
    std::string m_class_name;
    std::map<std::string, RelectionVariable> m_variable_maps;
};


class RelectionClassFactory
{
public:
    // reflect class
    void register_class(const std::string & class_name);
    RelectionClass * get_class(const std::string & class_name);
    static RelectionClassFactory * instance();
public:
    RelectionClassFactory() {}
    ~RelectionClassFactory() {}
private:
    std::map<string, RelectionClass> m_class_maps;
};

template <typename T>
void RelectionObject::get(const std::string & variable_name, T & value)
{
    int rc = 0;
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(m_class_name);
    RelectionVariable * Variable = cls->get_variable(variable_name);
    size_t offset = Variable->offset();
    value = *((T *)((unsigned char *)(this) + offset));
}
template <typename T>
void RelectionObject::set(const std::string & variable_name, const T & value)
{
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(m_class_name);
    RelectionVariable * Variable = cls->get_variable(variable_name);
    size_t offset = Variable->offset();
    *((T *)((unsigned char *)(this) + offset)) = value;
}


#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#define container_of(ptr, type, member) ({                      \
                const typeof(((type *)0)->member) * __mptr = (ptr);     \
                (type *)((char *)__mptr - offsetof(type, member)); })


#define IMPREFECTION_OBJECT_CLS(s) \
do{ \
    RelectionClassFactory::instance()->register_class(#s); \
}while(0)
#define IMPREFECTION_OBJECT_VARIABLE(cls, s, x) \
do{ \
    size_t offset = offsetof(s, x); \
    using type = variable_traits<decltype(s::x)>::type; \
    cls->register_variable(#x, typeid(type).name(), offset); \
}while(0)


#define Reflection_ARGSEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _n, ...) _n
#define Reflection_ARGRSEQ() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define Reflection_MACRO_EXPEND(...) __VA_ARGS__
#define Reflection_ARGMAX_HELPER(...) Reflection_MACRO_EXPEND(Reflection_ARGSEQ(__VA_ARGS__))
#define  Reflection_ARGN(...)  Reflection_ARGMAX_HELPER(__VA_ARGS__, \
                Reflection_ARGRSEQ())
#define Reflection_MACRO_CONCAT1(A, B) A##_##B
#define Reflection_MACRO_CONCAT(A, B) Reflection_MACRO_CONCAT1(A, B)


#define REGISTER_OBJECT_VARIABLE(cls, s);
#define REGISTER_OBJECT_VARIABLE_1(cls, s, _1) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _1)
#define REGISTER_OBJECT_VARIABLE_2(cls, s, _1, _2) \
    REGISTER_OBJECT_VARIABLE_1(cls, s, _1) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _2)
#define REGISTER_OBJECT_VARIABLE_3(cls, s, _1, _2, _3) \
    REGISTER_OBJECT_VARIABLE_2(cls, s, _1, _2) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _3)
#define REGISTER_OBJECT_VARIABLE_4(cls, s, _1, _2, _3, _4) \
    REGISTER_OBJECT_VARIABLE_3(cls, s, _1, _2, _3) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _4)
#define REGISTER_OBJECT_VARIABLE_5(cls, s, _1, _2, _3, _4, _5) \
    REGISTER_OBJECT_VARIABLE_4(cls, s, _1, _2, _3, _4) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _5)
#define REGISTER_OBJECT_VARIABLE_6(s, _1, _2, _3, _4, _5, _6) \
    REGISTER_OBJECT_VARIABLE_5(cls, s, _1, _2, _3, _4, _5) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _6)
#define REGISTER_OBJECT_VARIABLE_7(cls, s, _1, _2, _3, _4, _5, _6, _7) \
   REGISTER_OBJECT_VARIABLE_6(cls, s, _1, _2, _3, _4, _5, _6) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _7)
#define REGISTER_OBJECT_VARIABLE_8(s, _1, _2, _3, _4, _5, _6, _7, _8) \
    REGISTER_OBJECT_VARIABLE_7(cls, s, _1, _2, _3, _4, _5, _6, _7) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _8)
#define REGISTER_OBJECT_VARIABLE_9(cls, s, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    REGISTER_OBJECT_VARIABLE_8(cls, s, _1, _2, _3, _4, _5, _6, _7, _8) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _9)
#define REGISTER_OBJECT_VARIABLE_10(cls, s, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    REGISTER_OBJECT_VARIABLE_9(cls, s, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    IMPREFECTION_OBJECT_VARIABLE(cls, s, _10)
#define REGISTER_OBJECT_VARIABLE_LIST(cls, s, args_n, ...) \
    Reflection_MACRO_CONCAT(REGISTER_OBJECT_VARIABLE, args_n)(cls, s, __VA_ARGS__)
#define REGISTER_CLASS_VARIABLE(cls, s, ...) \
    REGISTER_OBJECT_VARIABLE_LIST(cls, s, Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \


#define REGISTER_RELECTION(class_name, ...) \
template <typename T> \
void RelectionObject::register_for_each_variable() \
{ \
    IMPREFECTION_OBJECT_CLS(class_name); \
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(#class_name); \
    REGISTER_CLASS_VARIABLE(cls, class_name, __VA_ARGS__); \
    is_load_relection = true; \
}


#endif
