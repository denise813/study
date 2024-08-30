#ifndef _STAUCT2JSON_REFLECTION_OBJECT_H
#define _STAUCT2JSON_REFLECTION_OBJECT_H


#include <string>
#include <typeinfo>
#include <memory>

#include "cJSON.h"

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

class RelectionClassFactory
{
public:
  template <class Func>
  void register_shared_construction(const std::string & class_name, Func construction);
  void * call_shared_construction(const std::string & class_name);
  static RelectionClassFactory * instance();
public:
    RelectionClassFactory() {}
    ~RelectionClassFactory() {}
};

template <class Func>
void RelectionClassFactory::register_shared_construction(
        const std::string & class_name,
        Func construction)
{
    return;
}

template <typename T, typename Func>
static constexpr void object_iterate_members(T & obj, cJSON *root, Func&& f) {}
#define REFLECT_STRUCT(class_name, ...)                                           \
std::shared_ptr<ReflctionObject> mk_shard_construction_##class_name(void)         \
{                                                                                 \
  std::shared_ptr<class_name> cls_obj = std::make_shared<class_name>();           \
  shared_ptr<RelectionObject> base =                                              \
          dynamic_pointer_cast<RelectionObject>(cls_obj);                         \
  return base;                                                                    \
}                                                                                 \
RelectionClassFactory::instance()->register_shared_construction(                  \
        #class_name, mk_shard_construction_##class_name);                         \
template <typename Func>                                                 \
static constexpr void object_iterate_members(                    \
          class_name & obj, cJSON *root, Func&& f) {                              \
  REGISTER_CLASS_OBJECT_EACH_FUNC(class_name, root, FUNC, __VA_ARGS__)            \
}


#endif
