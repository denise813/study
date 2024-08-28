#ifndef _STAUCT2JSON_REFLECTION_H
#define _STAUCT2JSON_REFLECTION_H


#include <string>
#include <typeinfo>


using namespace std; 


#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define container_of(ptr, type, member) ({                      \
                const typeof(((type *)0)->member) * __mptr = (ptr);     \
                (type *)((char *)__mptr - offsetof(type, member)); })



#define Reflection_ARGSEQ(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _n, ...) _n
#define Reflection_ARGRSEQ() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define Reflection_MACRO_EXPEND(...) __VA_ARGS__
#define Reflection_ARGMAX_HELPER(...) Reflection_MACRO_EXPEND(Reflection_ARGSEQ(__VA_ARGS__))
#define  Reflection_ARGN(...)  Reflection_ARGMAX_HELPER(__VA_ARGS__, \
                Reflection_ARGRSEQ())
#define Reflection_MACRO_CONCAT1(A, B) A##_##B
#define Reflection_MACRO_CONCAT(A, B) Reflection_MACRO_CONCAT1(A, B)



#endif
