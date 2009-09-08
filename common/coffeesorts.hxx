#ifndef COFFEE_SORTS_HXX
#define COFFEE_SORTS_HXX

#include <string>
#include <map>

struct NameStr
{
    std::string  name;
    int          qty;

    NameStr(const std::string& iname, int iqty=0) { name = iname; qty=iqty;};
};

typedef std::map<std::string, NameStr*>     SortMap;

const char* const COFFEE_SORT_FNAME="coffee.sorts";
const char* const USERS_FNAME="users.list";
const char* const LEVELS_FNAME="levels.list";
const char* const WEIGHTS_FNAME="weight.list";

class CoSorts
{
    SortMap                   data;
public:
    CoSorts();

    bool                      load_file(const std::string& fname);
    bool                      save_file(const std::string& fname);

    SortMap&                  get_map() { return data;};

    void                      clear() { for(SortMap::iterator it = data.begin(); it!=data.end();++it) delete it->second; data.clear();};
    int                       size() { return data.size();};
    bool                      add(const std::string& id, const std::string& name);
    bool                      add(const std::string& id, const std::string& name, int qty);
};

#endif
