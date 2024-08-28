#ifndef _REFLECTION_TYPEINFO_H
#define _REFLECTION_TYPEINFO_H

#include <typeinfo>


using namespace std;

#if 0
class ReflectionTypeInfo
{
public:
    ReflectionTypeInfo(){}
    ReflectionTypeInfo(const std::type_info& tinfo) : pInfo(&tinfo) {}
    ReflectionTypeInfo(const ReflectionTypeInfo& that) : pInfo(that.pInfo) {}
    ReflectionTypeInfo& operator=(const ReflectionTypeInfo& that) {
        this->pInfo = that.pInfo;
        return *this;
    }
    const std::type_info& get() const {
        return *this->pInfo;
    }
    const std::type_info& operator*() const { return this->get(); }
    const char* name() const { return this->get().name(); }
  private:
   const std::type_info* pInfo = nullptr;
};


typedef struct reflection_meta_info
{
public:
    ReflectionTypeInfo m_std_type;
    std::string m_var_type;
    std::string m_var_name;
    std::string m_this_type;
    void * m_data;
}reflection_meta_info_t;

#endif


#endif
