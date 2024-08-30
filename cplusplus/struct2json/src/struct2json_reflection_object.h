#ifndef _STAUCT2JSON_REFLECTION_OBJECT_H
#define _STAUCT2JSON_REFLECTION_OBJECT_H

#include <string>
#include "cJSON.h"
#include "struct2json_reflection_def_func.h"

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

template <typename T, typename Func>
constexpr void object_iterate_members(T & obj, cJSON *root, Func&& f) {}
#define REFLECT_STRUCT(class_name, ...)                                           \
template <typename Func>                                                          \
constexpr void object_iterate_members(                                            \
          class_name & obj, cJSON *root, Func&& f) {                              \
  REGISTER_CLASS_OBJECT_EACH_FUNC(class_name, obj, root, f, __VA_ARGS__);         \
}


#endif
