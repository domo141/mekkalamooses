#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs gtk+-2.0 | sed 's/-I/-isystem '/g`
 case ${1-} in '') set x -O2; shift; esac
 set -x; ${CC:-gcc} -std=c99 "$@" -o "$trg" "$0" $FLAGS
 exit
 */
#endif
/*
 * $ mm-kattele.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *      Copyright (c) 2016 Tomi Ollila
 *          All rights reserved
 *
 * Created: Mon 29 Feb 2016 19:18:15 EET too
 * Last modified: Sun 17 Sep 2017 17:09:29 +0300 too
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

#define _DEFAULT_SOURCE
#define _SVID_SOURCE
#define _POSIX_SOURCE

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>

#undef execve
extern int execve(const char *cmd, const char *argv[], char * const envp[]);

extern char ** environ;

#define null ((void*)0)

// (variable) block begin/end -- explicit liveness...
#define BB {
#define BE }

#define isizeof (int)sizeof

struct {
    char * mem;
    int cnt;
    int * offs;
    pid_t cpid;
    GtkWidget * window;
#define MAX_ROWS 12
    GtkWidget * labels[MAX_ROWS];
    GtkAdjustment * adjustment;
    int rows;
    int offset;
    char * last_text;
    int last_index;
    const char * kielet;
    int kielev;
} G;

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

__attribute__ ((format(printf, 1,2), noreturn))
static
void die(const char * format, ...)
{
    int err = errno;
    va_list ap;

    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);

    if (format[strlen(format) - 1] == ':')
	fprintf(stderr, " %s\n", strerror(err));
    else
	fputc('\n', stderr);
    exit(1);
}

static pid_t xfork(void)
{
    pid_t pid = fork();
    if (pid < 0)
	die("fork() failed:");
    return pid;
}

static
void _sigact(int sig, void (*handler)(int))
{
    struct sigaction action = {
	.sa_handler = handler,
	.sa_flags = SA_RESTART|SA_NOCLDSTOP, /* NOCLDSTOP needed if ptraced */
    };
    sigemptyset(&action.sa_mask);
    sigaction(sig, &action, null);
}


static
void sigchld_handler(int sig)
{
    (void)sig;
    if (waitpid(-1, null, WNOHANG) == G.cpid)
	G.cpid = 0;
}

static gboolean present_window(void /* *p */)
{
    gtk_window_present(GTK_WINDOW(G.window));
    return false;
}

static
void sigusr1_handler(int sig)
{
    (void)sig;
    g_idle_add((GSourceFunc)present_window, null);
}

static void env_exists(const char * name)
{
    if (getenv(name) == null)
	die("Ympäristömuuttujaa '%s' ei asetettu.", name);
}

static int update_lengths(int lt[4][2], int dist, int mtl, int index)
{
    // XXX should get utf-8 length in arg... but perhaps good enough anyway.
    for (int i = 0; i < 4; i++) {
	if (mtl >= lt[i][0]) {
	    lt[i][0] = dist;
	    lt[i][1] = index;
	    for (i++; i < 4; i++) if (lt[i][0] < dist) dist = lt[i][0];
	    break;
	}
    }
    return dist;
}

