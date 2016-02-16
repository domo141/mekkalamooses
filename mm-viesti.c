#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -e; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs gtk+-2.0 | sed 's/-I/-isystem '/g`
 case $1 in '') set x -O2 ### set x -ggdb;
        shift ;; esac;
 set -x; ${CC:-gcc} -std=c99 "$@" -o "$trg" "$0" $FLAGS
 exit $?
 */
#endif
/*
 * $ mm-viesti.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *      Copyright (c) 2016 Tomi Ollila
 *          All rights reserved
 *
 * Created: Sun 24 Jan 2016 12:26:05 EET too
 * Last modified: Sat 06 Feb 2016 13:37:58 +0200 too
 */

// Licensed under GPLv3

#if 0 // <- set to one (1) to see these...
#pragma GCC diagnostic warning "-Wpadded"
#pragma GCC diagnostic warning "-Wpedantic"
#endif

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"
#pragma GCC diagnostic error "-Wstrict-prototypes"
#pragma GCC diagnostic error "-Winit-self"

// -Wformat=2 ¡currently! equivalent of the following
#pragma GCC diagnostic error "-Wformat"
#pragma GCC diagnostic error "-Wformat-nonliteral"
#pragma GCC diagnostic error "-Wformat-security"
#pragma GCC diagnostic error "-Wformat-y2k"

#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic error "-Wpointer-arith"
#pragma GCC diagnostic error "-Wwrite-strings"
#pragma GCC diagnostic error "-Wcast-qual"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic error "-Wmissing-include-dirs"
#pragma GCC diagnostic error "-Wundef"
#pragma GCC diagnostic error "-Wbad-function-cast"
#pragma GCC diagnostic error "-Wlogical-op"
#pragma GCC diagnostic error "-Waggregate-return"
#pragma GCC diagnostic error "-Wold-style-definition"
#pragma GCC diagnostic error "-Wmissing-prototypes"
#pragma GCC diagnostic error "-Wmissing-declarations"
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnested-externs"
#pragma GCC diagnostic error "-Winline"
#pragma GCC diagnostic error "-Wvla"
#pragma GCC diagnostic error "-Woverlength-strings"

//ragma GCC diagnostic error "-Wfloat-equal"
//ragma GCC diagnostic error "-Werror"
//ragma GCC diagnostic error "-Wconversion"

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define null ((void*)0)

// (variable) block begin/end -- explicit liveness...
#define BB {
#define BE }

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);

int main(int argc, char ** argv)
{
    close(3); // jos auki...
    gtk_init(&argc, &argv);
    if (argc < 3)
        exit(argc + 1);
    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.
    BB;
    char buf[8192];
    char * p = buf;
    int l = sizeof buf - 50;

    for (int i = 2; i < argc; i++) {
        int al = strlen(argv[i]);
        if (al >= l)
            exit(99);
        memcpy(p, argv[i], al);
        p+= al;
        l-= al;
        *p++ = '\n';
        l--;
    }
    strcpy(p, "\n<b><u><big>Asia selvä</big></u></b>");

    GtkWidget * label = gtk_label_new(null);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
    gtk_label_set_markup(GTK_LABEL(label), buf);
    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), argv[1]);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    gtk_container_add(GTK_CONTAINER(window), label);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    signal_connect(window, delete-event, gtk_main_quit, null);
    signal_connect(window, button-press-event, gtk_main_quit, null);
    gtk_widget_add_events(GTK_WIDGET(window), GDK_BUTTON_PRESS_MASK);
    //gtk_widget_add_events(GTK_WIDGET(window), GDK_KEY_PRESS_MASK);
    gtk_widget_show_all(window);
    BE;
    for (int fd = 0; fd < 3; fd++) {
        close(fd);
        int nfd = open("/dev/null", O_RDWR, 0644);
        if (nfd != fd) {
            dup2(nfd, fd);
            close(nfd);
        }
    }
    gtk_main();
    return 0;
}
