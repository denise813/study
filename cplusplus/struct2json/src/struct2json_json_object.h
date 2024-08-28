#ifndef _STAUCT2JSON_JSON_OBJECT_H
#define _STAUCT2JSON_JSON_OBJECT_H


#include <string>
#include <vector>
#include <memory>
#include "struct2json_reflection_object.h"


using namespace std;


class Object2Json
{
public:
    Object2Json();
    ~Object2Json();
    void list_variable(RelectionObject *object,
                    std::vector<RelectionVariable> &var_list);
    template <typename T>
    std::string toString(RelectionObject *object);
    template <typename T>
    void fromString(std::string &json_doc, RelectionObject * object);
private:
    std::shared_ptr<cJSON> strToJson(std::string json_doc);
    std::string JsonToString(cJSON * json);
    //void variable_append_json(RelectionVariable * variable, cJSON * json);
    //void json_to_variable(RelectionVariable * variable, cJSON * json);
    template <typename T>
     std::shared_ptr<cJSON> objectToJson(RelectionObject *object);
    template <typename T>
    RelectionObject * jsonToObject(cJSON * json, RelectionObject *object);
};

template <typename T>
std::shared_ptr<cJSON> Object2Json::objectToJson(RelectionObject *object)
{
    std::shared_ptr<cJSON> root;
    std::vector<RelectionVariable> var_list;
    list_variable(object, var_list);
    for (auto itor : var_list) {
        //cJSON *object = cJSON_CreateObject();

    }
    return root;
}

template <typename T>
RelectionObject * Object2Json::jsonToObject(cJSON * json, RelectionObject *object)
{
    std::shared_ptr<cJSON> root;
    std::vector<RelectionVariable> var_list;
    list_variable(object, var_list);
    for (auto itor : var_list) {
    }
    return nullptr;
}
template <typename T>
std::string Object2Json::toString(RelectionObject *object)
{
    std::shared_ptr<cJSON> root = objectToJson<T>(object);
    std::string json_doc = JsonToString(root.get());
    return json_doc;
}

template <typename T>
void Object2Json::fromString(std::string &json_doc, RelectionObject *object)
{
    std::shared_ptr<cJSON> root = strToJson(json_doc);
    jsonToObject<T>(root.get(), object);
    return;
}

#endif
