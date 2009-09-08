#ifndef DATA_WATCHER_HXX
#define DATA_WATCHER_HXX

#include "ace/Task.h"
#include "dataset.hxx"
#include "event_notifier.hxx"

class DataAddedEvent: public NotificationEvent
{
public:
    DataSet           *data;
    int                from_idx;
    DataAddedEvent(DataSet* dset, int idx_): data(dset), from_idx(idx_) {};
};


class DataWatcher: public ACE_Task<ACE_MT_SYNCH>
{
    DataSet            *data;
    EventNotifier      *notifier;
public:
    DataWatcher(DataSet* dset, EventNotifier* notif) { data=dset; notifier=notif;};
    int                 svc();
};

#endif
