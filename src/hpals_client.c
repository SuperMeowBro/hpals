//
// Created by Alexander Wecker on 06.01.16.
//

#include <Eina.h>
#include <Eet.h>
#include <Eldbus.h>
#include <Ecore.h>
#include "hpals_client.h"

// susssa[s]a{sv}i -> u

// Todo: Fehlerbehandlung einpfelgen

static Eldbus_Connection *system_conn, *session_conn = NULL;
static Eldbus_Object *notifications_obj, *hpals_obj = NULL;
static Eldbus_Proxy *notifications_proxy, *hpals_proxy = NULL;
static u_int32_t rid = 0;

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
    const char *errname;
    const char *errmsg;
    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
    }

    if (!eldbus_message_arguments_get(msg, "u", &rid)) {
        printf("eldbus_message_arguments_get() error\n");
    }

    char rid_str[128];
    eina_convert_itoa(rid, rid_str);

    Eet_File *eet_file;
    eet_file = eet_open(".config/hpals", EET_FILE_MODE_WRITE);
    if (eet_file != NULL) {
        eet_write(eet_file, "rid", rid_str, strlen(rid_str) + 1, 0);
        eet_close(eet_file);
    }

    ecore_main_loop_quit();
}

static void on_toggle_on_off(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    const char *errname;
    const char *errmsg;
    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
    }

    int32_t r;
    if (!eldbus_message_arguments_get(msg, "i", &r)) {
        printf("eldbus_message_arguments_get() error\n");
    }

    // Todo: Lokalisierung
    const char *body = r == 0 ? "ALS is disabled now" : "ALS is enabled now";

    // Todo: Korrektes icon verwenden!
    call_notify(notifications_proxy, "HPALS", rid, "dialog-information", "Ambient Light Sensor", body, -1);
}

static unsigned int read_rid_from_file() {
    int size;
    char *rid_str, *ptr;
    unsigned int rid_conv = 0;

    Eet_File *eet_file = eet_open(".config/hpals", EET_FILE_MODE_READ);
    if (eet_file != NULL) {
        rid_str = eet_read(eet_file, "rid", &size);
        // Todo: Konvertierung überarbeiten.
        rid_conv = (unsigned int) strtod(rid_str, &ptr);

        free(rid_str);
        eet_close(eet_file);

        return rid_conv;
    }

    return rid_conv;
}

int main(int argc, char **argv) {
    ecore_init();
    eldbus_init();
    eet_init();
    //********************

    rid = read_rid_from_file();

    system_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
    hpals_obj = eldbus_object_get(system_conn, HPALS_BUS, HPALS_PATH);
    hpals_proxy = eldbus_proxy_get(hpals_obj, HPALS_INTERFACE);

    session_conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SESSION);
    notifications_obj = eldbus_object_get(session_conn, NOTIFICATIONS_BUS, NOTIFICATIONS_PATH);
    notifications_proxy = eldbus_proxy_get(notifications_obj, NOTIFICATIONS_INTERFACE);

    call_toggle_on_off(hpals_proxy);

    ecore_main_loop_begin();

    eldbus_connection_unref(session_conn);
    eldbus_connection_unref(system_conn);

    //********************
    eet_shutdown();
    eldbus_shutdown();
    ecore_shutdown();

    return 0;
}