static void read_vnames(int lt[4][2])
{
    int pipefd[2];
    if (pipe(pipefd) < 0)
	die("pipe() failed:");
    pid_t pid = xfork();
    if (pid == 0) {
	// child
	close(pipefd[0]);
	dup2(pipefd[1], 1);
	close(pipefd[1]);
	exec_command("./mm-kattele.pl", null, null);
    }
    // parent
    close(pipefd[1]);
#define MM_SIZE (1024 * 1024)
    // note: related values may need to be changed if above is: 65536 & 65520
    char * mem = mmap(null, MM_SIZE, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0); // XXX linux only flags
    if (mem == null)
	die("mmap() failed:");
    char * p = mem;
    int l, tl = 0;
    while ((l = read(pipefd[0], p, MM_SIZE - 65536 - tl)) > 0) {
	tl = tl + l;
	p = p + l;
    }
    if (tl >= MM_SIZE - 65536)
	die("Liikaa tietoa (eli vajaa megatavu videotiedostojen nimiä)");
#undef MM_SIZE
    close(pipefd[0]);
    wait(null);
    if (tl < 3)
	die("Ei katsottavia tiedostoja");
    if (p[-1] != '\n' || p[-2] != '/')
	die("sisällön loppu ei oletetunlainen");
    char * q = p++;
    *q = '\0';
    // align to next 16. don't dare to do & ~0xf anymore (is it safe?)
    // p = (p + 16) & ~0xf;
    if ((intptr_t)p % 16 != 0)
	p += 16 - (intptr_t)p % 16;

    int32_t * r = (int32_t *)p;
    p = mem;

    while (*p != '\n' && p < q)
	p++;
    if (p < q)
	p++;
    int i = 0;
    int mtl = 0;
    while (p[0] == '/' && p[1] == '/') {
	if (i >= 65520 / isizeof (int32_t)) // XXX a "sufficient" bounds check?
	    break;
	char *s = p + 2;
	r[i] = s - mem;
	while (*s != '/' && s < q)
	    s++;
	if (*s == '/') {
	    int dist = s - p - 1;
	    if (dist > mtl)
		mtl = update_lengths(lt, dist, mtl, i);
	    p[0] = dist / 256;
	    p[1] = dist % 256;
	    *s++ = '\0';
	}
	while (*s != '\n' && s < q) {
	    if (*s == '/')
		*s = '\0';
	    s++;
	}
	if (*s != '\n')
	    break;
	p = s + 1;
	i++;
    }
    // varmistetaan vielä uuestaan...
    if (p[-2] != '\0')
	die("sisällön loppu ei oletetunlainen");
    G.mem = mem;
    G.cnt = i;
    G.offs = r;
#if 0
    for (i = 0; i < G.cnt; i++) {
	printf("%3d %5d %s\n", i, G.offs[i], G.mem + G.offs[i]);
    }
    printf("# %d\n", G.cnt);
    exit(0);
#endif
}

static void fork_me(void)
{
    FILE * fh = popen("exec pgrep -c mm-kattele", "r");
    char buf[4] = {0,0,0,0};
    (void)fread(buf, 1, 3, fh);
    pclose(fh);
    if (isspace(buf[1]) || buf[1] == '\0') {
	if (buf[0] == '1') {
	    if (! isatty(1) && xfork())
		exit(0); // parent
	    else
		return;  // child
	}
	if (buf[0] == '2')
	    exec_command("/usr/bin/pkill", "-USR1", "mm-kattele", null);
    }
    /**/ if (isspace(buf[1])) buf[1] = '\0';
    else if (isspace(buf[2])) buf[2] = '\0';
    die("'pgrep -c mm-kattele' tulosti: '%s'", buf);
}

#define gtk_widget_(fn, wid, ...) \
    gtk_widget_ ## fn(GTK_WIDGET(wid), ##__VA_ARGS__)
#define gtk_window_(fn, wid, ...) \
    gtk_window_ ## fn(GTK_WINDOW(wid), ##__VA_ARGS__)
#define gtk_box_(fn, wid, ...) \
    gtk_box_ ## fn(GTK_BOX(wid), ##__VA_ARGS__)
#define gtk_label_(fn, wid,...) \
    gtk_label_ ## fn(GTK_LABEL(wid), ##__VA_ARGS__)
#define gtk_misc_(fn, wid,...) \
    gtk_misc_ ## fn(GTK_MISC(wid), ##__VA_ARGS__)
#define gtk_container_(fn, wid,...) \
    gtk_container_ ## fn(GTK_CONTAINER(wid), ##__VA_ARGS__)
#define gtk_frame_(fn, wid,...) \
    gtk_frame_ ## fn(GTK_FRAME(wid), ##__VA_ARGS__)


