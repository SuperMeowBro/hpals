//
// Created by Alexander Wecker on 06.01.16.
//

#include <Ecore_Getopt.h>
#include "hpals_client.h"

// susssa[s]a{sv}i -> u

// Todo: Fehlerbehandlung einpfelgen

static Eldbus_Connection *system_conn, *session_conn = NULL;
static Eldbus_Object *notifications_obj, *hpals_obj = NULL;
static Eldbus_Proxy *notifications_proxy, *hpals_proxy = NULL;
static u_int32_t rid = 0;
static Eina_Bool opt_enable_notififcations = EINA_FALSE;

static void call_notify(Eldbus_Proxy *proxy, const char *app_name, unsigned int replaces_id, const char *app_icon,
                        const char *summary, const char *body, int expire_timeout) {
    Eldbus_Message *msg = eldbus_proxy_method_call_new(proxy, "Notify");
    Eldbus_Message_Iter *msg_iter = eldbus_message_iter_get(msg);
    eldbus_message_iter_basic_append(msg_iter, 's', app_name);
    eldbus_message_iter_basic_append(msg_iter, 'u', replaces_id);
    eldbus_message_iter_basic_append(msg_iter, 's', app_icon);
    eldbus_message_iter_basic_append(msg_iter, 's', summary);
    eldbus_message_iter_basic_append(msg_iter, 's', body);
    // server unterstützt keine actions. container leer lassen.
    Eldbus_Message_Iter *actions_iter = eldbus_message_iter_container_new(msg_iter, 'a', "s");
    eldbus_message_iter_container_close(msg_iter, actions_iter);
    // server unterstützt keine hints. container leer lassen.
    Eldbus_Message_Iter *hints_iter = eldbus_message_iter_container_new(msg_iter, 'a', "{sv}");
    eldbus_message_iter_container_close(msg_iter, hints_iter);
    eldbus_message_iter_basic_append(msg_iter, 'i', expire_timeout);

    Eldbus_Pending *pending = eldbus_proxy_send(proxy, msg, on_notify, NULL, -1);
}

static void call_close_notification(Eldbus_Proxy *proxy, unsigned int id) {
    eldbus_proxy_call(proxy, "CloseNotification", NULL, NULL, -1, "u", id);
}

static void call_toggle_on_off(Eldbus_Proxy *proxy) {
    eldbus_proxy_call(hpals_proxy, "ToggleOnOff", on_toggle_on_off, NULL, -1, "");
}

static void on_notify(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    const char *errname = NULL;
    const char *errmsg = NULL;

    if(eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
    }

    if(!eldbus_message_arguments_get(msg, "u", &rid)) {
        printf("eldbus_message_arguments_get() error\n");
    }

    char rid_str[11]; // 10 digits + \0
    // convert uint to string
    snprintf(rid_str, sizeof(rid_str), "%u", rid);
    int len = strlen(rid_str) + 1;

    // save notification replace id in file
    Eet_File *eet_file = eet_open(RID_FILE, EET_FILE_MODE_WRITE);
    if(eet_file != NULL) {
        eet_write(eet_file, "rid", rid_str, len, 0);
        eet_close(eet_file);
    } else {
        fprintf(stderr, "Could not open '%s'\n", RID_FILE);
    }

    ecore_main_loop_quit();
}

static void on_toggle_on_off(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    const char *errname;
    const char *errmsg;

    if(eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
    }

    int32_t new_value;

    if(!eldbus_message_arguments_get(msg, "i", &new_value)) {
        printf("eldbus_message_arguments_get() error\n");
    }

    // Todo: Lokalisierung
    const char *msgtitle = "Ambient Light Sensor";
    const char *msgbody = new_value == 0 ? "ALS disabled" : "ALS enabled";

    if(opt_enable_notififcations) {
        call_notify(notifications_proxy, "HPALS", rid, "dialog-information", msgtitle, msgbody, -1);
    }
}

static u_int32_t read_rid_from_file() {
    u_int32_t rid = 0;

    // read last knwon notification replace id from file
    Eet_File *eet_file = eet_open(RID_FILE, EET_FILE_MODE_READ);
    if(eet_file != NULL) {
        int size;
        char *rid_str = eet_read(eet_file, "rid", &size);

        errno = 0;
        char *temp = NULL;
        rid = (u_int32_t) strtoul(rid_str, &temp, 10);

        if(temp == rid_str || *temp != '\0' || ((rid == LONG_MIN || rid == LONG_MAX) && errno == ERANGE)) {
            fprintf(stderr, "Could not convert '%s' to unsigned long and leftover string is: '%s'\n", rid_str, temp);
        }

        free(rid_str);
        eet_close(eet_file);

        return rid;
    } else {
        fprintf(stderr, "Could not open '%s'\n", RID_FILE);
    }

    return rid;
}

static const Ecore_Getopt options = {
        "hpals",
        "%prog [options]",
        "0.1",
        "(C) 2015-2016 Alexander 'r34' Wecker",
        "GPL2",
        "Switch HP ambient light sensors on or off.",
        0,
        {
                ECORE_GETOPT_STORE_TRUE('n', "notifications", "Show desktop notifications"),
                ECORE_GETOPT_VERSION('V', "version"),
                ECORE_GETOPT_COPYRIGHT('C', "copyright"),
                ECORE_GETOPT_LICENSE('L', "license"),
                ECORE_GETOPT_HELP('h', "help"),
                ECORE_GETOPT_SENTINEL
        }
};

int main(int argc, char **argv) {
    Eina_Bool opt_quit = EINA_FALSE;

    Ecore_Getopt_Value values[] = {
            ECORE_GETOPT_VALUE_BOOL(opt_enable_notififcations),
            ECORE_GETOPT_VALUE_BOOL(opt_quit),
            ECORE_GETOPT_VALUE_BOOL(opt_quit),
            ECORE_GETOPT_VALUE_BOOL(opt_quit),
            ECORE_GETOPT_VALUE_BOOL(opt_quit),
            ECORE_GETOPT_VALUE_NONE
    };

    if(ecore_getopt_parse(&options, values, argc, argv) < 0) {
        printf("Argument parsing failed\n");
        return 1;
    }

    if(opt_quit) {
        return 0;
    }

    //------------------------------------------

    //setlocale(LC_ALL, "");
    //bindtextdomain("hpals","/usr/share/locale");
    //textdomain("hpals");

    ecore_init();
    eldbus_init();
    eet_init();

    // read last known replace id form file
    rid = read_rid_from_file();

    // setup dbus stuff / connect
    system_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
    hpals_obj = eldbus_object_get(system_conn, HPALS_BUS, HPALS_PATH);
    hpals_proxy = eldbus_proxy_get(hpals_obj, HPALS_INTERFACE);

    session_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
    notifications_obj = eldbus_object_get(session_conn, NOTIFICATIONS_BUS, NOTIFICATIONS_PATH);
    notifications_proxy = eldbus_proxy_get(notifications_obj, NOTIFICATIONS_INTERFACE);

    // toggle als
    call_toggle_on_off(hpals_proxy);

    // start main loop
    ecore_main_loop_begin();

    // close dbus connection
    eldbus_connection_unref(session_conn);
    eldbus_connection_unref(system_conn);

    eet_shutdown();
    eldbus_shutdown();
    ecore_shutdown();

    return 0;
}
