#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs webkit2gtk-4.0 | sed s/-I/-isystem\ /g`
 case $FLAGS in '') exec 2>&1; echo 'Empty FLAGS!'; exit 1; esac
 case ${1-} in '') set x -O2; shift; esac
 #case ${1-} in '') set x -ggdb; shift; esac
 set -x; exec ${CC:-gcc} -std=c99 "$@" -o "$trg" "$0" $FLAGS -ldl
 exit $?
 */
#endif
/*
 * $ mm-selain.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *	Copyright (c) 2015-2016 Tomi Ollila
 *	    All rights reserved
 *
 * Created: Mon 01 Jun 2015 22:19:23 EEST too // telekkarista-wkg.c
 * Created: Mon 11 Jan 2016 20:48:31 EET too // mm-selain.c
 * Created: Sat 10 Mar 2018 22:56:01 EET too // webkitgtk2
 * Last modified: Sat 17 Mar 2018 17:22:10 +0200 too
 */

// Licensed under GPLv3

#if 0 // <- set to one (1) to see these...
#pragma GCC diagnostic warning "-Wpadded"
#pragma GCC diagnostic warning "-Wpedantic"
#endif

#if __GNUC__ >= 7

#pragma GCC diagnostic error "-Wimplicit-fallthrough" // in -Wextra already ???

#define FALLTHROUGH __attribute__ ((fallthrough))
#else
#define FALLTHROUGH do {} while(0)

#endif /* __GNUC__ >= 7 */

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

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1
#define _BSD_SOURCE 1

// In unix-like system I've never seen execvp() fail with const argv
#define execve(a,b,c) xexecve(a,b,c)

#include <gtk/gtk.h>
#include <webkit2/webkit2.h>

//#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#undef execve
extern int execve(const char *cmd, const char *argv[], char * const envp[]);

// (variable) block begin/end -- explicit liveness...
#define BB {
#define BE }

#define null ((void*)0)

#if ! defined __GNUC__ || __GNUC__ < 4
#define __attribute__(x)
#endif

// let's try something different -- use '#if 0' and then go through all
// warnings & outcomment the unused lines just to see that are really unused
#if 1
#define uu(u) (void)(u)
#else
#define uu(u) (void)u; u
#endif

#if 0
#define DBG 1
#define dprintf(fmt, ...) do { \
	fprintf(stderr, "%d: %s: " fmt "\n", __LINE__,__func__, __VA_ARGS__); \
    } while (0)
#define dprints(str) fprintf(stderr, "%s\n", #str)
#define dprint0(...) do {} while (0)
#else
#define DBG 0
#define dprintf(...) do {} while (0)
#define dprints(...) do {} while (0)
#define dprint0(...) do {} while (0)
#endif

#define eprintf(...) fprintf(stderr, __VA_ARGS__)


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

const char ALKURI[] = "https://areena.yle.fi/";
//const char ALKURI[] = "http://127.1:8080/";

struct {
    regex_t preg1;
    regex_t preg2;
    WebKitWebView * web_view;
    GtkWidget * addrlw;
    char ** argv; // for execvp :/ (XXX gtk args gets lost if any)
    int reload_painettu;
} G;


__attribute__((sentinel))
static
pid_t run_command(const char * command, const char * s, ...)
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

    pid_t pid = fork();
    if (pid > 0) return pid;
    if (pid < 0) { exit(9); return 0; }
    // child //
    dprintf("execve: %s %s %s %s %s",
	    command, argv[1], argv[2], argv[3], argv[4]);

    (void)execve(command, argv, environ);
    die("execve(%s, ...) failed:", command);
}


#if 0
static gboolean sigchld_handler(void *user_data)
{
    (void)(user_data);
    pid_t pid;
    while ( (pid = waitpid(-1, null, WNOHANG)) >= 0) {
        if (pid == G.xxx_pid)
            G.xxx_pid = 0;
    }
    return true;
}
#else
static void sigchld_handler(int sig)
{
    (void)sig;
    //dz df da("): entry: sig: ") di(sig) dw;
    pid_t pid;
    int status;
    while ( (pid = waitpid(-1, &status, WNOHANG)) > 0) {
	printf("sig: %d, pid: %d, status: %d\n", sig, pid, status);
	fflush(stdout);
#if 0
        if (pid == G.xxx_pid)
            G.xxx_pid = 0;
#endif
    }
}
#endif

