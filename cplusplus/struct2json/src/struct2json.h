#ifndef _STAUCT2JSON_H
#define _STAUCT2JSON_H


#include <vector>
#include "struct2json_reflection.h"
#include "cJSON.h"


using namespace std;


#define ReflectionObject(s, ...) \
    ReflectionFieldList(s, Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \
    void _reflect_for_each() { \
       IMPReflectionFieldListBuild(s, m_Fields, Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \
    }

class Struct2Json
{
public:
    Struct2Json();
    virtual ~Struct2Json();
public:
    virtual std::string ToString();
    virtual void FormString(std::string doc);
protected:
    virtual void _reflect_for_each() = 0;
private:
    void _toString(std::string item_name, cJSON * root);
    void _formString(cJSON * root);
    void do_reflect_for_each();
protected:
    std::vector<reflection_type_info_t> m_Fields;
private:
    bool m_reflected = false;
};



#endif
