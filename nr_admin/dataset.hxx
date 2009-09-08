#ifndef DATASET_HXX
#define DATASET_HXX

#include "ace/OS.h"
#include "ace/Task.h"
#include <vector>
#include <string>
#include "boost/lexical_cast.hpp"
#include "nr_exception.hxx"


typedef int          DataValue;
typedef std::vector<DataValue>    DataVec;
typedef DataVec::iterator         DataVecIt;

struct ExDataBadIdx: public NRException
{
    std::string       dataset_id;
    int               bad_idx;

    ExDataBadIdx(const std::string& id_, int idx_)
        : NRException(std::string("NR Exception: bad index in dataset access, set_id: ") 
        + id_ + " with idx: " + boost::lexical_cast<std::string>(idx_), -1),
        dataset_id(id_), bad_idx(idx_) {};
};

class DataSet
{
    ACE_Thread_Mutex        mtx;
    DataVec                 data;
    std::string             id;

    void                    generate();
public:
    DataSet();

    DataValue               get(int idx) throw(ExDataBadIdx);
    bool                    set(DataValue val, int idx) throw(ExDataBadIdx);
    bool                    push_back(DataValue val);

    size_t                  size() { ACE_Guard<ACE_Thread_Mutex> guard(mtx); return data.size();};
};

#endif
