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
 * $Id; mm-valinta.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2016 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Wed 29 Apr 2009 20:19:00 EEST too
 * Created: Sun 14 Jun 2015 10:42:38 +0300 too
 * Last modified: Thu 17 Mar 2016 17:37:54 +0200 too
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

static void lokita_valinta(const char ** argv)
{
    time_t t = time(NULL);
    struct tm * tm = localtime(&t);
    char buf[1024];
    size_t l = strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S: Valittiin", tm);
    for (int i = 0; argv[i]; i++) {
	buf[l++] = ' ';
	const char * a = memcmp(argv[i], "./", 2)? argv[i]: argv[i] + 2;
	int al = strlen(a);
	if (l + al >= (int)sizeof buf - 4)
	    return;
	memcpy(buf + l, a, al);
	l += al;
    }
    buf[l++] = '\n';
    (void)(write(1, buf, l) == 0);
}

__attribute__((sentinel, noreturn))
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

    lokita_valinta(argv);
    (void)execve(command, argv, environ);
    fprintf(stderr, "virhe execve(%s, ...):n suorituksessa: %s\n",
	    command, strerror(errno));
    exit(77); // execve() failed
}

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);

static
struct {
    void (*cb)(const char * k);
    const char * k;
} G;

static void set_cb(void (*cb)(const char *), const char * k)
{
    gtk_main_quit();
    G.cb = cb;
    G.k = k;
}

static void bcy(const char * k)
{
    exec_command("./tv-yle.pl", k, null);
}
static void bcl(void) { gtk_main_quit(); }
static void bc1(void) { set_cb(bcy, "1"); }
static void bc2(void) { set_cb(bcy, "2"); }
static void bct(void) { set_cb(bcy, "t"); }
static void bcf(void) { set_cb(bcy, "f"); }

static void bcx(const char * x)
{
    exec_command(x, null, null);
}
static void bck(void) { set_cb(bcx, "./mm-kattele"); }
static void bcp(void) { set_cb(bcx, "./mm-selain"); }

static
GtkWidget * tee_nappi_laatikkoon(GtkBox * box, void (*cb)(void),
				 const char * teksti)
{
    char buf[128];
    GtkWidget * l = gtk_label_new(null);
    // I'd use other way to have 'x-large'r font if I knew...
    sprintf(buf, "<span font-size=\"x-large\">%s</span>", teksti);
    gtk_label_set_markup(GTK_LABEL(l), buf);
    GtkWidget * b = gtk_button_new();
    gtk_container_add(GTK_CONTAINER(b), l);
    gtk_box_pack_start(box, b, true, true, 0);
    signal_connect(b, clicked, cb, null);
    return b;
}

static void env_exists(const char * name)
{
    if (getenv(name) == null) {
	fprintf(stderr, "Ympäristömuuttujaa '%s' ei asetettu.\n", name);
	exit(1);
    }
}

int main(int argc, char ** argv)
{
    env_exists("MM_TIEDOSTOHAKEMISTO");

    for (int i = 3; i < 16; i++) close(i);
    gtk_init(&argc, &argv);
    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.
    BB;
    GtkBox * box = GTK_BOX(gtk_vbox_new(false, 12));
    tee_nappi_laatikkoon(box, bcl, "Ei sittenkään mitään");
    tee_nappi_laatikkoon(box, bc1, "Katso Yle TV 1:ta suorahkona (vlcllä)");
    tee_nappi_laatikkoon(box, bc2, "Katso Yle TV 2:ta suorahkona (vlcllä)");
    tee_nappi_laatikkoon(box, bct, "Katso Yle Teemaa suorahkona (vlcllä)");
    tee_nappi_laatikkoon(box, bcf, "Katso Yle Femmiä suorahkona (vlcllä)");
    tee_nappi_laatikkoon(box, bck, "Kattele ja poista ladattuja");
    tee_nappi_laatikkoon(box, bcp, "Plaraile muita ohjelmia");

    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "mitäs pannaan");
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "mm-ikoni.png", null);
    gtk_container_set_border_width(GTK_CONTAINER(window), 12);
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(box));
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_keep_above(GTK_WINDOW(window), true);

    signal_connect(window, delete-event, gtk_main_quit, null);
    signal_connect(window, destroy, gtk_main_quit, null);
    gtk_widget_show_all(window);
    BE;
    gtk_main();

    if (G.cb)
	G.cb(G.k);

    return 0;
}
