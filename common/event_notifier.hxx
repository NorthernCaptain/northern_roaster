#ifndef EVENT_NOTIFIER_HXX
#define EVENT_NOTIFIER_HXX

class NotificationEvent
{
    int          dummy_value;
public:
    virtual void dummy() {};
  virtual ~NotificationEvent() {};
};

class EventNotifier
{
    int          dummy_value;
public:
    virtual void       send_notification(NotificationEvent*)=0;
    virtual ~EventNotifier() {};
};

#endif

