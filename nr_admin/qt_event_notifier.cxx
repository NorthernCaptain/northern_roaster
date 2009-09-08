#include "qt_event_notifier.hxx"
#include <qapplication.h>

void QtEventNotifier::send_notification(NotificationEvent *ev)
{
    QApplication::postEvent(receiver, new QtNotificationEvent(ev));
}
