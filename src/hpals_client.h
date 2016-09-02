//
// Created by Alexander Wecker on 07.01.16.
//

#ifndef HPALS_HPALS_CLIENT_H
#define HPALS_HPALS_CLIENT_H

#include <Eldbus.h>

#define NOTIFICATIONS_BUS "org.freedesktop.Notifications"
#define NOTIFICATIONS_PATH "/org/freedesktop/Notifications"
#define NOTIFICATIONS_INTERFACE "org.freedesktop.Notifications"

#define HPALS_BUS "org.hpals.als"
#define HPALS_PATH "/org/hpals/als"
#define HPALS_INTERFACE "org.hpals.als"

static void on_toggle_on_off(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED);

static void on_notify(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED);

static void call_notify(Eldbus_Proxy *proxy, const char *app_name, unsigned int replaces_id,
                        const char *app_icon, const char *summary, const char *body, int expire_timeout);

static void call_close_notification(Eldbus_Proxy *proxy, unsigned int id);

static void call_toggle_on_off(Eldbus_Proxy *proxy);

static unsigned int read_rid_from_file();

#endif //HPALS_HPALS_CLIENT_H
