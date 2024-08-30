#ifndef _STAUCT2JSON_REFLECTION_H
#define _STAUCT2JSON_REFLECTION_H


#include <string>
#include <typeinfo>


#define Reflection_ARGSEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _n, ...) _n
#define Reflection_ARGRSEQ() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define Reflection_MACRO_EXPEND(...) __VA_ARGS__
#define Reflection_ARGMAX_HELPER(...) Reflection_MACRO_EXPEND(Reflection_ARGSEQ(__VA_ARGS__))
#define  Reflection_ARGN(...)  Reflection_ARGMAX_HELPER(__VA_ARGS__, \
                Reflection_ARGRSEQ())
#define Reflection_MACRO_CONCAT1(A, B) A##_##B
#define Reflection_MACRO_CONCAT(A, B) Reflection_MACRO_CONCAT1(A, B)


#define CALL_FUNC(s, root, f, x) \
do { \
    func(#x, root, s::x); \
}while(0)
#define REGISTER_MEMBER_OBJECT_EACH_FUNC(s, func);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_1(s, root, f, _1) \
    CALL_FUNC(s, root, f, _1);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_2(s, root, f, _1, _2) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_1(s, root, f, _1); \
     CALL_FUNC(s, root, f, _2);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_3(s, root, f, _1, _2, _3) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_2(s, root, f, _1, _2); \
    CALL_FUNC(s,func, _3);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_4(s, root, f, _1, _2, _3, _4) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_3(s, root, f, _1, _2, _3); \
    CALL_FUNC(s,func, _4);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_5(s, root, f, _1, _2, _3, _4, _5) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_3(s, root, f, _1, _2, _3, _4); \
    CALL_FUNC(s,func, _5);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_LIST(s, root, f, args_n, ...) \
    Reflection_MACRO_CONCAT(REGISTER_MEMBER_OBJECT_EACH_FUNC, args_n)(s, root, f, __VA_ARGS__)
#define REGISTER_CLASS_OBJECT_EACH_FUNC(s, root, f, ...) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_LIST(s, root, f, Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \


#endif