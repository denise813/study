#include <string>
#include <memory>

#include "cJSON.h"
#include "struct2json.h"


Struct2Json::Struct2Json()
{
}

Struct2Json::~Struct2Json()
{
}

void Struct2Json::do_reflect_for_each()
{
    if (m_reflected) {
        return;
    }
    _reflect_for_each();

}

void Struct2Json::_toString(std::string item_name, cJSON * root)
{
    do_reflect_for_each();

    cJSON * item = cJSON_CreateObject();
    for (auto itor : m_Fields) {
        if (itor.m_var_type == IMPReflectionTypeIdName(int)) {
             int * data = (int*)(itor.m_data);
             long long l_data = (*data);
             cJSON_AddIntToObject(item, itor.m_var_name.c_str(), l_data);
        } else if (itor.m_var_type == IMPReflectionTypeIdName(std::string)) {
            std::string * data = (std::string *)(itor.m_data);
            cJSON_AddStringToObject(item, itor.m_var_name.c_str(), data->c_str());
        } else if (itor.m_var_type == IMPReflectionTypeIdName(Struct2Json)) {
            Struct2Json * data = (Struct2Json*)(itor.m_data);
            data->_toString(itor.m_var_name, item);
        } else {
        }
    }

    cJSON_AddItemToObject(root, item_name.c_str(), item);

    return;
}


std::string Struct2Json::ToString()
{
    std::string struct_str;
     std::shared_ptr<cJSON> j_rootPtr = std::shared_ptr<cJSON>(
                    cJSON_CreateObject(),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    });

    do_reflect_for_each();
    
    for (auto itor : m_Fields) {
        if (itor.m_var_type == IMPReflectionTypeIdName(int)) {
             int * data = (int*)(itor.m_data);
             long long l_data = (*data);
             cJSON_AddIntToObject(j_rootPtr.get(), itor.m_var_name.c_str(), l_data);
        } else if (itor.m_var_type == IMPReflectionTypeIdName(std::string)) {
            std::string * data = (std::string *)(itor.m_data);
            cJSON_AddStringToObject(j_rootPtr.get(), itor.m_var_name.c_str(), data->c_str());
        } else if (itor.m_var_type == IMPReflectionTypeIdName(Struct2Json)) {
            Struct2Json * data = (Struct2Json*)(itor.m_data);
            data->_toString(itor.m_var_name, j_rootPtr.get());
        } else {
        }
    }
    char * jsonPrint = cJSON_Print(j_rootPtr.get());
    struct_str = std::string(jsonPrint);
    free(jsonPrint);

    return struct_str;
}

void Struct2Json::_formString(cJSON * root)
{
    cJSON * j_item = NULL;

    do_reflect_for_each();
    
    for (auto itor : m_Fields) {
        if (itor.m_var_type == IMPReflectionTypeIdName(int)) {
            int * data = (int*)(itor.m_data);
            j_item = cJSON_GetObjectItem(root, itor.m_var_name.c_str());
            *data = (int)(cJSON_GetIntValue(j_item));
        } else if (itor.m_var_type == IMPReflectionTypeIdName(std::string)) {
            std::string * data = (std::string*)(itor.m_data);
            j_item = cJSON_GetObjectItem(root, itor.m_var_name.c_str());
            *data = std::string(cJSON_GetStringValue(j_item));
        } else if (itor.m_var_type == IMPReflectionTypeIdName(Struct2Json)) {
            Struct2Json * data = (Struct2Json*)(itor.m_data);
            j_item = cJSON_GetObjectItem(root, itor.m_var_name.c_str());
            data->_formString(j_item);
        }
    }

   return;
}


void Struct2Json::FormString(std::string doc)
{
    cJSON * j_item = NULL;

    std::shared_ptr<cJSON> j_rootPtr = std::shared_ptr<cJSON>(
                    cJSON_ParseWithLength(doc.c_str(), doc.length()),
                    [](cJSON * root){
                        if (root != nullptr) {
                            cJSON_Delete(root);
                        }
                    });

    do_reflect_for_each();
    
    for (auto itor : m_Fields) {
        if (itor.m_var_type == IMPReflectionTypeIdName(int)) {
            int * data = (int*)(itor.m_data);
            j_item = cJSON_GetObjectItem(j_rootPtr.get(), itor.m_var_name.c_str());
            *data = (int)(cJSON_GetIntValue(j_item));
        } else if (itor.m_var_type == IMPReflectionTypeIdName(std::string)) {
            std::string * data = (std::string*)(itor.m_data);
            j_item = cJSON_GetObjectItem(j_rootPtr.get(), itor.m_var_name.c_str());
            *data = std::string(cJSON_GetStringValue(j_item));
        } else if (itor.m_var_type == IMPReflectionTypeIdName(Struct2Json)) {
            Struct2Json * data = (Struct2Json*)(itor.m_data);
            j_item = cJSON_GetObjectItem(j_rootPtr.get(), itor.m_var_name.c_str());
            data->_formString(j_item);
        }
    }

   return;
}
