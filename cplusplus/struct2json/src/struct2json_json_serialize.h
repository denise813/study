#ifndef _STAUCT2JSON_JSON_SERIALIZE_H
#define _STAUCT2JSON_JSON_SERIALIZE_H


#include <string>
#include <vector>
#include <memory>
#include "cJSON.h"

#include "struct2json_type_traits.hpp"
#include "struct2json_robject.h"


using namespace std;


//serialize
template<typename T, std::enable_if_t<is_int_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                    const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
    cJSON_AddNumberToObject(root, item_name, (double)value);
};
template<typename T, std::enable_if_t<is_double_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                    const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
    cJSON_AddNumberToObject(root, item_name, (double)value);
};
template<typename T, std::enable_if_t<is_uint64_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                    const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
    cJSON_AddIntToObject(root, item_name, (long long)value);
};
template<typename T, std::enable_if_t<is_string_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON_AddNumberToObject(root, item_name, value.c_str());
   // std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void json_item_serialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON * j_object = cJSON_CreateObject();
    objectToJson(value, j_object);
    cJSON_AddItemToObject(root, item_name, j_object);
    //std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void json_item_serialize(
                const char * item_name, cJSON *root, T * value) {
    cJSON * j_object = cJSON_CreateObject();
    objectToJson(*value, j_object);
    cJSON_AddItemToObject(root, item_name, j_object);
    //std::cout << "json_item_serialize 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_vector_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                const char * item_name, cJSON *root, T &value) {
    cJSON *j_array = cJSON_CreateArray();
    for (auto itor : value) {
        cJSON *j_item = nullptr;
        create_json_object(&j_item, itor);
        cJSON_AddItemToArray(j_array, j_item);
    }
    cJSON_AddItemToObject(root, item_name, j_array);
    //std::cout << "json_serialize 1: vector " << std::endl;
}
template<typename T, std::enable_if_t<is_map_traits<T>::value, int> = 0>
constexpr void json_item_serialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON * j_map = cJSON_CreateObject();
    for (auto itor: value) {
        cJSON *j_item = nullptr;
        create_json_object(&j_item, itor);
        cJSON_AddItemToObject(j_map, itor->frist, j_item);
    }
    cJSON_AddItemToObject(root, item_name, j_map);
    //std::cout << "json_serialize 1: map " << std::endl;
}

template<typename T, std::enable_if_t<is_int_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    *root = cJSON_CreateNumber((double)value);
};
template<typename T, std::enable_if_t<is_double_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    *root = cJSON_CreateNumber(value);
};
template<typename T, std::enable_if_t<is_uint64_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    *root = cJSON_CreateInt((long long)value);
};
template<typename T, std::enable_if_t<is_string_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    *root = cJSON_CreateString(value.c_str());
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    cJSON * j_object = cJSON_CreateObject();
    objectToJson(value, j_object);
    *root = j_object;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T * value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    cJSON * j_object = cJSON_CreateObject();
    objectToJson(*value, j_object);
    *root = j_object;
};
template<typename T, std::enable_if_t<is_vector_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    //std::cout << "create_json_object 1: " << typeid(value).name() << std::endl;
    cJSON *j_array = cJSON_CreateArray();
    for(auto itor : value) {
        cJSON *j_item = nullptr;
        create_json_object(&j_item, itor);
        cJSON_AddItemToArray(j_array, j_item);
    }
    *root = j_array;
};
template<typename T, std::enable_if_t<is_map_traits<T>::value, int> = 0>
constexpr void create_json_object(
                cJSON **root, T & value) {
    cJSON * j_map = cJSON_CreateObject();
    for (auto itor: value) {
        cJSON *j_item = nullptr;
        create_json_object(&j_item, itor);
        cJSON_AddItemToObject(j_map, itor->frist, j_item);
    }
    *root = j_map;
    //std::cout << "json_serialize 1: map " << std::endl;
}

