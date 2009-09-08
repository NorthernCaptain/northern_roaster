#include "dataset.hxx"

DataSet::DataSet()
{
    generate();
}

DataValue DataSet::get(int idx)
{
    try
    {
        ACE_Guard<ACE_Thread_Mutex> guard(mtx);
        DataValue val=data.at(idx);
        return val;
    }
    catch(...)
    {
        throw ExDataBadIdx(id, idx);
    }
}

bool DataSet::set(DataValue val, int idx)
{
    try
    {
        ACE_Guard<ACE_Thread_Mutex> guard(mtx);
        DataValue& to=data.at(idx);
        to=val;
        return true;
    }
    catch(...)
    {
        throw ExDataBadIdx(id, idx);
    }
}

bool DataSet::push_back(DataValue val)
{
    ACE_Guard<ACE_Thread_Mutex> guard(mtx);
    data.push_back(val);
    return true;
}

void DataSet::generate()
{
    int div=rand()% 3 + 1;
    for(int i=0;i<1000;i++)
        push_back(i/div+rand()%20);
}