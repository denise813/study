#ifndef _STRUCT2JSON_TYPE_TRAITS_HPP_
#define _STRUCT2JSON_TYPE_TRAITS_HPP_


#include <stdint.h>

#include <typeinfo>
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>
#include <map>


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
template <typename, template <typename...> class>
struct is_specialization: std::false_type {};
 template <template <typename...> class Template, typename... Args>
struct is_specialization<Template<Args...>, Template>: std::true_type {};
#endif


template<typename T>
struct is_int_traits : std::false_type {};
template<>
struct is_int_traits<int> : std::true_type {};

template<typename T>
struct is_uint64_traits : std::false_type {};
template<>
struct is_uint64_traits<uint64_t> : std::true_type {};

template<typename T>
struct is_double_traits : std::false_type {};
template<>
struct is_double_traits<double> : std::true_type {};

template<typename T>
struct is_string_traits : std::false_type {};
template<>
struct is_string_traits<std::string> : std::true_type {};

template<typename T, typename=bool>
struct is_reflection_object : std::false_type {};
template<typename T>
struct is_reflection_object<T, decltype((bool)&T::is_reflection_object)> : std::true_type {};

template <typename>
struct is_vector_traits : std::false_type {};
template <typename... Args>
struct is_vector_traits<std::vector<Args...>> : std::true_type {};
template <typename... Args>
struct is_vector_traits<std::vector<Args...> &> : std::true_type {};

template <typename>
struct is_map_traits : std::false_type {};
template <typename... Args>
struct is_map_traits<std::map<Args...>> : std::true_type {};
template <typename... Args>
struct is_map_traits<std::map<Args...>&> : std::true_type {};

template <typename T>
struct is_string_key_map_traits {
    static constexpr bool value = std::is_same<typename T::key_type, std::string>::value;
};

template <typename T>
struct element_type_traits {
    using type = T;
};
template <typename T, typename A>
struct element_type_traits<std::vector<T, A>> {
    using type = T;
};
template <typename K, typename V, typename C, typename A>
struct element_type_traits<std::map<K,V,C,A>> {
    using type = V;
};

template <typename T>
struct can_reflection_type_trait{};

#endif