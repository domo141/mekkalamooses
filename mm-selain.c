#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -eu; trg=${0%.c}; rm -f "$trg"
 FLAGS=`pkg-config --cflags --libs gtk+-2.0 webkit-1.0 | sed s/-I/-isystem\ /g`
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
 *      Copyright (c) 2015-2016 Tomi Ollila
 *          All rights reserved
 *
 * Created: Mon 01 Jun 2015 22:19:23 EEST too // telekkarista-wkg.c
 * Created: Mon 11 Jan 2016 20:48:31 EET too // mm-selain.c
 * Last modified: Wed 13 Sep 2017 20:19:42 +0300 too
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

#define _DEFAULT_SOURCE 1
#define _GNU_SOURCE 1
#define _BSD_SOURCE 1

// In unix-like system I've never seen execvp() fail with const argv
#define execve(a,b,c) xexecve(a,b,c)
//#define open xopen // in this case varargs...

#define access(p, m) xaccess(p, m) // p has nonnull attribute which broke somet

#include <gtk/gtk.h>
#include <webkit/webkit.h>

//#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <ctype.h>
#include <regex.h>
//#include <fcntl.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#undef execve
extern int execve(const char *cmd, const char *argv[], char * const envp[]);

#undef access
extern int access(const char *pathname, int mode);

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

#if 1
#define DBG 1
//#warning dbg set
static char dbuf[32768];
#define dz do { char * dptr = dbuf; int dbgl; (void)dbgl; \
    di(__LINE__) dc(':') spc
#define da(a) { memcpy(dptr, a, sizeof a - 1); dptr += sizeof a - 1; }
#define ds(s) if (s) { dbgl = strlen(s); memcpy(dptr, s, dbgl); dptr += dbgl; \
    } else da("(null)")

#define dc(c) *dptr++ = c;
#define spc *dptr++ = ' ';
#define dot *dptr++ = '.';
#define cmm *dptr++ = ','; spc
#define cln *dptr++ = ':'; spc
#define dnl *dptr++ = '\n';
#define du(u) dptr += sprintf(dptr, "%lu", (unsigned long)i);
#define di(i) dptr += sprintf(dptr, "%ld", (long)i);
#define dx(x) dptr += sprintf(dptr, "%lx", (long)x);

#define df da(__func__) dc('(')
#define dfc dc(')')
#define dss(s) da(#s) da(": \"") ds(s) dc('"')
#define dsi(i) da(#i) da(": ") di(i)

#define dw  dnl int _z=write(2, dbuf, dptr - dbuf); (void)(_z=_z); } while (0)
#define dws spc int _z=write(2, dbuf, dptr - dbuf); (void)(_z=_z); } while (0)

#else

#define DBG 0
#define dz do {
#define da(a)
#define ds(s)
#define dc(c)
#define spc
#define dot
#define cmm
#define cln
#define dnl
#define du(u)
#define di(i)
#define dx(x)

#define df
#define dfc
#define dss(s)
#define dsi(i)

#define dw } while (0)
#define dws } while (0)
#endif
#define dw0 } while (0)

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

const char ALKURI[] = "http://areena.yle.fi/";

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
#if 0
    dz da("execve: ") ds(command) spc ds(argv[1]) spc ds(argv[2])
        spc ds(argv[3]) spc ds(argv[4])
        dw;
#endif
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
        dz df da("): sig: ") di(sig)
            da(", pid: ") di(pid) da(", status: ") di(status) dw;
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

    if (str != null)
        exit(1);
    //diev(1, "finding symbol '", symbol, "' failed: ", str, null);

    return sym;
}
#define set_next(name) *(void**)(&name##_next) = dlsym_next(#name)

int access(const char * pathname, int mode)
{
    static int (*access_next)(const char *, int) = null;
    if (! access_next)
        set_next(access);

    //dz df da("): path: ") ds(pathname) da(": mode: ") di(mode) dw;

#if 1
    if (strstr(pathname, "/plugins/") != null
        // XXX w/o this I got this executed & my SIGCHLD handler re-directed //
        || strstr(pathname, "/gst-install-plugins-helper") != null) {
        dz df da("): skipping ") ds(pathname) dw;
        errno = ENOENT;
        return -1;
    }
#endif
    return access_next(pathname, mode);
}

