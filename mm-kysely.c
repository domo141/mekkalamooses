#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs gtk+-2.0 | sed 's/-I/-isystem '/g`
 case ${1-} in '') set x -O2 ### set x -ggdb;
	shift ;; esac;
 set -x; ${CC:-gcc} -std=c99 "$@" -o "$trg" "$0" $FLAGS
 exit
 */
#endif
/*
 * $Id; mm-kysely.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2016 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Fri 22 Jan 2016 20:22:03 +0200 too
 * Last modified: Mon 08 Feb 2016 21:36:52 +0200 too
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

// In unix-like system I've never seen execvp() fail with const argv
#define execve(a,b,c) xexecve(a,b,c)

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#undef execve
extern int execve(const char *cmd, const char *argv[], char * const envp[]);

extern char ** environ;

#define null ((void*)0)

// (variable) block begin/end -- explicit liveness...
#define BB {
#define BE }

__attribute__((sentinel)) __attribute__((noreturn))
static
pid_t exec_command(const char * command, const char * s, ...)
{
    const char * argv[10];
    argv[0] = command;
    int ai = 1;

    va_list ap;
    va_start(ap, s);

    while (s) {
        argv[ai++] = s;
        s = va_arg(ap, const char *);
        if (s == null)
            break;
        if (ai == (sizeof argv / sizeof argv[0]) - 1)
            exit(8);
    }
    va_end(ap);
    argv[ai] = null;

    (void)execve(command, argv, environ);
    fprintf(stderr, "virhe execve(%s, ...):n suorituksessa: %s\n",
	    command, strerror(errno));
    exit(77);
}

static
struct {
    int go;
} G;


static void clicked(void * b, void * d)
{
    (void)b;
    G.go = (d != null);
    gtk_main_quit();
}

static
gboolean motion_notify(void * w, GdkEventMotion * m, void * d)
{
    (void)w; (void)d;
    static guint32 t = 0;
    if (m->time - t > 15000)
	alarm(60);
    //fprintf(stderr, "motion: %d\n", m->time);
    return true;
}

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);

static GtkWidget *
tee_nappi_laatikkoon(GtkBox * box, int go, int f, const char * teksti)
{
    char buf[128];
    GtkWidget * l = gtk_label_new(null);
    // I'd use other way to have 'large'r font if I knew...
    sprintf(buf, "<span font-size=\"large\">%s</span>", teksti);
    gtk_label_set_markup(GTK_LABEL(l), buf);
    GtkWidget * b = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(b), l);
    gtk_box_pack_start(box, b, true, true, 0);
    if (f)
	gtk_widget_grab_focus(b);
    signal_connect(G_OBJECT(b), clicked, clicked, (void*)&(((char*)0)[go]));
    return b;
}

int main(int argc, char ** argv)
{
    gtk_init(&argc, &argv);
    if (argc != 2) exit(2);
    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.
    BB;
    GtkBox * vbox = GTK_BOX(gtk_vbox_new(false, 12));
    GtkBox * hbox = GTK_BOX(gtk_hbox_new(false, 12));

    GtkWidget * label = gtk_label_new(null);
    gtk_label_set_markup(GTK_LABEL(label),
	"Muistathan että voit ladata ohjelmia vain omaan käyttöön.\n"
	"Näiden ohjelmien levittäminen ilman tekijänoikeuden haltian\n"
	"lupaa <b>ei ole</b> sallittua.");
    gtk_box_pack_start(vbox, label, true, true, 0);

    tee_nappi_laatikkoon(hbox, 1, 0, "Joo muistan");
    tee_nappi_laatikkoon(hbox, 0, 1, "En muista");
    tee_nappi_laatikkoon(hbox, 0, 0, "Ei tätä");
    gtk_box_pack_start(vbox, (GtkWidget *)hbox, true, true, 0);

    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), argv[1]);
    gtk_container_set_border_width(GTK_CONTAINER(window), 12);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);

    signal_connect(window, delete-event, gtk_main_quit, null);
    signal_connect(window, destroy, gtk_main_quit, null);

    signal_connect(window, motion-notify-event, motion_notify, null);
    gtk_widget_add_events(GTK_WIDGET(window), GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(GTK_WIDGET(window), GDK_POINTER_MOTION_HINT_MASK);

    gtk_widget_show_all(window);
    BE;
    alarm(60);
    gtk_main();
    alarm(0);

    if (G.go)
	exec_command("./mm-lataaja.pl", argv[1], null);
    return 0;
}