#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);
#define signal_connect_swapped(widget, signal, func, data) \
    g_signal_connect_swapped(widget, #signal, G_CALLBACK(func), data);

#define set_one_property(object, name, value) \
	g_object_set(G_OBJECT(object), #name, value, null)


static void refresh_labels(void)
{
    for (int i = 0; i < G.rows; i++)
	gtk_label_(set_text, G.labels[i], G.mem + G.offs[G.offset + i]);
}

static inline int d2i(double d) { return (int)d; }

static void adjm_changed(GtkAdjustment * adjm /*, void * d*/)
{
    int value = d2i(gtk_adjustment_get_value(adjm));
    //printf("adjm value: %d (offset %d)\n", value, G.offset);
    if (value != G.offset) {
	G.offset = value;
	refresh_labels();
    }
}

static inline void scroll_down(void)
{
    if (G.offset < G.cnt - MAX_ROWS)
	gtk_adjustment_set_value(G.adjustment, G.offset + 1.9);

}
static inline void scroll_up(void)
{
    if (G.offset > 0)
	gtk_adjustment_set_value(G.adjustment, G.offset - 1.0);

}

static gboolean key_press(void * last, GdkEventKey * k)
{
    //printf("keypress: %p %p -- ", last, k);
    //printf("%x %d, %x\n", k->keyval, k->keyval, k->state);
    if ((k->state & (0x0f)) != 0) // shift lock control mod1
	return false;
    if (last) {
	if (k->keyval == GDK_Down) {
	    scroll_down();
	    return true;
	}
    }
    else {
	if (k->keyval == GDK_Up) {
	    scroll_up();
	    return true;
	}
    }
    return false;
}

static gboolean tw_key_press(GtkWindow * tw, GdkEventKey * k)
{
    (void)tw;
    static guint32 prev_time;
    //printf("twkp: %d  %x %d  %x\n", k->time, k->keyval, k->keyval, k->state);
    if (k->keyval == 'q') {
	if (k->time - prev_time < 200) // double-q in 200 milliseconds
	    gtk_main_quit();
	else
	    prev_time = k->time;
    }
    return false;
}

static gboolean scroll_event(void * wid, GdkEventScroll * s)
{
    (void)wid;
    //printf("button scroll: %p %d\n", s, s->direction);

    /**/ if (s->direction == GDK_SCROLL_DOWN)
	scroll_down();
    else if (s->direction == GDK_SCROLL_UP)
	scroll_up();
    else
	return false;

    return true;
}


static void rb_toggled(GtkWidget * button, char * p)
{
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button));
    //printf("toggled: active: %d, p: %s\n", active, p);
    if (active)
	G.last_text = p;
}

static gboolean sulje(GtkWidget ** window /*, ... */)
{
    //printf("%s\n", __func__);
    gtk_widget_destroy(*window);
    *window = null;
    return true;
}

static pid_t
run_kattele(const char * command, const char * arg1, const char * arg2)
{
    pid_t pid = xfork();
    if (pid == 0) {
	exec_command("./mm-kattele.pl", command, arg1, arg2, null);
    }
    return pid;
}

static void move_file(GtkWidget ** window /*, ... */)
{
    //printf("%s\n", __func__);
    sulje(window);
    run_kattele("poista", G.mem + G.offs[G.last_index], null);
    wait(null);
    (G.mem + G.offs[G.last_index])[0] = '\0';
    refresh_labels();
}

static void run_player(const char * player)
{
    G.cpid = run_kattele(player, G.mem + G.offs[G.last_index], G.last_text);
}

static void play_vlc(GtkWidget ** window /*, ... */)
{
    //printf("%s\n", __func__);
    sulje(window);
    run_player("vlclla");
}

static void play_mplayer(GtkWidget ** window /*, ... */)
{
    //printf("%s\n", __func__);
    sulje(window);
    run_player("mplayerillä");
}

static void may_set_active(GtkWidget * w, const char * txt)
{
    const char * p = G.kielet;
    int i = 0;
    while (true) {
	for (int j = 0; true; j++) {
	    if (txt[j] != '\0' && txt[j] == p[j])
		continue;
	    if (txt[j] == '\0' && (p[j] == '\0' || p[j] == ',')) {
		if (i < G.kielev) {
		    G.kielev = i;
		    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), true);
		}
		return;
	    }
	    while (p[j] != '\0' && p[j] != ',')
		j++;
	    if (p[j] == '\0')
		return;
	    p += j + 1;
	    i++;
	    break;
	}
    }
}