int sigaction(int signum, const struct sigaction *act,
              struct sigaction *oldact)
{
    static int (*sigaction_next)(int, const struct sigaction *,
                                 struct sigaction *) = null;
    if (! sigaction_next)
        set_next(sigaction);

    dz da("*** ") df da("): signum: ") di(signum) dw;

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

#if 0
int open(const char * pathname, int flags, mode_t mode)
{
    static int (*open_next)(const char *, int, mode_t) = null;
    if (! open_next)
        set_next(open);

    dz df da("): path: ") ds(pathname) da(": flags: ") dx(flags) dw;

#if 0
    if (strstr(pathname, "/pipelight/") != null) {
        errno = ENOENT;
        return -1;
    }
#endif
    return open_next(pathname, flags, mode);
}
#endif

#if 0
FILE * fopen(const char * path, const char * mode)
{
    static FILE * (*fopen_next)(const char *, const char *) = null;
    if (! fopen_next)
        set_next(fopen);

    dz df da("): path: ") ds(path) da(": mode: ") ds(mode) dw;
    return fopen_next(path, mode);
}
#endif

// In webkit1 file
// webkitgtk-2.4.9/Source/WebKit/gtk/WebCoreSupport/ChromeClientGtk.cpp
//
// there is
/*
static void clearEverywhereInBackingStore(WebKitWebView* webView, cairo_t* cr)
{
    // The strategy here is to quickly draw white into this new canvas, so that
    // when a user quickly resizes the WebView in an environment that has opaque
    // resizing (like Gnome Shell), there are no drawing artifacts.
    if (!webView->priv->transparent) {
        cairo_set_source_rgb(cr, 1, 1, 1);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    } else
        cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
}*/
// so let's try wrapping cairo_paint() and see what happens -- the goal
// is to stop white flashing of the window before content is drawn...
// (fyi: this would not have worked if webkitgtk1 and cairo were in same lib.)

void cairo_paint(cairo_t * cr) { (void)cr; }

// ...hoo, the above seems to be enough, no white flashing anymore
// ...the side effects are to be seen, but nothing disturbing so far
// ...actually I've seen no difference...


// these macros added 2015-10-11 -- not used everywhere (yet...)
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
// end of 2015-10-11 //

#if 0
// ...later -- eikä ees käytössä...
#define gtk_bin_(fn, wid,...) \
    gtk_bin_ ## fn(GTK_BIN(wid), ##__VA_ARGS__)
#endif

#define set_one_property(object, name, state) \
    g_object_set (G_OBJECT(object), #name, state, null)

#define signal_connect(widget, signal, func, data) \
    g_signal_connect(widget, #signal, G_CALLBACK(func), data);
#define signal_connect_swapped(widget, signal, func, data) \
    g_signal_connect_swapped(widget, #signal, G_CALLBACK(func), data);


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
    WebKitWebBackForwardList * wkbfl
        = webkit_web_view_get_back_forward_list(G.web_view);
    webkit_web_back_forward_list_clear(wkbfl);
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


#if 1
static void destroy_toplevels_main_quit(void)
{
#if 0  // tis didnt wrok
    while (true) {
        GList * list = gtk_window_list_toplevels();
        if (!list)
            break;
        GList * first = g_list_first(list);
        if (!first)
            break;
        gtk_widget_(destroy, first->data);
    }
#endif
    gtk_main_quit();
}
#endif


// informational message only
static gboolean download_requested(WebKitWebView  * web_view,
                                   WebKitDownload * download,
                                   gpointer         user_data)
{
    uu(web_view); uu(user_data);
    WebKitNetworkRequest *
        request = webkit_download_get_network_request(download);
    const gchar * dest = webkit_network_request_get_uri(request);

    dz df dfc spc dss(dest) dw;

    return false;
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

static gboolean console_message(WebKitWebView * web_view,
                                gchar         * message,
                                gint            line,
                                gchar         * source_id,
                                gpointer        user_data)
{
    static uint32_t hash = 0;
    uu(web_view); uu(user_data);

    uint32_t h = djb2_hash(message);
    if (h != hash) {
        eprintf("%s: %s: %d: %s\n", __func__, source_id, line, message);
        hash = h;
    }
    return true; // do not call other (i.e. default) handler
}

#if 0
static void request_queued(SoupSession * session,
                           SoupMessage * message,
                           gpointer      user_data)
{
    uu(session); uu(user_data);
    //const char * method = message->method;

    //dz df dfc spc dss(method) dws;

#if 0
    if (method[0] != 'P' || method[1] != 'O' || method[2] != 'S'
        || method[3] != 'T' || method[4] != '\0')
        return;
#endif
    SoupURI * uri = soup_message_get_uri(message);

    uu(message); dz dss(uri->host) cmm dss(uri->path) dw;

    //eprintf("%s: POST %p path: %s\n", __func__, (void*)message, uri->path);
}
#endif

#if 0
static void frame_created(WebKitWebView  *web_view,
                          WebKitWebFrame *web_frame,
                          gpointer        user_data)
{
    eprintf("%s: %p %p\n", __func__, (void*)web_view, (void*)web_frame);
}
#endif

static gboolean navigation_policy_decision_requested(
    WebKitWebView             * web_view,
    WebKitWebFrame            * frame,
    WebKitNetworkRequest      * request,
    WebKitWebNavigationAction * navigation_action,
    WebKitWebPolicyDecision   * policy_decision,
    gpointer                    user_data)
{
    uu(web_view); uu(frame); uu(navigation_action);
    uu(policy_decision); uu(user_data);
    const char * uri = webkit_network_request_get_uri(request);

    int nomatch = regexec(&G.preg1, uri, 0, null, 0);

    eprintf("%s: %s (%d)\n", __func__, uri, nomatch);

    static gint64 pt = 0;
    gint64 ct = g_get_real_time();
    static uint32_t ph = 0;
    uint32_t ch = djb2_hash(uri);

    int rp = G.reload_painettu;
    G.reload_painettu = false;

    if (ct - pt < 2000000) {
        if (ch == ph) {
            pt = ct;
            webkit_web_policy_decision_ignore(policy_decision);
            return true;
        }
    }
    pt = ct;
    ph = ch;

    if (nomatch == 0) {
        if (rp || ! regexec(&G.preg2, uri, 0, null, 0)) {
            run_command("./mm-kysely", uri, null);
            if (! rp) {
                webkit_web_policy_decision_ignore(policy_decision);
                return true;
            }
        }
    }
    gtk_label_(set_text, G.addrlw, uri);
    return false;
}

// informational message only
static gboolean new_window_policy_decision_requested(
    WebKitWebView             * web_view,
    WebKitWebFrame            * frame,
    WebKitNetworkRequest      * request,
    WebKitWebNavigationAction * navigation_action,
    WebKitWebPolicyDecision   * policy_decision,
    gpointer                    user_data)
{
    uu(web_view); uu(frame); uu(navigation_action);
    uu(policy_decision); uu(user_data);

    const char * uri = webkit_network_request_get_uri(request);

    dz df dfc spc dss(uri) dw;

    return false;
}

#if 0
static void
resource_request_starting(WebKitWebView         *web_view,
                          WebKitWebFrame        *web_frame,
                          WebKitWebResource     *web_resource,
                          WebKitNetworkRequest  *request,
                          WebKitNetworkResponse *response,
                          gpointer               user_data)
{
    uu(web_view); uu(web_frame); uu(web_resource);
    uu(response); uu(user_data);
    //webkit_network_request_set_uri(request, "http://127.1:8080/");
#if 0
    SoupMessage * message = webkit_network_request_get_message(request);
    const char * method = message->method;
    if (method[0] != 'G' || method[1] != 'E' || method[2] != 'T' || method[3]
        != '\0')
        return;
#endif
    const char * uri = webkit_network_request_get_uri(request);

    dz df dfc spc dss(uri) dw;
}
#endif

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

static int is_geometry_string(char * str)
{
    while (*str != 'x' && *str != '\0') {
        if (*str < '0' || *str > '9')
            return false;
        str++;
    }
    if (*str++ == '\0')
        return false;

    for (int i = 0; i < 2; i++) {
        while (*str != '+' && *str != '-' && *str != '\0') {
            if (*str < '0' || *str > '9')
                return false;
            str++;
        }
        if (*str++ == '\0')
            return false;
    }
    while (*str >= '0' && *str <= '9')
        str++;
    //dz df da("): *str: ") di(*str) dw;

    return *str == '\0';
}

static void create_top_hbox(GtkBox * vbox)
{
    GtkBox * hbox = GTK_BOX(gtk_hbox_new(false, 0));

    GtkWidget *
    w = gtk_button_new_with_label("←");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, back, null);
    gtk_box_pack_start(hbox, w, false, true, 0);

    w = gtk_button_new_with_label("→");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, forward, null);
    gtk_box_pack_start(hbox, w, false, true, 0);

    w = gtk_button_new_with_label("↑");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, beginning, null);
    gtk_box_pack_start(hbox, w, false, true, 0);

    w = gtk_button_new_with_label("⟳");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, reload, null);
    gtk_box_pack_start(hbox, w, false, true, 0);

    gtk_box_pack_start(hbox, gtk_label_new("│"), false, true, 0);

    w = gtk_label_new(null);
    gtk_misc_(set_alignment, w, 0.0, 0.5);
    gtk_misc_(set_padding, w, 5, 1);
    gtk_box_pack_start(hbox, w, true, true, 0);
    G.addrlw = w;

    w = gtk_button_new_with_label("pysäytä lataus");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, stopp, null);
    gtk_box_pack_start(hbox, w, false,true, 0);

