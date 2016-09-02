//
// Created by Alexander Wecker on 06.01.16.
//

#include <Eeze.h>
#include <Eldbus.h>
#include <Ecore.h>
#include "hpals_service.h"

// Todo: Fehlerbehandlung einpfelgen

static Eldbus_Connection *connection = NULL;

static Eldbus_Message *toggle_on_off(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg) {
    Eldbus_Message *reply = eldbus_message_method_return_new(msg);

    // get current value
    const char *old_value_str = eeze_udev_syspath_get_sysattr(HPWMI_SYSPATH, HPWMI_SYSATTR_ALS);
    if (old_value_str == NULL) {
        printf("eeze_udev_syspath_get_sysattr() error\n");
        eldbus_message_arguments_append(reply, "i", -1);
        return reply;
    }

    // convert string to double
    char *ptr;
    double old_value = strtod(old_value_str, &ptr);

    // toggle value form 1 to 0 or from 0 to 1
    double new_value = old_value == 0 ? 1 : 0;

    // set new value
    if (!eeze_udev_syspath_set_sysattr(HPWMI_SYSPATH, HPWMI_SYSATTR_ALS, new_value)) {
        printf("eeze_udev_syspath_set_sysattr() error\n");
        eldbus_message_arguments_append(reply, "i", -1);
        return reply;
    }

    // return new value
    eldbus_message_arguments_append(reply, "i", (int) new_value);
    return reply;
}

static Eldbus_Message *set_value(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg) {
    Eldbus_Message *reply = eldbus_message_method_return_new(msg);

    // get value from received message
    int new_value = -1;
    if (!eldbus_message_arguments_get(msg, "i", &new_value)) {
        printf("eldbus_message_arguments_get() error\n");
        eldbus_message_arguments_append(reply, "i", -1);
        return reply;
    }

    // check if value is valid
    if (new_value != 0 && new_value != 1) {
        printf("error: unknown value\n");
        eldbus_message_arguments_append(reply, "i", -1);
        return reply;
    }

    // set new value
    if (!eeze_udev_syspath_set_sysattr(HPWMI_SYSPATH, HPWMI_SYSATTR_ALS, (double) new_value)) {
        printf("eeze_udev_syspath_set_sysattr() error\n");
        eldbus_message_arguments_append(reply, "i", -1);
        return reply;
    }

    eldbus_message_arguments_append(reply, "i", new_value);
    return reply;
}

static Eldbus_Message *get_value(const Eldbus_Service_Interface *iface EINA_UNUSED, const Eldbus_Message *msg) {
    Eldbus_Message *reply = eldbus_message_method_return_new(msg);

    // get current value
    const char *current_value_str = eeze_udev_syspath_get_sysattr(HPWMI_SYSPATH, HPWMI_SYSATTR_ALS);

    // convert from string to double
    char *ptr;
    double current_value = strtod(current_value_str, &ptr);

    eldbus_message_arguments_append(reply, "i", (int) current_value);
    return reply;
}

static void on_name_request(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    // Todo: Wofür genau ist dieses Callback??? Was ist mit reply?
    unsigned int reply;

    if (eldbus_message_error_get(msg, NULL, NULL)) {
        printf("Error on on_name_request\n");
        return;
    }
    if (!eldbus_message_arguments_get(msg, "u", &reply)) {
        printf("Error geting arguments on on_name_request\n");
        return;
    }
    if (reply != ELDBUS_NAME_REQUEST_REPLY_PRIMARY_OWNER) {
        printf("Error name already in use\n");
        return;
    }
}


int main(int argc, char **argv) {
    ecore_init();
    eldbus_init();
    eeze_init();
    //=================

    connection = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
    Eldbus_Service_Interface *iface = eldbus_service_interface_register(connection, PATH, &iface_desc);
    eldbus_name_request(connection, BUS, ELDBUS_NAME_REQUEST_FLAG_DO_NOT_QUEUE, on_name_request, iface);

    ecore_main_loop_begin();

    eldbus_connection_unref(connection);

    //=================
    eeze_shutdown();
    eldbus_shutdown();
    ecore_shutdown();
    return 0;
}