//deserialize
template<typename T, std::enable_if_t<is_int_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    double data = cJSON_GetNumberValue(j_item);
    value = (T)(data);
};
template<typename T, std::enable_if_t<is_double_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    double data = cJSON_GetNumberValue(j_item);
    value = data;
};
template<typename T, std::enable_if_t<is_uint64_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    long long data = cJSON_GetNumberValue(j_item);
    value = (T)data;
};
template<typename T, std::enable_if_t<is_string_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON_AddNumberToObject(root, item_name, value.c_str());
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    value = cJSON_GetStringValue(j_item);
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    jsonToObject(j_item, value);
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T * value) {
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    jsonToObject(j_item, *value);
    //std::cout << "json_item_deserialize 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_vector_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON * j_item = cJSON_GetObjectItem(root, item_name);
    int array_size = cJSON_GetArraySize(j_item);
    cJSON *j_array = cJSON_CreateArray();
    for (int i = 0; i < array_size; i++) {
        cJSON * j_item = cJSON_GetArrayItem(root, i);
        using element_type = typename element_type_traits<T>::type;
        element_type data;
        create_struct_object(j_item, data);
        value.push_back(data);
    }
    cJSON_AddItemToObject(root, item_name, j_array);
    //std::cout << "json_item_deserialize 1: vector " << std::endl;
}
template<typename T, std::enable_if_t<is_map_traits<T>::value, int> = 0>
constexpr void json_item_deserialize(
                const char * item_name, cJSON *root, T & value) {
    cJSON j_item = cJSON_GetObjectItem(root, item_name);
    for(cJSON * j_itor = root->child; j_itor != NULL; j_itor = j_itor->next) {
        using element_type = typename element_type_traits<T>::type;
        element_type data;
        create_struct_object(j_itor, data);
        value.insert(j_itor->string, data);
    }
    //std::cout << "json_item_deserialize 1: map " << std::endl;
}

template<typename T, std::enable_if_t<is_int_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T & value) {
    double data = cJSON_GetNumberValue(root);
    value = (T)(data);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_double_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T & value) {
    double data = cJSON_GetNumberValue(root);
    value = (data);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_uint64_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T & value) {
    long long data = cJSON_GetNumberValue(root);
    value = (T)(data);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_string_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T & value) {
    value = cJSON_GetStringValue(root);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T & value) {
    jsonToObject(root, value);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<std::is_base_of<RelectionObject,T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T * value) {
    jsonToObject(root, *value);
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_vector_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T& value) {
    int array_size = cJSON_GetArraySize(root);
    cJSON *j_array = cJSON_CreateArray();
    for (int i = 0; i < array_size; i++) {
        cJSON * j_item = cJSON_GetArrayItem(root, i);
        using element_type = typename element_type_traits<T>::type;
        element_type data;
        create_struct_object(j_item, data);
        value.push_back(data);
    }
    //std::cout << "create_struct_object 1: " << typeid(value).name() << std::endl;
};
template<typename T, std::enable_if_t<is_map_traits<T>::value, int> = 0>
constexpr void create_struct_object(
                cJSON *root, T& value) {
    for(cJSON * j_itor = root->child; j_itor != NULL; j_itor = j_itor->next) {
        using element_type = typename element_type_traits<T>::type;
        element_type data;
        create_struct_object(j_itor, data);
        value.insert(j_itor->string, data);
    }
    //std::cout << "create_json_object 1: map " << std::endl;
}

template<typename T>
void objectToJson(T &object, cJSON* root)
{
    auto func = [](const char* item_name, cJSON* json, auto& value) {
                    json_item_serialize(item_name, json, value);
                    };
    object_iterate_members(object, root, func);
    return;
}

template<typename T>
void jsonToObject(cJSON * root, T& object)
{
    auto func = [](const char* item_name, cJSON* json, auto& value) {
                    json_item_deserialize(item_name, json, value);
                    };
    object_iterate_members(object, root, func);
    return;
}

//class
class Struct2Json
{
public:
    Struct2Json() = default;
    ~Struct2Json() = default;
    template<typename T>
    static std::string toString(T &object);
    template<typename T>
    static void fromString(std::string &json_doc, T & object);
private:
    static std::shared_ptr<cJSON> stringToJson(std::string json_doc);
    static std::string JsonToString(cJSON * root);
};


template<typename T>
std::string Struct2Json::toString(T &object)
{
    std::shared_ptr<cJSON> j_rootPtr =
                    std::shared_ptr<cJSON>(cJSON_CreateObject(),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    });
    objectToJson(object, j_rootPtr.get());
    std::string json_doc = JsonToString(j_rootPtr.get());
    return json_doc;
}

template<typename T>
void Struct2Json::fromString(std::string &json_doc, T &object)
{
    std::shared_ptr<cJSON> root = stringToJson(json_doc);
    jsonToObject(root.get(), object);
    return;
}


#endif