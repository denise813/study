#ifndef _STRUCT2JSON_MACRO_REFLECTION_H_
#define _STRUCT2JSON_MACRO_REFLECTION_H_


#define Reflection_ARGSEQ(\
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14, _15, _16, _17, _18, \
                _n, ...) _n
#define Reflection_ARGRSEQ() \
                18, 17, 16, 15, 14, 13, \
                12, 11, 10, 9, 8, 7, \
                6, 5, 4, 3, 2, 1, 0
#define Reflection_MACRO_EXPEND(...) __VA_ARGS__
#define Reflection_ARGMAX_HELPER(...) \
                Reflection_MACRO_EXPEND(Reflection_ARGSEQ(__VA_ARGS__))
#define  Reflection_ARGN(...)  Reflection_ARGMAX_HELPER(__VA_ARGS__, \
                Reflection_ARGRSEQ())
#define Reflection_MACRO_CONCAT1(A, B) A##_##B
#define Reflection_MACRO_CONCAT(A, B) Reflection_MACRO_CONCAT1(A, B)


#define CALL_FUNC(s, obj, root, f, x) \
do { \
    f(#x, root, obj.x); \
}while(0)
#define REGISTER_MEMBER_OBJECT_EACH_FUNC(\
                s, obj, f);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_1(\
                s, obj, root, f, \
                _1) \
    CALL_FUNC(s, obj, root, f, _1);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_2(\
                s, obj, root, f, \
                _1, _2) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_1(s, obj, root, f, \
                    _1); \
     CALL_FUNC(s, obj, root, f, _2);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_3(\
                s, obj, root, f, \
                _1, _2, _3) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_2(s, obj, root, f, \
                    _1, _2); \
    CALL_FUNC(s, obj, root, f, _3);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_4(\
                s, obj, root, f, \
                _1, _2, _3, _4) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_3(\
                    s, obj, root, \
                    f, _1, _2, _3); \
    CALL_FUNC(s, obj, root, f, _4);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_5(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_4(\
                    s, obj, root, f, \
                    _1, _2, _3, _4); \
    CALL_FUNC(s, obj, root, f, _5);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_6(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_5(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5); \
    CALL_FUNC(s, obj, root, f, _6);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_7(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_6(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6); \
    CALL_FUNC(s, obj, root, f, _7);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_8(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_7(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7); \
    CALL_FUNC(s, obj, root, f, _8);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_9(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_8(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8); \
    CALL_FUNC(s, obj, root, f, _9);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_10(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_9(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9); \
    CALL_FUNC(s, obj, root, f, _10);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_11(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_10(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10); \
    CALL_FUNC(s, obj, root, f, _11);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_12(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_11(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11); \
    CALL_FUNC(s, obj, root, f, _12);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_13(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_12(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12); \
    CALL_FUNC(s, obj, root, f, _13);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_14(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_13(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12, \
                    _13); \
    CALL_FUNC(s, obj, root, f, _14);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_15(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14, _15) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_14(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12, \
                    _13, _14); \
    CALL_FUNC(s, obj, root, f, _15);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_16(\
               s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14, _15, _16, _17, _18) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_15(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12, \
                    _13, _14, _15); \
    CALL_FUNC(s, obj, root, f, _16);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_17(\
                 s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14, _15, _16, _17) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_16(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12, \
                    _13, _14, _15, _16); \
    CALL_FUNC(s, obj, root, f, _16);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_18(\
                s, obj, root, f, \
                _1, _2, _3, _4, _5, _6, \
                _7, _8, _9, _10, _11, _12, \
                _13, _14, _15, _16, _17, _18) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_17(\
                    s, obj, root, f, \
                    _1, _2, _3, _4, _5, _6, \
                    _7, _8, _9, _10, _11, _12, \
                    _13, _14, _15, _16, _17); \
    CALL_FUNC(s, obj, root, f, _18);
#define REGISTER_MEMBER_OBJECT_EACH_FUNC_LIST(\
                s, obj, root, f, args_n, ...) \
    Reflection_MACRO_CONCAT(REGISTER_MEMBER_OBJECT_EACH_FUNC, args_n)(\
                    s, obj, root, f, __VA_ARGS__)
#define REGISTER_CLASS_OBJECT_EACH_FUNC(s, obj, root, f, ...) \
    REGISTER_MEMBER_OBJECT_EACH_FUNC_LIST(\
                    s, obj, root, f, \
                    Reflection_ARGN(__VA_ARGS__), __VA_ARGS__); \


//template <typename T, typename Func>
//constexpr void object_iterate_members(T & obj, cJSON *root, Func&& f) {}
#define STATIC_REFLECT_STRUCT(cls_name, ...)                                           \
template <typename Func>                                                          \
constexpr void object_iterate_members(                                            \
          cls_name & obj, cJSON *root, Func&& f) {                              \
    REGISTER_CLASS_OBJECT_EACH_FUNC(cls_name, obj, root, f, __VA_ARGS__);         \
}


#endif