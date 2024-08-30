#include <memory>
#include <typeinfo>

#include "cJSON.h"
#include "struct2json_json_serialize.h"

using namespace std;


 std::shared_ptr<cJSON> Struct2Json::stringToJson(std::string json_doc)
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
 
std::string Struct2Json::JsonToString(cJSON * json)
{
    char * jsonPrint = cJSON_Print(json);
    std::string struct_str = std::string(jsonPrint);
    free(jsonPrint);
    return struct_str;
}


