#ifndef _STAUCT2JSON_VARIABLE_TRAITS_TEMPLATE_H
#define _STAUCT2JSON_VARIABLE_TRAITS_TEMPLATE_H


#include <type_traits>


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
struct is_reflection_object : std::false_type {};
template <typename T>
struct is_reflection_object<T, decltype((void)T::item_d, false)> : std::true_type {};
#endif

template<typename T, typename=bool>
struct is_reflection_object : std::false_type {};
template<typename T>
struct is_reflection_object<T, decltype((bool)&T::is_reflection_object)> : std::true_type {};

#if 0
template <typename, template <typename...> class>
struct is_specialization: std::false_type {};
 template <template <typename...> class Template, typename... Args>
struct is_specialization<Template<Args...>, Template>: std::true_type {};
#endif

template <typename T>
struct is_std_vector : std::false_type {};
template <typename... Args>
struct is_std_vector<std::vector<Args...>> : std::true_type {};

template <typename T>
struct element_type_traits {
    using type = T;
};
template <typename T>
struct element_type_traits<std::vector<T>> {
    using type = T;
};

#endif
