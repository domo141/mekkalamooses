#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs gtk+-2.0 | sed 's/-I/-isystem '/g`
 test -d .git && VS=`git log -1 --format=%ci,%h | sed 's/ .*,/-g/'` || VS=evt.
 case ${1-} in '') set x -O2 ### set x -ggdb;
	shift ;; esac;
 set -x; ${CC:-gcc} -std=c99 "$@" -o "$trg" "$0" $FLAGS -DVS="\"$VS\""
 exit $?
 */
#endif
/*
 * $ mm-tausta.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2012-2016 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Tue 25 Sep 2012 22:10:56 EEST too
 * Last modified: Sun 07 Feb 2016 22:06:54 +0200 too
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


#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>

#include <gtk/gtk.h>
#include <cairo.h>

#include <stdbool.h>

#define null ((void*)0)

// No internal defines. else not recommended.
#define unless(x) if (!(x))

// (variable) block begin/end -- explicit liveness...
#define BB {
#define BE }

// at least we try to achieve these.
#define TLINES 30
#define TCOLUMNS 80

struct {
    const char * progname;
    int cpid;
    int cfd;
    int usr1;
    int valo;
    char hold;
    cairo_t * cr;
    double top, advance, height; // these *should* be 14, 9 & 17
    double lineheight; // XXX document...
    int x, y;
    int drawstate;
    const char * thd;
    int logsize;

    GdkPixmap * pixmap;
    GtkWidget * image;
} G;

static void verrf(const char * format, va_list ap)
{
    int error = errno; /* XXX is this too late ? */
    vfprintf(stderr, format, ap);
    if (format[strlen(format) - 1] == ':')
	fprintf(stderr, " %s\n", strerror(error));
    else
	fputs("\n", stderr);
}

static __attribute__((noreturn))
void vdie(const char * format, va_list ap)
{
    fprintf(stderr, "%s: ", G.progname);
    verrf(format, ap);
    exit(1);
}

static __attribute__((noreturn))
void die(const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    vdie(format, ap);
    va_end(ap);
}

static
int run_prog(char * argv[])
{
    int pfd[2];

    if (pipe(pfd) < 0)
	die("pipe:");

    int pid = fork();
    if (pid > 0) {
	close(pfd[1]);
	G.cpid = pid;
	return pfd[0];
    }
    if (pid < 0)
	die("fork:");
    /* child */
    close(99);
    close(pfd[0]);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    execvp(argv[0], argv);
    die("execvp:");
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
    if (G.cpid && waitpid(G.cpid, null, WNOHANG) == G.cpid)
	G.cpid = 0;
}
static
void sigusr1_handler(int sig)
{
    (void)sig;
    G.usr1 = 1;
}

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);
#define signal_connect_swapped(widget, signal, func, data) \
    g_signal_connect_swapped(widget, #signal, G_CALLBACK(func), data);


static void windowp_close(GtkWidget ** window)
{
    gtk_widget_destroy(*window);
    *window = null;
}

#define gtk_window_(fn, wid, ...) \
    gtk_window_ ## fn(GTK_WINDOW(wid), ##__VA_ARGS__)

static void move_bmw(GtkWidget * cqw, GdkRectangle * a, GtkWidget * mainwin)
{
    GdkWindow * gdkwindow = gtk_widget_get_window(mainwin);
    gint mx, my, mwx, mwy;
    gdk_window_get_pointer(gdkwindow, &mx, &my, null);
    gdk_window_get_position(gdkwindow, &mwx, &mwy);
    GtkAllocation mwa;
    gtk_widget_get_allocation(mainwin, &mwa);
#if 0
    dz dix(mx) cmm dix(my) cmm
        dix(mwa.width) cmm dix(mwa.height) cmm dix(mwx) cmm dix(mwy) dw;
    dz dix(a->width) cmm dix(a->height) cmm dix(a->x) cmm dix(a->y) dw;
#endif
    // 32, just arbitrary value (this part copied from mm-selain...)
    if (abs(mx - mwa.width) < 32 && abs(my) < 32) {
        gtk_window_(move, cqw, mwx + mwa.width - a->width - 0, mwy + 0);
    }
    else if (abs(mx) < 32 && abs(my) < 32) {
        gtk_window_(move, cqw, mwx + 0, mwy + 0);
    }
    // else don't move //
}


static void buttonmenu(GtkWidget * mainwin)
{
    static GtkWidget * window = null;
    if (window) {
	//gtk_widget_destroy(window);
	//window = null;
	return;
    }
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "vaiheessa");
    gtk_window_set_decorated(GTK_WINDOW(window), false);
    gtk_container_set_border_width(GTK_CONTAINER(window), 4);
    GtkWidget * label = gtk_label_new("Hieno on nappula, mutta\n"
				      "tää ei tee (vielä) mitään.");
    GtkWidget * frame = gtk_frame_new(null);
    gtk_misc_set_padding(GTK_MISC(label), 4, 4);
    gtk_container_add(GTK_CONTAINER(frame), label);
    gtk_container_add(GTK_CONTAINER(window), frame);
    //gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
    signal_connect_swapped(window, delete-event, windowp_close, &window);
    signal_connect_swapped(window, button-press-event, windowp_close, &window);
    gtk_widget_add_events(GTK_WIDGET(window), GDK_BUTTON_PRESS_MASK);
    signal_connect_swapped(window, focus-out-event, windowp_close, &window);

    signal_connect(window, size-allocate, move_bmw, mainwin);
    //gtk_window_(set_transient_for, window, GTK_WINDOW(mainwin));
    gtk_widget_show_all(window);
    gtk_window_present(GTK_WINDOW(window));
}

