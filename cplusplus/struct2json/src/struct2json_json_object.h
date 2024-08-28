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
    std::string toString(RelectionObject *object);
    void fromString(std::string &json_doc, RelectionObject * object);
private:
    std::shared_ptr<cJSON> strToJson(std::string json_doc);
    std::string JsonToString(cJSON * json);
    void variable_append_json(RelectionObject * object, RelectionVariable * variable, cJSON * json);
    void json_to_variable(RelectionObject * object, RelectionVariable * variable, cJSON * json);
    std::shared_ptr<cJSON> objectToJson(RelectionObject *object);
    RelectionObject * jsonToObject(cJSON * json, RelectionObject *object);
    void list_variable(RelectionObject *object,
                    std::vector<RelectionVariable> &var_list);
    std::shared_ptr<RelectionObject> mk_shared_object(std::string class_name);
};


#endif
