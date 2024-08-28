#ifndef _STAUCT2JSON_VARIABLE_TRAITS_TEMPLATE_H
#define _STAUCT2JSON_VARIABLE_TRAITS_TEMPLATE_H


using namespace std;


template <typename T>
struct variable_traits {
    using type = T;
};
template <typename T>
struct variable_traits<T *> {
    using type = T;
    using type_pointer = T*;
};
template <typename T, typename Cls>
struct variable_traits<T Cls::*> {
    using type = T;
     using type_pointer = T*;
    using clz = Cls; 
};

#if 0
template <typename T, typename = bool>
struct has_member : std::false_type {};
template <typename T>
struct has_member<T, decltype((void)T::reflection_object, false)> : std::true_type {};
#endif
#endif
