#include <memory>

#include "cJSON.h"
#include "struct2json_json_object.h"


using namespace std;




Object2Json::Object2Json()
{
}

Object2Json::~Object2Json()
{
}

void Object2Json::list_variable(RelectionObject *object,
                std::vector<RelectionVariable> &var_list)
{
    std::string cls_name = object->get_class_name();
    RelectionClass * cls = RelectionClassFactory::instance()->get_class(cls_name);
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