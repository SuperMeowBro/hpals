//
// Created by Alexander Wecker on 07.01.16.
//

#ifndef HPALS_HPALS_SERVICE_H
#define HPALS_HPALS_SERVICE_H

#include <Eldbus.h>

#define BUS "org.hpals.als"
#define PATH "/org/hpals/als"
#define INTERFACE "org.hpals.als"

#define HPWMI_SYSPATH "/sys/devices/platform/hp-wmi/"
#define HPWMI_SYSATTR_ALS "als"

static void on_name_request(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED);

static Eldbus_Message *get_value(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg);

static Eldbus_Message *set_value(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg);

static Eldbus_Message *toggle_on_off(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg);

static const Eldbus_Method methods[] = {
        {"ToggleOnOff", NULL,                          ELDBUS_ARGS({ "i", "int32" }), toggle_on_off},
        {"SetValue",    ELDBUS_ARGS({ "i", "int32" }), ELDBUS_ARGS({ "i", "int32" }), set_value},
        {"GetValue",    NULL,                          ELDBUS_ARGS({ "i", "int32" }), get_value},
        {}
};

static const Eldbus_Service_Interface_Desc iface_desc = {
        INTERFACE, methods
};

#endif //HPALS_HPALS_SERVICE_H
