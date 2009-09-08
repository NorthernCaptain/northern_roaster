#ifndef QT_EVENT_NOTIFIER_HXX
#define QT_EVENT_NOTIFIER_HXX

#include <qwidget.h>
#include "event_notifier.hxx"

const int  qt_notification_event_id=19765;
class QtEventNotifier: public EventNotifier
{
    QWidget*          receiver;
public:
    QtEventNotifier(QWidget* rec): receiver(rec) {};
    void              send_notification(NotificationEvent* ev);
};

class QtNotificationEvent: public QCustomEvent
{
    NotificationEvent  *n_event;
public:
    QtNotificationEvent(NotificationEvent* ev_) : QCustomEvent(qt_notification_event_id), n_event(ev_) {};
    ~QtNotificationEvent() { delete n_event;};
    NotificationEvent*  notification() { return n_event;};
};

#endif