static void clicked(void * d /*, ... */)
{
    if (G.cpid)
	return;
    static GtkWidget * window = null;
    if (window)
	sulje(&window);

    int i = (char *)d - (char *)0;
    i = i + G.offset;
    //printf("clicked: %d\n", i);
    G.last_index = i;

    unsigned char * ltxt = (unsigned char *)(G.mem + G.offs[i]);
    if (ltxt[0] == '\0')
	return;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_(set_title, window, "kattele");
    gtk_container_(set_border_width, window, 1);
    gtk_window_(set_decorated, window, false);
    gtk_window_(set_resizable, window, false);
    gtk_window_(set_position, window, GTK_WIN_POS_MOUSE);

    GtkWidget * vbox = gtk_vbox_new(false, 14);

    GtkWidget * widget = gtk_label_new((char *)ltxt);
    gtk_box_(pack_start, vbox, widget, true, true, 0);

    unsigned char * p = ltxt + ltxt[-2] * 256 + ltxt[-1];
    //printf("%d\n", *p); write(1, p-2, 7); write(1, "\n", 1);
    if (*p != '\n') {
	GtkWidget * hbox = gtk_hbox_new(false, 6);

	widget = gtk_label_new("tekstitys:");
	gtk_box_(pack_start, hbox, widget, false, false, 4);

	widget = gtk_radio_button_new_with_label(NULL, "ei");
	signal_connect(widget, toggled, rb_toggled, null);
	gtk_box_(pack_start, hbox, widget, false, false, 4);

#define rbnwlfw(w, txt) \
    gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(w), txt)
	G.kielev = 12345;
	while (*p != '\n') {
	    const char * txt = (*p == '\0')? "joo": (const char *)p;
	    widget = rbnwlfw(widget, txt);
	    signal_connect(widget, toggled, rb_toggled, p);
	    gtk_box_(pack_start, hbox, widget, false, false, 4);
	    may_set_active(widget, txt);
	    p += strlen((char *)p) + 1;
	}
#undef rbxl
	//gtk_box_(pack_start, hbox, widget, false, false, 0);
	gtk_box_(pack_start, vbox, hbox, true, true, 0);
    }

    GtkWidget * hbox = gtk_hbox_new(false, 6);
    widget = gtk_button_new_with_label("poista");
    signal_connect_swapped(widget, clicked, move_file, &window);
    gtk_box_(pack_start, hbox, widget, false, true, 0);

    widget = gtk_button_new_with_label("mplayerillä");
    signal_connect_swapped(widget, clicked, play_mplayer, &window);
    gtk_box_(pack_end, hbox, widget, false, true, 0);

    widget = gtk_button_new_with_label("kato vlcllä");
    signal_connect_swapped(widget, clicked, play_vlc, &window);
    gtk_box_(pack_end, hbox, widget, false, true, 0);

    widget = gtk_button_new_with_label("jotain muuta");
    signal_connect_swapped(widget, clicked, sulje, &window);
    GtkWidget * grab_widget = widget;
    gtk_box_(pack_end, hbox, widget, false, true, 0);

    gtk_box_(pack_start, vbox, hbox, true, true, 0);

    // gtk3 gtkwidget would have 'margin' properties but gtk2 don't
    GtkWidget * frame1 = gtk_frame_new(null);
    gtk_container_(set_border_width, frame1, 4);
    gtk_frame_(set_shadow_type, frame1, GTK_SHADOW_NONE);
    GtkWidget * frame = gtk_frame_new(null);
    //gtk_container_(set_border_width, frame, 4);
    gtk_frame_(set_shadow_type, frame, GTK_SHADOW_ETCHED_OUT);

    gtk_container_(add, frame1, vbox);
    gtk_container_(add, frame, frame1);
    gtk_container_(add, window, frame);

    signal_connect(window, delete-event, sulje, null);
    gtk_window_(set_transient_for, window, GTK_WINDOW(G.window));
    gtk_widget_show_all(window);
    gtk_widget_grab_focus(grab_widget);
}

