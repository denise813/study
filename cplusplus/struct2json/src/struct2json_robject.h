#ifndef _STRUCT2JSON_ROBJECT_H_
#define _STRUCT2JSON_ROBJECT_H_


#include <string>
#include "cJSON.h"
#include "struct2json_macro_reflection.h"


using namespace std;


class RelectionObject
{
public:
    RelectionObject() = default;
    virtual ~RelectionObject() = default;
public:
  void set_class_name(const std::string & class_name);
  std::string get_class_name();
private:
  std::string m_class_name;
};


typedef void (*construction_object_t)();
class RelectionClassFactory
{
public:
  void register_shared_construction(const std::string & class_name, construction_object_t construction);
  void * call_shared_construction(const std::string & class_name);
  static RelectionClassFactory * instance();
public:
    RelectionClassFactory() = default;
    ~RelectionClassFactory() = default;
};


#endif