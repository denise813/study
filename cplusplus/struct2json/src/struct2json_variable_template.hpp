#ifndef _STAUCT2JSON_VARIABLE_TEMPLATE_H
#define _STAUCT2JSON_VARIABLE_TEMPLATE_H


using namespace std;


namespace detail {


template <typename T>
struct variable_type {
    using type = T;
};
template <typename T, typename Cls>
struct variable_type<T Cls::*> {
    using type = T;
    using class_type = Cls;
    static constexpr bool is_class = true;
};


};


template <typename T>
using variable_type_t = typename detail::variable_type<T>::type;


namespace internal {


template <typename T>
struct basic_variable_traits {
    using type = variable_type_t<T>;
};


};


template <typename T>
struct variable_traits {};
template <typename T>
struct variable_traits<T *> : internal::basic_variable_traits<T>{
    using pointer_type = T *;
};
template <typename T, typename Cls>
struct variable_traits<T Cls::*> : internal::basic_variable_traits<T Cls::*>{
    using pointer_type = T Cls::*;
    using clz = Cls; 
};


template <typename T>
struct field_traits {
    constexpr field_traits(T&& pointer) : pointer(pointer) {}
    T pointer;
};


#endif
