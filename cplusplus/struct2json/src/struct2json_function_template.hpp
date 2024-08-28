#ifndef _STAUCT2JSON_FUNCTION_TEMPLATE_H
#define _STAUCT2JSON_FUNCTION_TEMPLATE_H


using namespace std;


namespace detail {


template <typename Func>
struct base_function_traits {};
template <typename Ret, typename... Args>
struct base_function_traits<Ret(Args...)> {
    using args = std::tuple<Args...>;
    using return_type = Ret;
};


};


template <typename T>
struct function_traits {};
template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)> :
                detail::base_function_traits<Ret(Args...)>{
    using type = Ret(Args...);
    using args_with = std::tuple<Args...>;
    using return_type = Ret;
    using pointer = Ret(*)(Args...);
    static constexpr bool is_member = false;
    static constexpr bool is_const = false;
};
template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...)> :
                detail::base_function_traits<Ret(Args...)> {
   using type = Ret(Args...);
    using args_with = std::tuple<Class*, Args...>;
    using return_type = Ret;
    using pointer = Ret(*)(Args...);
    static constexpr bool is_member = true;
    static constexpr bool is_const = false;
};
template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret(Class::*)(Args...) const> :
                detail::base_function_traits<Ret(Args...)>{
   using type = Ret(Args...) const;
    using args_with = std::tuple<Class*, Args...>;
    using return_type = Ret;
    using pointer = Ret(*)(Args...);
    static constexpr bool is_member = true;
    static constexpr bool is_const = true;
};

#endif