static int label_width(int lt[4][2])
{
    // return 150;
    if (lt[0][0] == 0) // unlikely
	return 200;

    GtkWidget * label = gtk_label_new(G.mem + G.offs[lt[0][1]]);
    int width = 0;
    int i = 0;
    while (true) {
	GtkRequisition req;
	gtk_widget_size_request(label, &req);
	// printf("%d %d %d %d %d\n", i,lt[i][0],lt[i][1],req.width,width);
	if (req.width > width)
	    width = req.width;
	i++;
	if (i >= 4)
	    break;
	gtk_label_(set_text, label, G.mem + G.offs[lt[i][1]]);
    }
    gtk_widget_destroy(label);
    return width > 900? 900: (width < 80? 80: width);
}

int main(int argc, char ** argv)
{
    env_exists("MM_TIEDOSTOHAKEMISTO");
    fork_me();
    _sigact(SIGCHLD, sigchld_handler);
    _sigact(SIGUSR1, sigusr1_handler);
    int lw;
    BB;
    int lt[4][2];
    memset(lt, 0, sizeof lt);
    read_vnames(lt);

    G.kielet = getenv("MM_KATTELE_KIELET");
    if (G.kielet == null)
	G.kielet = "ei";

    gtk_init(&argc, &argv);
    lw = label_width(lt);
    BE;
    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.

    G.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_(set_title, G.window, "kattele");
    gtk_container_(set_border_width, G.window, 1);
    //gtk_window_(set_default_size, G.window, 980, 600);
    gtk_window_(set_resizable, G.window, false);
    gtk_window_(set_position, G.window, GTK_WIN_POS_CENTER);
    gtk_window_(set_icon_from_file, G.window, "mm-harmaa.png", null);
    BB;
    GtkWidget * hbox = null;
    G.rows = G.cnt;
    if (G.cnt > MAX_ROWS) {
	G.adjustment = GTK_ADJUSTMENT(
	    gtk_adjustment_new(0, 0, G.cnt + 0.9999, 1,MAX_ROWS,MAX_ROWS));
	GtkWidget * scrollbar = gtk_vscrollbar_new(G.adjustment);
	hbox = gtk_hbox_new(false, 2);
	gtk_box_(pack_end, hbox, scrollbar, false, false, 0);
	G.rows = MAX_ROWS;
	signal_connect(G.adjustment, value-changed, adjm_changed, null);
    }

    signal_connect(G.window, delete-event, gtk_main_quit, null);
    signal_connect(G.window, destroy, gtk_main_quit, null);
    signal_connect(G.window, key-press-event, tw_key_press, null);

    GtkWidget * fbox = gtk_vbox_new(false, 0);

    GtkWidget * button;
    for (int i = 0; i < G.rows; i++) {
	button = gtk_button_new();
	set_one_property(button, relief, GTK_RELIEF_NONE);
	GtkWidget * label = gtk_label_new(G.mem + G.offs[i]);
	set_one_property(label, width-request, lw);
	gtk_container_(add, button, label);
	gtk_misc_(set_alignment, label, 0.0, 0.5);
	gtk_box_(pack_start, fbox, button, true, true, 0);
	G.labels[i] = label;
	signal_connect_swapped(button, clicked, clicked, &((char *)0)[i]);
	if (i == 0 && hbox)
	    signal_connect_swapped(button, key-press-event, key_press, null);
    }
    if (hbox) {
	signal_connect_swapped(button, key-press-event, key_press, (void *)1);
	signal_connect(G.window, scroll-event, scroll_event, null);
	gtk_widget_add_events(G.window, GDK_SCROLL_MASK);

	gtk_box_(pack_start, hbox, fbox, false, false, 0);
	fbox = hbox;
    }
    gtk_container_(add, G.window, fbox);
    BE;
    gtk_widget_show_all(G.window);
    gtk_main();
    return 0;
}
