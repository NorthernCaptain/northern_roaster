#include "data_watcher.hxx"

int DataWatcher::svc()
{
    while(1)
    {
        size_t sz=data->size();
        DataValue val=data->get(sz-1);
        val-=rand()%10;
        if(val<=0)
            return -1;
        data->push_back(val);
        notifier->send_notification(new DataAddedEvent(data, sz));
        ACE_OS::sleep(1);
    }
}