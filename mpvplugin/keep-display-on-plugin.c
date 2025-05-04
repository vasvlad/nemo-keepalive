/****************************************************************************************
**
** Copyright (C) 2014 - 2018 Jolla Ltd.
**
** Author: Simo Piiroinen <simo.piiroinen@jollamobile.com>
**
** All rights reserved.
**
** This file is part of nemo-keepalive package.
**
** You may use this file under the terms of the GNU Lesser General
** Public License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
****************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <glib-unix.h>
#include <dbus/dbus.h>
#include "../dbus-gmain/dbus-gmain.h"
#include <keepalive-glib/keepalive-displaykeepalive.h>
#include <mpv/client.h>

#include <assert.h>

#define failure(FMT, ARGS...) do {\
    fprintf(stderr, "%s: "FMT"\n", __FUNCTION__, ## ARGS);\
    exit(EXIT_FAILURE); \
} while(0)

typedef struct UserData
{
    mpv_handle *mpv;
    GMainLoop *loop;
    gint bus_id;
    GDBusConnection *connection;
    GDBusInterfaceInfo *root_interface_info;
    GDBusInterfaceInfo *player_interface_info;
    guint root_interface_id;
    guint player_interface_id;
    const char *status;
    const char *loop_status;
    GHashTable *changed_properties;
    GVariant *metadata;
    gboolean seek_expected;
    gboolean idle;
    gboolean paused;
} UserData;

static const char *STATUS_PLAYING = "Playing";
static const char *STATUS_PAUSED = "Paused";
static const char *STATUS_STOPPED = "Stopped";
static const char *LOOP_NONE = "None";
static const char *LOOP_TRACK = "Track";
static const char *LOOP_PLAYLIST = "Playlist";


static DBusConnection *system_bus = 0;
//static GMainLoop *mainloop_handle = 0;

static void disconnect_from_systembus(void)
{
    if( system_bus )
        dbus_connection_unref(system_bus), system_bus = 0;
}

static void connect_to_system_bus(void)
{
    DBusError err = DBUS_ERROR_INIT;
    system_bus = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if( !system_bus )
        failure("%s: %s", err.name, err.message);
    dbus_gmain_set_up_connection(system_bus, 0);
    dbus_error_free(&err);
}

static gboolean event_handler(int fd, G_GNUC_UNUSED GIOCondition condition, gpointer data)
{
    UserData *ud = data;
    gboolean has_event = TRUE;

    // Discard data in pipe
    char unused[16];
    while (read(fd, unused, sizeof(unused)) > 0);

    while (has_event) {
        mpv_event *event = mpv_wait_event(ud->mpv, 0);
        switch (event->event_id) {
        case MPV_EVENT_NONE:
            has_event = FALSE;
            break;
        case MPV_EVENT_SHUTDOWN:
            //set_stopped_status(ud);
            g_main_loop_quit(ud->loop);
            break;
        default:
            break;
        }
    }

    return TRUE;
}

static gboolean emit_property_changes(gpointer data)
{
    UserData *ud = (UserData*)data;
    GError *error = NULL;
    gpointer prop_name, prop_value;
    GHashTableIter iter;
    return TRUE;
}

static void wakeup_handler(void *fd)
{
    (void)!write(*((int*)fd), "0", 1);
}

// Plugin entry point
int mpv_open_keepdisplay(mpv_handle *mpv)
{

    GMainLoop *loop;
    int pipe[2];
    GError *error = NULL;
    UserData ud = {0};
    GSource *mpv_pipe_source;

    loop = g_main_loop_new(0, 0);
    connect_to_system_bus();
    /* Create display keepalive object */
    displaykeepalive_t *displaykeepalive = displaykeepalive_new();

    fprintf(stderr, "BLOCK DISPLAY BLANKING\n");
    displaykeepalive_start(displaykeepalive);

    ud.mpv = mpv;
    ud.loop = loop;
    ud.status = STATUS_STOPPED;
    ud.loop_status = LOOP_NONE;
    ud.changed_properties = g_hash_table_new(g_str_hash, g_str_equal);
    ud.seek_expected = FALSE;
    ud.idle = FALSE;
    ud.paused = FALSE;
    
    // Run callback whenever there are events
    g_unix_open_pipe(pipe, 0, &error);
    if (error != NULL) {
        g_printerr("%s", error->message);
    }
    fcntl(pipe[0], F_SETFL, O_NONBLOCK);
    mpv_set_wakeup_callback(mpv, wakeup_handler, &pipe[1]);

    mpv_pipe_source = g_unix_fd_add(pipe[0], G_IO_IN, event_handler, &ud);

    g_main_loop_run(loop);
    /* Allow automatic dimming / blanking to happen again */
    fprintf(stderr, "ALLOW DISPLAY BLANKING\n");
    displaykeepalive_stop(displaykeepalive);

    /* Release display keepalive object */
    displaykeepalive_unref(displaykeepalive);

    disconnect_from_systembus();

    g_main_loop_unref(loop);
    return 0;
}