    w = gtk_button_new_with_label(" kattele ");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, vieww, null);
    gtk_box_pack_start(hbox, w, false,true, 0);

    w = gtk_button_new_with_label("uusix.");
    set_one_property(w, relief, GTK_RELIEF_NONE);
    signal_connect(w, clicked, restart, null);
    gtk_box_pack_start(hbox, w, false,true, 0);

    gtk_box_pack_start(vbox, GTK_WIDGET(hbox), false, true, 0);
}

int main(int argc, char* argv[])
{
    gtk_init(&argc, &argv);
    G.argv = argv; // needed for exevp() XXX gtk args are lost :(

    gtk_rc_parse("./gtk2-tummakahvi-muokattu.rc"); // if exists.
    doreg();
    webkit_set_cache_model(WEBKIT_CACHE_MODEL_DOCUMENT_VIEWER);
#if 0
    BB;
    SoupSession * session = webkit_get_default_session();
    signal_connect(session, request-queued, request_queued, null);
    BE;
#endif
    set_sigchld_handler();
    BB;
    GtkWidget * window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GdkColor color = { .red = 0, .green = 0, .blue = 0 };
    gtk_widget_modify_bg(window, GTK_STATE_NORMAL, &color);
    gtk_window_set_title(GTK_WINDOW(window), "mekkalamooses-selain");
    gtk_window_set_icon_from_file(GTK_WINDOW(window), "mm-ikoni.png", null);

    if (argc > 1 && is_geometry_string(argv[1]))
        gtk_window_parse_geometry(GTK_WINDOW(window), argv[1]);
    //  XXX no 'width-request' setting here...
    else {
        gtk_window_set_default_size(GTK_WINDOW(window), 1020, 720);
        // although attempted to keep window width fixed (by some mystic
        // widget adding order below) some pages made window resized anyhow
        // this property (hopefully) keeps it constant now
        set_one_property(window, width-request, 1020);
    }

    GtkWidget * scrollw = gtk_scrolled_window_new(null, null);

    // Create a browser instance
    WebKitWebView * web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    webkit_web_view_set_transparent (web_view, true);
#if 0 // webkitgtk 2 feature -- above transparent + cairo_paint() works...
    GdkRGBA rgba = { 0.0, 0.0, 0.0, 1.0 };
    webkit_web_view_set_background_color (web_view, &rgba);
#endif

    WebKitWebSettings * settings = webkit_web_view_get_settings(web_view);

    //g_object_set (G_OBJECT(settings), "user-agent", "jep", null);

    // in some systems this when "accelerated compositing" is enabled,
    // second connection to X server is made, and poll(2) from that fd
    // lags like hmm. ~400 ms before there is data. this makes this browser
    // unusable. until we know how to detect where this is usable,
    // this setting is outcommented.
    // set_one_property(settings, enable-accelerated-compositing, true);

     set_one_property(settings, enable-webgl, true); // <may have no effect
     set_one_property(settings, enable-private-browsing, true);
    // set_one_property(settings, enable-spatial-navigation, true);

     set_one_property(settings, enable-fullscreen, false);
     set_one_property(settings, enable-java-applet, false);
     set_one_property(settings, enable-plugins, false);

    GtkBox * vbox = GTK_BOX(gtk_vbox_new(false, 0));

    create_top_hbox(vbox);

    // note: attempted to move creation of web_view to separate function
    //   -- adding web_view to window later (probably after
    //      webkit_web_view_load_uri()) caused window to be resized...
    gtk_container_add(GTK_CONTAINER(scrollw), GTK_WIDGET(web_view));

    gtk_box_pack_start(vbox, scrollw, true, true, 0);

    //gtk_container_add();
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));

    signal_connect(window, delete-event, destroy_toplevels_main_quit, null);
    signal_connect(window, destroy, destroy_toplevels_main_quit, null);
    // välkkymisenestoa... tai siis oli joskus
    //signal_connect(window, motion-notify-event, gtk_true, null);
    //signal_connect(web_view, motion-notify-event, gtk_true, null);
