#ifndef _STAUCT2JSON_REFLECTION_TEMPLATE_H
#define _STAUCT2JSON_REFLECTION_TEMPLATE_H


using namespace std;


template <typename T>
struct remote_pointer {};
template <typename T>
struct remote_pointer<T*> {
    using type = T;
};


template <typename T>
struct remote_const {};
template <typename T>
struct remote_pointer<const T> {
    using type = T;
};


#endif