static
gboolean button_press(GtkWidget * w, void * e, void * d)
{
    (void)e; (void)d;
    //printf("button: valo: %d\n", G.valo);
    if (G.valo == 1) {
	buttonmenu(w);
	G.valo = 0;
    }
    if (G.cpid == 0)
	gtk_main_quit();
    return true;
}

static
gboolean key_press(void * w, GdkEventKey * k, void * d)
{
    (void)w; (void)d;
    if (G.cpid == 0)
	gtk_main_quit();
    else if (k->keyval == 'c' && k->state & GDK_CONTROL_MASK)
	gtk_widget_hide(w);
    return true;
}

static
gboolean timeout_valo0(void * p)
{
    (void)p;
    //printf("timeout_valo0!\n");
    G.valo = 0;
    return false;
}

static void draw_btn(int qd);

static
gboolean leave_notify(void * w, GdkEvent * e, void * d)
{
    (void)w; (void)e; (void)d;
    //printf("leave!\n");
    if (G.valo) {
	G.valo = 0;
	draw_btn(1);
	G.valo = 1;
        g_timeout_add(100, timeout_valo0, null);
    }
    return true;
}

#if 0
static
gboolean enter_notify(void * w, GdkEvent * e, void * d)
{
    (void)w; (void)e; (void)d;
    printf("enter!\n");
    return true;
}
#endif

static
gboolean motion_notify(void * w, GdkEventMotion * m, void * d)
{
    (void)w; (void)d;
    if (G.valo) {
	if (m->x < 710.0 || m->y > 20.0) {
	    //printf("motion: >0 %lf %lf\n", m->x, m->y);
	    G.valo = 0;
	    draw_btn(1);
	}
    }
    else {
	if (m->x > 711.0 && m->y < 19.0) {
	    //printf("motion: >1 %lf %lf\n", m->x, m->y);
	    G.valo = 1;
	    draw_btn(1);
	}
    }
    return true;
}


static
void init_G(const char * progname)
{
    memset(&G, 0, sizeof G);
    G.progname = progname;
    G.y = 1;
}

gboolean read_process_input(GIOChannel * source,
			    GIOCondition condition, gpointer data);

static
void tee_tilaikoni(void * w)
{
    //dz df dfc dnl dw;
    GtkStatusIcon * gsi = gtk_status_icon_new_from_file("mm-ikoni.png");
    signal_connect_swapped(gsi, button-press-event, gtk_window_present, w);
    gtk_status_icon_set_visible(gsi, true);
}

#define set_text_color(cr)  cairo_set_source_rgb((cr), 0.6, 0.3, .9)

#define LBW 4.0 // left border width
#define TBW 4.0 // top border width

static void log_open(void);