#if 0
    signal_connect(web_view, close-web-view, close_cb, window);
#endif
    signal_connect(web_view, download-requested, download_requested, null);

    signal_connect(web_view, console-message, console_message, null);
    //signal_connect(b_view, create-plugin-widget, create_plugin_widget, null);

    //signal_connect(b_view, frame-created, frame_created, null);

    signal_connect(web_view, navigation-policy-decision-requested,
                   navigation_policy_decision_requested, null);

    signal_connect(web_view, new-window-policy-decision-requested,
                   new_window_policy_decision_requested, null);

    //signal_connect(web_view, resource-request-starting,
    //               resource_request_starting, null);

    WebKitWebFrame * wf = webkit_web_view_get_main_frame(web_view);
    signal_connect(wf, scrollbars-policy-changed, gtk_true, null);

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollw),
                                   GTK_POLICY_NEVER,
                                   GTK_POLICY_ALWAYS);

    eprintf("osoite: %s\n", ALKURI);
    webkit_web_view_load_uri(web_view, ALKURI);

    // make sure that when the browser area becomes visible, it will get mouse
    // and keyboard events
    gtk_widget_grab_focus(GTK_WIDGET(web_view));
    G.web_view = web_view;
    gtk_widget_show_all(window);
    BE;

    gtk_main();

#if 0 // tis didnt wrok (either)
    while (true) {
        GList * list = gtk_window_list_toplevels();
        if (!list)
            break;
        GList * first = g_list_first(list);
        if (!first)
            break;
        gtk_widget_(destroy, first->data);
    }
#endif
    // w/ this we have high chance to disconnect from (X) server
    for (int i = 3; i < 64; i++)
        close(i);

    while (1) {
        if (wait(null) < 0 && errno == ECHILD)
            break;
        sleep(1);
    }
    _exit(0);
    return 0;
}
