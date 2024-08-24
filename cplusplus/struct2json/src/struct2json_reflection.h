#ifndef _STAUCT2JSON_REFLECTION_H
#define _STAUCT2JSON_REFLECTION_H


#include <string>
#include <typeinfo>


using namespace std; 


typedef struct reflection_type_info
{
public:
    std::string m_var_type;
    std::string m_var_name;
    std::string m_this_type;
    void * m_data;
}reflection_type_info_t;



#define IMPReflectionTypeIdName(x) \
({ \
    std::string type =  typeid(x).name(); \
    if (type == typeid(int).name()) type = 'i'; \
    else if (type == typeid(std::string).name()) type = 's'; \
    else type = "object"; \
    (type); \
})


#define IMPReflectionStruct(class_name) \
    void _reflect_get_##class_name(reflection_type_info_t * info) { \
        info->m_var_type = IMPReflectionTypeIdName(class_name); \
        info->m_var_name = #class_name; \
        info->m_this_type = typeid(class_name).name(); \
        info->m_data = (void)(this); \
    }

#define IMPReflectionField(class_name, filed_name) \
    void _reflect_get_##filed_name(reflection_type_info_t * info) { \
        info->m_var_type = IMPReflectionTypeIdName(filed_name); \
        info->m_var_name = #filed_name; \
        info->m_this_type =typeid(filed_name).name(); \
        info->m_data = (void*)(&this->filed_name); \
    }


#define Reflection_ARGSEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _n, ...) _n
#define Reflection_ARGRSEQ() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define Reflection_MACRO_EXPEND(...) __VA_ARGS__
#define Reflection_ARGMAX_HELPER(...) Reflection_MACRO_EXPEND(Reflection_ARGSEQ(__VA_ARGS__))
#define  Reflection_ARGN(...)  Reflection_ARGMAX_HELPER(__VA_ARGS__, \
    Reflection_ARGRSEQ())
#define Reflection_MACRO_CONCAT1(A, B) A##_##B
#define Reflection_MACRO_CONCAT(A, B) Reflection_MACRO_CONCAT1(A, B)


#define Reflection_Field_PUSH(struct_name, filed_list, filed_name) \
do { \
    reflection_type_info_t info; \
    Reflection_MACRO_CONCAT(_reflect_get, filed_name)(&info); \
    filed_list.push_back(info); \
}while(0)


#define Reflection_Field_PUSH_MAP(s, filed_list);
#define Reflection_Field_PUSH_MAP_1(s, filed_list, _1) \
    Reflection_Field_PUSH(s, filed_list, _1);
#define Reflection_Field_PUSH_MAP_2(s, filed_list, _1, _2) \
    Reflection_Field_PUSH_MAP_1(s, filed_list, _1); \
    Reflection_Field_PUSH(s, filed_list, _2);
#define Reflection_Field_PUSH_MAP_3(s, filed_list, _1, _2, _3) \
    Reflection_Field_PUSH_MAP_2(s, filed_list, _1, _2); \
    Reflection_Field_PUSH(s, filed_list, _3);
#define Reflection_Field_PUSH_MAP_4(s, filed_list, _1, _2, _3, _4) \
    Reflection_Field_PUSH_MAP_3(s, filed_list, _1, _2, _3); \
    Reflection_Field_PUSH(s, filed_list, _4);
#define Reflection_Field_PUSH_MAP_5(s, filed_list, _1, _2, _3, _4, _5) \
    Reflection_Field_PUSH_MAP_4(s, filed_list, _1, _2, _3, _4); \
    Reflection_Field_PUSH(s, filed_list, _5);
#define Reflection_Field_PUSH_MAP_6(s, filed_list, _1, _2, _3, _4, _5, _6) \
    Reflection_Field_PUSH_MAP_5(s, filed_list, _1, _2, _3, _4, _5); \
    Reflection_Field_PUSH(s, filed_list, _6);
#define Reflection_Field_PUSH_MAP_7(s, filed_list, _1, _2, _3, _4, _5, _6, _7) \
    Reflection_Field_PUSH_MAP_6(s, filed_list, _1, _2, _3, _4, _5, _6); \
    Reflection_Field_PUSH(s, filed_list, _7);
#define Reflection_Field_PUSH_MAP_8(s, filed_list, _1, _2, _3, _4, _5, _6, _7, _8) \
    Reflection_Field_PUSH_MAP_7(s, filed_list, _1, _2, _3, _4, _5, _6, _7); \
    Reflection_Field_PUSH(s, filed_list, _8);
#define Reflection_Field_PUSH_MAP_9(s, filed_list, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    Reflection_Field_PUSH_MAP_8(s, filed_list, _1, _2, _3, _4, _5, _6, _7, _8); \
    Reflection_Field_PUSH(s, filed_list, _9);
#define Reflection_Field_PUSH_MAP_10(s, filed_list, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    Reflection_Field_PUSH_MAP_9(s, filed_list, _1, _2, _3, _4, _5, _6, _7, _8, _9); \
    Reflection_Field_PUSH(s, filed_list, _10);

#define Reflection_Field(s) \
     IMPReflectionStruct(s)
#define Reflection_Field_1(s, _1) \
    IMPReflectionField(s, _1)
#define Reflection_Field_2(s, _1, _2) \
    Reflection_Field_1(s, _1) \
    IMPReflectionField(s, _2)
#define Reflection_Field_3(s, _1, _2, _3) \
    Reflection_Field_2(s, _1, _2) \
    IMPReflectionField(s, _3)
#define Reflection_Field_4(s, _1, _2, _3, _4) \
    Reflection_Field_3(s, _1, _2, _3) \
    IMPReflectionField(s, _4)
#define Reflection_Field_5(s, _1, _2, _3, _4, _5) \
    Reflection_Field_4(s, _1, _2, _3, _4) \
    IMPReflectionField(s, _5)
#define Reflection_Field_6(s, _1, _2, _3, _4, _5, _6) \
    Reflection_Field_5(s, _1, _2, _3, _4, _5) \
    IMPReflectionField(s, _6)
#define Reflection_Field_7(s, _1, _2, _3, _4, _5, _6, _7) \
   Reflection_Field_6(s, _1, _2, _3, _4, _5, _6) \
    IMPReflectionField(s, _7)
#define Reflection_Field_8(s, _1, _2, _3, _4, _5, _6, _7, _8) \
    Reflection_Field_7(s, _1, _2, _3, _4, _5, _6, _7) \
    IMPReflectionField(s, _8)
#define Reflection_Field_9(s, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    Reflection_Field_8(s, _1, _2, _3, _4, _5, _6, _7, _8) \
    IMPReflectionField(s, _9)
#define Reflection_Field_10(s, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10) \
    Reflection_Field_9(s, _1, _2, _3, _4, _5, _6, _7, _8, _9) \
    IMPReflectionField(s, _10)


#define ReflectionFieldList(s, args_n, ...) \
    Reflection_MACRO_CONCAT(Reflection_Field, args_n)(s, __VA_ARGS__)


#define IMPReflectionFieldListBuild(s, filed_list, args_n, ...) \
   Reflection_MACRO_CONCAT(Reflection_Field_PUSH_MAP, args_n)(s, filed_list, __VA_ARGS__)



#endif
