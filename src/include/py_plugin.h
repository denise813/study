#ifndef _PY_PLUGIN_H_
#define _PY_PLUGIN_H_

#include <vector>
#include <string>
#include "boost/any.hpp"

using std::vector;
using std::string;


class PyPlugin
{
public:
    PyPlugin();
    virtual ~PyPlugin();
    int add_path_to_sys(const string &path);
    int add_path_to_sys(vector <string> &paths);

    int call_function(string &module_name, string &func_name,
                    vector<boost::any> & args,
                    vector<boost::any> & results);
};

#endif


