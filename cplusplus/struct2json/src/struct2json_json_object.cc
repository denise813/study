#include <memory>
#include <typeinfo>

#include "cJSON.h"
#include "struct2json_json_object.h"


using namespace std;


Object2Json::Object2Json()
{
}

Object2Json::~Object2Json()
{
}

std::shared_ptr<RelectionObject> Object2Json::mk_shared_object(std::string class_name)
{
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(class_name);
    std::shared_ptr<RelectionObject> object = cls->mk_shared_object();
    return 0;
}

void Object2Json::list_variable(RelectionObject *object,
                std::vector<RelectionVariable> &var_list)
{
    std::string class_name = object->get_class_name();
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(class_name);
    cls->list_variable(var_list);
}

 std::shared_ptr<cJSON> Object2Json::strToJson(std::string json_doc)
 {
    std::shared_ptr<cJSON> j_rootPtr = std::shared_ptr<cJSON>(
                    cJSON_ParseWithLength(json_doc.c_str(), json_doc.length()),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    });
    return j_rootPtr;
 }
std::string Object2Json::JsonToString(cJSON * json)
{
    char * jsonPrint = cJSON_Print(json);
    std::string struct_str = std::string(jsonPrint);
    free(jsonPrint);
    return struct_str;
}

void Object2Json::variable_append_json(
                RelectionObject *object,
                RelectionVariable * variable,
                cJSON * json)
{
    std::string type = variable->type();
    if (variable->is_object()) {
        RelectionObject* data = nullptr;
        object->get(variable->name(), &data);
        std::shared_ptr<cJSON> j_item = objectToJson(data);
        cJSON_AddItemToObject(j_item.get(), variable->name().c_str(), json);
    } else if (variable->is_array()) {
        //
        
    } else if (RelectionTypeUtils::is_uint64(type)) {
        uint64_t data;
        object->get(variable->name(), data);
        cJSON * j_item = cJSON_CreateInt((long long)data);
        cJSON_AddItemToObject(j_item, variable->name().c_str(), json);
    } else if (RelectionTypeUtils::is_bool(type)) {
        bool data;
        object->get(variable->name(), data);
        cJSON * j_item = cJSON_CreateBool(data);
        cJSON_AddItemToObject(j_item, variable->name().c_str(), json);
    } else if (RelectionTypeUtils::is_int(type)) {
        int data;
        object->get(variable->name(), data);
        cJSON * j_item = cJSON_CreateNumber(data);
        cJSON_AddItemToObject(j_item, variable->name().c_str(), json);
    } else if (RelectionTypeUtils::is_double(type)) {
        double data;
        object->get(variable->name(), data);
        cJSON * j_item = cJSON_CreateNumber(data);
        cJSON_AddItemToObject(j_item, variable->name().c_str(), json);
    } else if (RelectionTypeUtils::is_string(type)) {
        std::string data;
        object->get(variable->name(), data);
        cJSON * j_item = cJSON_CreateString(data.c_str());
        cJSON_AddItemToObject(j_item, variable->name().c_str(), json);
    } else {

    }
    return;
}

void Object2Json::json_to_variable(
                RelectionObject *object,
                RelectionVariable * variable,
                cJSON * json)
{
    return;
}


std::shared_ptr<cJSON> Object2Json::objectToJson(RelectionObject *object)
{
    std::shared_ptr<cJSON> root;
    std::vector<RelectionVariable> var_list;
    list_variable(object, var_list);
    for (auto itor : var_list) {   
        variable_append_json(object, &itor, root.get());
    }
    return root;
}

RelectionObject * Object2Json::jsonToObject(cJSON * json, RelectionObject *object)
{
    std::shared_ptr<cJSON> root;
    std::vector<RelectionVariable> var_list;
    list_variable(object, var_list);
    for (auto itor : var_list) {
        json_to_variable(object, &itor, root.get());
    }
    return nullptr;
}
std::string Object2Json::toString(RelectionObject *object)
{
    std::shared_ptr<cJSON> root = objectToJson(object);
    std::string json_doc = JsonToString(root.get());
    return json_doc;
}

void Object2Json::fromString(std::string &json_doc, RelectionObject *object)
{
    std::shared_ptr<cJSON> root = strToJson(json_doc);
    jsonToObject(root.get(), object);
    return;
}