int main(int argc, char * argv[])
{
    init_G(argv[0]);
    gtk_init(&argc, &argv);
    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.
    BB;
    const char * title = null;
    for (; argc >= 2; argc--, argv++) {
	if (argv[1][0] != '-')
	    break;
	if (strcmp (argv[1], "--") == 0) {
	    argc--;
	    argv++;
	    break;
	}
	if (strcmp (argv[1], "-title") == 0) {
	    title = argv[2];
	    argc--; argv++;
	    continue;
	}
	if (strcmp (argv[1], "-hold") == 0) {
	    G.hold = true;
	    continue;
	}
	die("unrecognized option '%s'\n", argv[1]);
    }

    if (title == null)
	die("title not set (option -title)");

    if (argc < 2)
	die("No command to execute");

    GtkWidget * window =  gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkColor color = { .red = 0, .green = 0, .blue = 0 };
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
    gtk_window_set_title(GTK_WINDOW(window), title);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "mm-ikoni.png", null);

    signal_connect(window, delete-event, gtk_widget_hide, null);

    tee_tilaikoni(window);
    gtk_widget_realize(window);

#if 0
    cairo_surface_t * pm = gdk_window_create_similar_surface
	(window->window, CAIRO_CONTENT_COLOR, int 728, 548);
#endif
    G.pixmap = gdk_pixmap_new(window->window, 728, 548, -1);
    G.image = gtk_image_new_from_pixmap(G.pixmap, null);

    gtk_container_add(GTK_CONTAINER(window), G.image);

    G.cr = gdk_cairo_create(G.pixmap);

    cairo_set_source_rgb(G.cr, 0.0, 0.0, 0.2);
    cairo_paint(G.cr);

    set_text_color(G.cr);
    cairo_select_font_face(G.cr, "monospace",
			   CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    // attempt for 80x30, 9 pixels wide glyphs, 18 pixels line height (720x540)
    double trysize = 15.0;
    cairo_set_font_size(G.cr, trysize);

    cairo_font_extents_t fe;
    cairo_font_extents(G.cr, &fe);
    if (fe.max_x_advance < 9 && (fe.ascent + fe.descent) < 18) {
	trysize += 5.0;
	cairo_set_font_size(G.cr, trysize);
	cairo_font_extents(G.cr, &fe);
    }
    while (fe.max_x_advance > 9 || (fe.ascent + fe.descent) > 18) {
	trysize -= 1.0;
	cairo_set_font_size(G.cr, trysize);
	cairo_font_extents(G.cr, &fe);
	if (trysize < 5.0)
	    break;
    }

    G.top = fe.ascent + TBW;
    G.advance = fe.max_x_advance;
    G.lineheight = fe.ascent + fe.descent;

    draw_btn(0);

    cairo_move_to(G.cr, LBW, G.top + G.y * G.lineheight);

    cairo_show_text(G.cr, "mekkalamooseksen versio on ");
    cairo_show_text(G.cr, VS);
    BB;
    const char * thd = getenv("MM_TIEDOSTOHAKEMISTO");
    if (thd == null)
	die("MM_TIEDOSTOHAKEMISTO puuttuu ympäristöstä");
    G.thd = thd;
    for (int i = 0; thd[i]; i++) {
	// avoid potential for invalid utf-8
	if (thd[i] & 0x80) {
	    thd = null;
	    break;
	}
    }
    if (thd != null) {
	cairo_move_to(G.cr, LBW, G.top + (++G.y) * G.lineheight);
	cairo_show_text(G.cr, "tiedostohakemisto on ");
	cairo_show_text(G.cr, thd);
    }
    G.y++;
    cairo_move_to(G.cr, LBW, G.top + (++G.y) * G.lineheight);
    BE;
#if 0
    printf("%f %f %f %f %f\n", fe.ascent, fe.descent,
	   fe.height, fe.max_x_advance, fe.max_y_advance);
#endif
    gtk_widget_show_all(window);

    _sigact(SIGCHLD, sigchld_handler);
    _sigact(SIGUSR1, sigusr1_handler);

    G.cfd = run_prog(&argv[1]);

    GIOChannel * iochannel = g_io_channel_unix_new(G.cfd);

    // G.iosrc =
    g_io_add_watch(iochannel, G_IO_IN | G_IO_HUP, read_process_input, window);

    signal_connect(window, button-press-event, button_press, null);
    signal_connect(window, key-press-event, key_press, null);
    signal_connect(window, leave-notify-event, leave_notify, null);
#if 0
    signal_connect(window, enter-notify-event, enter_notify, null);
#endif
    signal_connect(window, motion-notify-event, motion_notify, null);
    gtk_widget_add_events(window, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
    gtk_widget_add_events(window, GDK_LEAVE_NOTIFY_MASK);
    gtk_widget_add_events(window, GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(window, GDK_POINTER_MOTION_HINT_MASK);

    log_open();
    BE;
    gtk_main();

    return 0;
}

/* // just that I don't forget...

static void close_connection(void)
{
    if (G.sd < 0)
	return;
    //g_io_channel_close(G.ioc); // closes G.sd //
    g_io_channel_shutdown(G.ioc, false, null); // closes G.sd //

    // I (once) wasted fscking 2 hours for figuring this out !!!
    // Why on earth cannot the unref/shutdown drop this from poll sets ???
    g_source_remove(G.iosrc);

    G.sd = -1;
}
*/

static void draw_btn(int qd)
{
    //printf("draw_btn (%d)!\n", G.valo);
    //cairo_set_line_width(G.cr, 0.2);
    if (G.valo)
	cairo_set_source_rgb(G.cr, 0.5, 0.5, 0.8);
    else
	cairo_set_source_rgb(G.cr, 0.4, 0.4, 0.7);
    cairo_rectangle(G.cr, 730 - LBW - 16, TBW - 2, 16, 16);
    cairo_fill(G.cr);
    set_text_color(G.cr);
    if (qd)
	gtk_widget_queue_draw(G.image);
}

static
void clear_line(void)
{
#if 0
    static int f = 1, g = 0;
    cairo_set_source_rgb(G.cr, 0.5 * (f++&1), 0.5 * (g++&1), 0.2);
#else
    cairo_set_source_rgb(G.cr, 0.0, 0.0, 0.2);
#endif
    cairo_rectangle(G.cr, LBW, TBW + G.y * G.lineheight, 720, G.lineheight);
    cairo_fill(G.cr);
    set_text_color(G.cr);
    cairo_move_to(G.cr, LBW, G.top + G.y * G.lineheight);
}
static
void maybe_scroll(void)
{
    //printf("maybe scroll: y: %d\n", G.y);

    if (G.y == TLINES - 1) {
	// for data copying -- using deprecated function -- as attempting
	// to play with window's GdkWindow is even more dangerous...
	// let's see how scroll works... (overlapping surfaces...)
	gdk_cairo_set_source_pixmap(G.cr, G.pixmap, 0.0, -G.lineheight);

	cairo_rectangle(G.cr, LBW, TBW, 720, 540 - G.lineheight);
	cairo_fill(G.cr);
	set_text_color(G.cr);
	clear_line();
	draw_btn(0);
    }
    else
	G.y++;

    cairo_move_to(G.cr, LBW, G.top + G.y * G.lineheight);
}
static
void draw_text(char * buf, int l)
{
    if (G.drawstate == 2)
	maybe_scroll();
    if (l > 0) {
	switch (G.drawstate) {
	case 3: maybe_scroll(); break;
	case 1: clear_line();	break;
	}
	char c = buf[l];
	buf[l] = '\0';
	//printf("draw (len %d): %s\n", l, buf);
	cairo_show_text(G.cr, buf);
	gtk_widget_queue_draw(G.image);
	buf[l] = c;
    }
#if 0
    else /* l == 0 and */ if (G.drawstate == 3)
	G.drawstate = 1;
#endif
    G.drawstate = 0;
}

static void log_open(void)
{
    char buf[1024];
    if (snprintf(buf, sizeof buf, "%s/kesken/loki.txt", G.thd) >= 1020)
	die("MM_TIEDOSTOHAKEMISTO '%s' too long", G.thd);
    int fd = open(buf, O_WRONLY|O_CREAT|O_APPEND, 0644);
    if (fd < 0)
	die("Opening '%s' failed:", buf);
    if (fd != 99) {
	dup2(fd, 99);
	close(fd);
    }
    struct stat st;
    if (fstat(3, &st) < 0)
	die("fstat() failed:");
    G.logsize = st.st_size;
}

static void log_rotate(void)
{
    char obuf[1024];
    char nbuf[1024];
    close(3);
    snprintf(obuf, sizeof obuf, "%s/kesken/loki.txt", G.thd);
    snprintf(nbuf, sizeof nbuf, "%s/kesken/loki.ed.txt", G.thd);
    rename(obuf, nbuf);
    int fd = open(obuf, O_WRONLY|O_CREAT|O_APPEND, 0644);
    if (fd < 0)
	die("Opening '%s' failed:", obuf);
    if (fd != 99) {
	dup2(fd, 99);
	close(fd);
    }
    G.logsize = 0;
}


static void log_chr(char c)
{
    static char buf[1024];
    static int pos = 0;
    static int lpos = 0;

    if (c == '\n') {
	if (lpos > pos) pos = lpos;
	buf[pos++] = '\n';
	(void)(write(99, buf, pos) == 0);
	G.logsize += pos;
	if (G.logsize > 65536)
	    log_rotate();
	pos = lpos = 0;
	return;
    }
    if (c == '\r') {
	if (pos > lpos) lpos = pos;
	pos = 0;
	return;
    }
    if (pos > 1020) {
	buf[pos++] = '\n';
	(void)(write(99, buf, pos) == 0);
	G.logsize += pos;
	if (G.logsize > 65536)
	    log_rotate();
	pos = lpos = 0;
    }
    buf[pos++] = c;
}

gboolean read_process_input(GIOChannel * source,
			    GIOCondition condition, gpointer data)
{
    (void)source;
    (void)condition;

    if (G.usr1) {
	gtk_window_present(GTK_WINDOW(data));
	G.usr1 = 0;
    }

    char buf[1024];

    int len = read(G.cfd, buf, sizeof buf);
    if (len <= 0) {
	log_chr('\n');
	unless (G.hold)
	    gtk_main_quit();
	/* no more data read -- and too shy to close fd */
	return false;
    }

    //printf("input text: len %d, text '%.*s'\n", len, len, buf);

    /* and now the fun begins -- render text to image */
    /* utf8 counting not implemented -- yet */
    int x = G.x;
    char * p = buf;
    int o = 0;
    while (len > 0) {
	len--;
	char c = p[o];
	log_chr(c);
	//fprintf(stderr, "%c", c);
	switch (c) {
	case '\n':
	    x = 0;
	    //printf("NL: %d: '%.*s'\n", o, o, p);
	    draw_text(p, o);
	    p += (o + 1);
	    G.drawstate = 2;
	    o = 0;
	    continue;
	case '\r':
	    //printf("CR: %d: '%.*s'\n", o, o, p);
	    x = 0;
	    int prevdrawstate = G.drawstate;
	    draw_text(p, o);
	    p += (o + 1);
	    if (o != 0 || prevdrawstate != 2)
		G.drawstate = 1;
	    o = 0;
	    continue;
	}
	if (x < 79) {
	    x++;
	    o++;
	    continue;
	}
	if (x == 79) {
	    x = 80;
	    // XXX quick&dirty eol half-utf8 avoidance
	    if (p[o] & 0x80) {
		int q = o - 1;
		while ((p[q] & 0xc0) == 0x80)
		    q--;
		if ((p[q] & 0x80) == 0)
		    q++;
		p[q] = '\0';
	    }
	    o++;
	    //printf("79: %d: '%.*s'\n", o, o, p);
	    draw_text(p, o);
	    p += o;
	    G.drawstate = 3;
	    o = 0;
	    continue;
	}
	if (isspace(c) || (c & 0xc0) == 0x80)
	    // leading whitespace or XXX utf8 continuation bytes
	    ;
	else {
	    x = 1;
	    o = 1;
	    continue;
	}
	// and last, drop trailing whitespace (o == 0)
	p++;
    }
    G.x = x;
    if (o) {
	//printf("rest: %d: '%.*s'\n", o, o, p);
	draw_text(p, o);
    }
    return true;
}
