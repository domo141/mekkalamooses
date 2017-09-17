#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf; trg=${0##*''/}; trg=${trg%.c}.so; test ! -e "$trg" || rm "$trg"
 WARN="-Wall -Wstrict-prototypes -Winit-self -Wformat=2" # -pedantic
 WARN="$WARN -Wcast-align -Wpointer-arith " # -Wfloat-equal #-Werror
 WARN="$WARN -Wextra -Wwrite-strings -Wcast-qual -Wshadow" # -Wconversion
 WARN="$WARN -Wmissing-include-dirs -Wundef -Wbad-function-cast -Wlogical-op"
 WARN="$WARN -Waggregate-return -Wold-style-definition"
 WARN="$WARN -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls"
 WARN="$WARN -Wnested-externs -Winline -Wvla -Woverlength-strings -Wpadded"
 case ${1-} in '') set x -O2; shift; esac
 #case ${1-} in '') set x -ggdb; shift; esac
 set -x
 exec ${CC:-gcc} -std=c99 -shared -fPIC -o "$trg" "$0" $WARN "$@" -ldl
 exit $?
 */
#endif

/*
 * $ ldpreload_fake_isatty.c $
 *
 * Author: Tomi Ollila -- too ät iki piste fi
 *
 *      Copyright (c) 2013 Tomi Ollila
 *          All rights reserved
 *
 * Created: Wed 27 Feb 2013 22:54:11 EET too
 * Prev modified: Mon 04 Mar 2013 21:38:13 EET too (in tx11ssh.c)
 * Last modified: Sun 17 Sep 2017 16:58:55 +0300 too
 */

#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/uio.h>
#include <dlfcn.h>

#include <unistd.h>
#include <termios.h>
#include <stdio.h>

#define null ((void*)0)

// clang -dM -E - </dev/null | grep __GNUC__  outputs '#define __GNUC__ 4'
#if (__GNUC__ >= 4)
#define ATTRIBUTE(x) __attribute__(x)
#else
#define ATTRIBUTE(x)
#endif

static void * dlsym_next(const char * symbol)
{
    void * sym = dlsym(RTLD_NEXT, symbol);
    char * str = dlerror();

    if (str != null) {
        fprintf(stderr, "finding symbol '%s' failed: %s", symbol, str);
        exit(1);
    }
    return sym;
}
#define set_next(name) *(void**)(&name##_next) = dlsym_next(#name)

int isatty(int fd)
{
    static int (*isatty_next)(int) = null;
    if (! isatty_next)
        set_next(isatty);

    if (fd <= 2) return 1;
    return isatty_next(fd);
}

int tcgetattr(int fd, struct termios *termios_p)
{
    static int (*tcgetattr_next)(int, struct termios *) = null;
    if (! tcgetattr_next)
        set_next(tcgetattr);

    int e = tcgetattr_next(fd, termios_p);
    if (e && fd <= 2) {
        cfmakeraw(termios_p);
        return 0;
    }
    return e;
}

int tcsetattr(int fd, int optional_actions, const struct termios *termios_p)
{
    static int (*tcsetattr_next)(int, int, const struct termios *) = null;

    if (! tcsetattr_next)
        set_next(tcsetattr);

    int e = tcsetattr_next(fd, optional_actions, termios_p);
    if (e && fd <= 2)
        return 0;
    return e;
}