static void set_sigchld_handler(void)
{
    // g_unix_signal_add(SIGCHLD, sigchld_handler, null); no CHLD support there
    struct sigaction action = {
        .sa_handler = sigchld_handler,
        .sa_flags = SA_RESTART|SA_NOCLDSTOP,
    };
    sigemptyset(&action.sa_mask);
    sigaction(SIGCHLD, &action, null);
#if 0
    sigset_t oldset;
    if (sigprocmask(SIG_UNBLOCK, null, &oldset) == 0)
        eprintf("%lx", oldset.__val[0]);
#endif
}

static void * dlsym_next(const char * symbol)
{
    void * sym = dlsym(RTLD_NEXT, symbol);
    char * str = dlerror();

    if (str != null) exit(1);
    //diev(1, "finding symbol '", symbol, "' failed: ", str, null);

    return sym;
}

// Macros FTW! -- use gcc -E to examine expansion
#define _deffn2(_rt, _fn, _args) \
_rt _fn _args; \
_rt _fn _args { \
    static _rt (*_fn##_next) _args = null; \
    if (! _fn##_next ) *(void**) (&_fn##_next) = dlsym_next(#_fn);

#define _deffn1(_rt, _fn, _args) \
_rt _fn _args { \
    static _rt (*_fn##_next) _args = null; \
    if (! _fn##_next ) *(void**) (&_fn##_next) = dlsym_next(#_fn);

_deffn1(int, sigaction,
	(int signum, const struct sigaction *act, struct sigaction *oldact))
#if 0
{
#endif
    dprintf("signum: %d", signum);

    // XXX but we want *our* handler to be called !!! //
    // XXX there may be "official" way but sure I cannot find it... //
    if (signum == SIGCHLD && act && act->sa_handler != sigchld_handler) {
        if (oldact) {
            memcpy(oldact, act, sizeof *oldact);
            return 0;
        }
    }
    return sigaction_next(signum, act, oldact);
}

// from http://www.cse.yorku.ca/~oz/hash.html

static uint32_t djb2_hash(const char * str)
{
    const unsigned char * s = (const unsigned char *)str;
    uint32_t hash = 5381;
    uint32_t c;

    while (( c = *s++) != '\0')
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static void _doregcomp(regex_t * preg, const char * regex)
{
    int ec = regcomp(preg, regex, REG_EXTENDED|REG_ICASE|REG_NOSUB);
    if (ec) {
        char ebuf[1024];
        regerror(ec, preg, ebuf, sizeof ebuf);
        die("Compiling re '%s' failed: %s", regex, ebuf);
    }
}

static void doreg(void)
{
    // when these are updated, update $url in mm-lataaja.pl too
    static const char re1[] = "^"
        "https?://areena.yle.fi/[0-9]+-[0-9]+([?#].*)?"
        "$";
    static const char re2[] = "^"
        "https?://areena.yle.fi/[0-9]+-[0-9]+[?#]autoplay=";
    _doregcomp(&G.preg1, re1);
    _doregcomp(&G.preg2, re2);
}


// these macros added 2015-10-11
#define gtk_widget_(fn, wid, ...) \
    gtk_widget_ ## fn(GTK_WIDGET(wid), ##__VA_ARGS__)
#define gtk_window_(fn, wid, ...) \
    gtk_window_ ## fn(GTK_WINDOW(wid), ##__VA_ARGS__)
#define gtk_label_(fn, wid,...) \
    gtk_label_ ## fn(GTK_LABEL(wid), ##__VA_ARGS__)
#define gtk_misc_(fn, wid,...) \
    gtk_misc_ ## fn(GTK_MISC(wid), ##__VA_ARGS__)
#define gtk_container_(fn, wid,...) \
    gtk_container_ ## fn(GTK_CONTAINER(wid), ##__VA_ARGS__)
// end of 2015-10-11... //

// ...later
#define gtk_bin_(fn, wid,...) gtk_bin_ ## fn(GTK_BIN(wid), ##__VA_ARGS__)

// 2018-03-14
#define gtk_box_(fn, wid, ...) gtk_box_ ## fn(GTK_BOX(wid), ##__VA_ARGS__)
// 2018-03-15
#define gtk_grid_(fn, wid, ...) gtk_grid_ ## fn(GTK_GRID(wid), ##__VA_ARGS__)


#define set_one_property(object, name, state) \
    g_object_set (G_OBJECT(object), #name, state, null)

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data)
#define signal_connect_swapped(widget, signal, func, data) \
    g_signal_connect_swapped(widget, #signal, G_CALLBACK(func), data)
#define signal_connect0(widget, signal, func) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), null)
#define signal_connect0_swapped(widget, signal, func) \
    g_signal_connect_swapped(widget, #signal, G_CALLBACK(func), null)


static void back(void)
{
    webkit_web_view_go_back(G.web_view);
}

static void forward(void)
{
    webkit_web_view_go_forward(G.web_view);
}

static void beginning(void)
{
//    WebKitWebBackForwardList * wkbfl
//        = webkit_web_view_get_back_forward_list(G.web_view);
//    webkit_web_back_forward_list_clear(wkbfl);
    webkit_web_view_load_uri(G.web_view, ALKURI);
}

static void reload(void)
{
    G.reload_painettu = true;
    webkit_web_view_reload(G.web_view);
}

static void stopp(void)
{
    run_command("/usr/bin/pkill", "-s", "0", "php", null);
}

static void vieww(void)
{
    run_command("./mm-kattele", null, null);
}

static void restart(void /* GtkButton *button, gpointer user_data */)
{
    // all gtk fd:s are neatly close_on_exec so this cleans them up
    execvp(G.argv[0], G.argv);
    // (should) not (be) reached //
    die("execvp(%s) failed:", G.argv[0]);
}

#if 0 // all the resources that are being loaded...
static void resource_load_started(WebKitWebView     *web_view,
				  WebKitWebResource *resource
				  /*WebKitURIRequest  *request*/
				  /*gpointer           user_data*/)
{
    uu(web_view);
    const char * uri = webkit_web_resource_get_uri(resource);
    dprintf("%s", uri);
}
#endif

// temporary silence of deprecated declarations warning -- the noise
// it produces hides other potential problems...
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

static gboolean decide_navigation_policy(WebKitPolicyDecision *decision)
{
    WebKitNavigationPolicyDecision *
	navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
    /* Make a policy decision here. */
    dprints(navigation action);
#define navdec navigation_decision
    WebKitURIRequest * // deprecated but with less calls to the same stuff
	request = webkit_navigation_policy_decision_get_request(navdec);
#undef navdec
    const gchar * uri = webkit_uri_request_get_uri(request);

    int nomatch = regexec(&G.preg1, uri, 0, null, 0);

    dprintf("%s (%d)", uri, nomatch);

    static gint64 pt = 0;
    gint64 ct = g_get_real_time();
    static uint32_t ph = 0;
    uint32_t ch = djb2_hash(uri);

    int rp = G.reload_painettu;
    G.reload_painettu = false;

    if (ct - pt < 2000000) {
        if (ch == ph) {
            pt = ct;
            webkit_policy_decision_ignore(decision);
            return true;
        }
    }
    pt = ct;
    ph = ch;

    if (nomatch == 0) {
        if (rp || ! regexec(&G.preg2, uri, 0, null, 0)) {
            run_command("./mm-kysely", uri, null);
            if (! rp) {
                webkit_policy_decision_ignore(decision);
                return true;
            }
        }
    }
    gtk_label_(set_text, G.addrlw, uri);
    return false;
}

static gboolean decide_policy (WebKitWebView *web_view,
			       WebKitPolicyDecision *decision,
			       WebKitPolicyDecisionType type)
{
    uu(web_view);
    switch (type) {
    case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
	return decide_navigation_policy(decision);
	break;
    case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION: {
//	WebKitNavigationPolicyDecision *
//	    navigation_decision = WEBKIT_NAVIGATION_POLICY_DECISION (decision);
	/* Make a policy decision here. */
	dprints(new window -- no action);
	webkit_policy_decision_ignore(decision);
	return true;
    } break;

    case WEBKIT_POLICY_DECISION_TYPE_RESPONSE: {
//	WebKitResponsePolicyDecision *
//	    response = WEBKIT_RESPONSE_POLICY_DECISION (decision);
	/* Make a policy decision here. */
	dprints(response -- action);
	return false;
    } break;
    default:
	/* Making no decision results in webkit_policy_decision_use(). */
	return false;
    }
    return true;
}


static void destroy_toplevels_main_quit(void)
{
    gtk_main_quit();
}

static gboolean close_web_view_main_quit(GtkWidget * window)
{
    gtk_widget_destroy(window);
    return true;
}

// XXX shold use GtkStyleProvider -- perhaps later
static void hack_button_label_color(GtkWidget * bw)
{
    GtkWidget * lw = gtk_bin_(get_child, bw);
    GdkRGBA color = { 0.5, 0.5, 0.5, 1.0 };
    gtk_widget_override_color(lw, GTK_STATE_FLAG_NORMAL, &color);
}
static GtkWidget * hacked_gtk_label_new(const gchar * str)
{
    GtkWidget * lw = gtk_label_new(str);
    GdkRGBA color = { 0.5, 0.5, 0.5, 1.0 };
    gtk_widget_override_color(lw, GTK_STATE_FLAG_NORMAL, &color);
    return lw;
}

static GtkWidget * create_top_hbox(void)
{
    GtkWidget * hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *
    w = gtk_button_new_with_label("←");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, back, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    w = gtk_button_new_with_label("→");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, forward, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    w = gtk_button_new_with_label("↑");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, beginning, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    w = gtk_button_new_with_label("⟳");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, reload, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    gtk_box_(pack_start, hbox, hacked_gtk_label_new("│"), false, true, 0);

    w = hacked_gtk_label_new(null);
    //gtk_label_(set_xalign, w, 0.0);
    //gtk_label_(set_yalign, w, 0.5);
    gtk_widget_set_halign(w, GTK_ALIGN_START);
    gtk_widget_set_valign(w, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_start(w, 5);
    gtk_widget_set_margin_end(w, 5);
    gtk_widget_set_margin_top(w, 0);
    gtk_widget_set_margin_bottom(w, 0);
    //gtk_misc_(set_alignment, w, 0.0, 0.5); // deprecated
    //gtk_misc_(set_padding, w, 5, 1); // ditto
    gtk_label_(set_ellipsize, w, PANGO_ELLIPSIZE_MIDDLE);
    gtk_box_(pack_start, hbox, w, true, true, 0);
    G.addrlw = w;

    w = gtk_button_new_with_label("pysäytä lataus");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, stopp, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    w = gtk_button_new_with_label(" kattele ");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, vieww, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    w = gtk_button_new_with_label("uusix.");
    hack_button_label_color(w);
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, restart, null);
    gtk_box_(pack_start, hbox, w, false, true, 0);

    return hbox;
}

// 2018-03-17
#define webkit_web_view_(fn, wwv, ...) \
    webkit_web_view_ ## fn(WEBKIT_WEB_VIEW(wwv), ##__VA_ARGS__)

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);
    G.argv = argv; // needed for exevp() XXX gtk args are lost :(

    doreg();
    set_sigchld_handler();
    BB;
    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_(set_title, window, "mekkalamooses-selain");
    gtk_window_(set_icon_from_file, window, "mm-ikoni.png", null);

    // if (argc > 1 && is_geometry_string(argv[1])) ...
    gtk_window_(set_default_size, window, 1020, 720);

    WebKitWebContext * context = webkit_web_context_new_ephemeral();
    GtkWidget * web_view = webkit_web_view_new_with_context(context);
    BB;
    // all black -- no flashing! -- alpha setting seems to not matter
    GdkRGBA black = { 0, 0, 0, 1.0 };
    // deprecated, to be css'd later...
    gtk_widget_(override_background_color, window, 0, &black);
    webkit_web_view_(set_background_color, web_view, &black);
    BE BB;
    WebKitSettings * settings = webkit_web_view_(get_settings, web_view);
    webkit_settings_set_enable_fullscreen(settings, false);
    webkit_settings_set_enable_java(settings, false);
    webkit_settings_set_enable_plugins(settings, false);
    webkit_settings_set_enable_smooth_scrolling(settings, false);
    BE BB;
    GtkWidget * hbox = create_top_hbox();
    GtkWidget * vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    gtk_box_(pack_start, vbox, hbox, false, true, 0);

#if 0
    GtkWidget * grid = gtk_grid_new();
    gtk_grid_(attach, grid, hbox, 0, 0, 1, 1);
    gtk_grid_(attach, grid, web_view, 0, 1, 1, 1);
    //gtk_widget_(set_halign, web_view, GTK_ALIGN_FILL);
    //gtk_widget_(set_valign, web_view, GTK_ALIGN_FILL);
    gtk_widget_(set_vexpand, web_view, true);
    gtk_container_(add, window, grid);
#endif

    gtk_box_(pack_start, vbox, web_view, true, true, 0);
    gtk_container_(add, window, vbox);
    BE;
    signal_connect(window, destroy, destroy_toplevels_main_quit, null);
    signal_connect(window, delete-event, destroy_toplevels_main_quit, null);
    signal_connect_swapped(web_view, close, close_web_view_main_quit, window);
#if 0
    signal_connect0(web_view, resource-load-started, resource_load_started);
#endif
    signal_connect0(web_view, decide-policy, decide_policy);

    webkit_web_view_(load_uri, web_view, ALKURI);

    // make sure that when the browser area becomes visible,
    // it will get mouse and keyboard events
    gtk_widget_(grab_focus, web_view);
    G.web_view = WEBKIT_WEB_VIEW(web_view);
    gtk_widget_show_all(window);
    BE;
    gtk_main();

    return 0;
}
