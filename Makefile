
# Last modified: Sat 13 Feb 2016 21:26:15 +0200 too

SHELL = /bin/sh

.PHONY: force

BIN = mm-selain mm-valinta mm-tausta mm-kysely mm-viesti

# set GITDIR if .git exists
ifeq ($(wildcard .git),)
	GITDIR =
else
	GITDIR = .git
endif

all: $(BIN)

mm-tausta: $(GITDIR)

mm-%:	mm-%.c
	sh $<

LHW=250

%.png:	%.tmp.png
	rm -f $@
	optipng --strip all -o9 -out $@ $<

%.tmp.png: %.pov
	povray -D0 +Q11 +A +UA -H$(LHW) -W$(LHW) +O$@ $<

lib.sh:
	test -n "$1" || exit 1 # internal shell lib; not to be made directly
	set -eu
	oo () { echo ongelma: "$@"; } >&2
	o () { echo ongelma: "$@"; exit 1; } >&2
	x () { echo '+' "$@" >&2; "$@"; }
	mdms () { for d; do test -d "$d" || x mkdir -p "$d"; done;  }
	datadir=${XDG_DATA_HOME:-$HOME/.local/share}
	td=$datadir/mekkalamooses
	dappdir=$datadir/applications
	iconbas=$datadir/icons/hicolor
	icondir=$datadir/icons/hicolor/128x128/apps
#	#eos
	exit 1 # not reached

install: $(BIN) force
	sed -n  -e '/[.]sh:$$/ s/^/#/' -e '/^#lib.sh/,/#.#eos/p' \
		-e '/^#$@.sh:/,/^#.#eos/p' Makefile | sh -s $(BIN)

install.sh:
	test -n "$1" || exit 1 # internal shell script; not to be made directly
	if test -d yle-dl
	then
		if test -d "$td"/yle-dl && test ! -h "$td"/yle-dl
		then
			oo $PWD/yle-dl on olemassa ja
			oo "$td"/yle-dl on olemassa
			o  mutta tämä jälkimmäinen ei ole symbolinen linkki
		fi
	fi
	x sh -n mekkalamooses
	pls='mm-lataaja.pl tv-yle.pl'
	for pl in $pls; do perl -c $pl; done
	mdms "$td" "$dappdir" "$icondir"
	x cp -f mekkalamooses $pls mm-ikoni.png mekkalamooses-128.png "$td"
	x cp -f gtk2-tummakahvi-muokattu.rc "$@" "$td"
	if test -d yle-dl
	then
		x rm -f "$td"/yle-dl
		x ln -s "`exec realpath -P yle-dl`" "$td"/yle-dl
	fi
	df=mekkalamooses.desktop; sed "s|=./|=$td/|" "$df" > "$dappdir"/"$df"
#	# I'd ln(1), but might be on different fs - and symlink needs more work
	x cp -f "$td"/mekkalamooses-128.png "$icondir"
	if command -v gtk-update-icon-cache >/dev/null
	then	x gtk-update-icon-cache --ignore-theme-index "$iconbas"
	fi
	echo
	echo Tiedostoja asennettiin hakemistoihin:
	echo ' ' $td
	echo ' ' $icondir
	echo ' ' $dappdir
	echo
	echo Sano \'make uninstall\' jos haluatkin niistä eroon.
#	#eos
	exit 1 # not reached

uninstall: force
	sed -n  -e '/[.]sh:$$/ s/^/#/' -e '/^#lib.sh/,/#.#eos/p' \
		-e '/^#$@.sh:/,/^#.#eos/p' Makefile | sh -s $(BIN)

uninstall.sh:
	test -n "$1" || exit 1 # internal shell script; not to be made directly
	x rm -rf "$td"
	x rm -rf "$icondir"/mekkalamooses*
	x rm -rf "$dappdir"/mekkalamooses*
	if command -v gtk-update-icon-cache >/dev/null
	then	x gtk-update-icon-cache --ignore-theme-index "$iconbas"
	fi
#	#eos
	exit 1 # not reached

# archive the files in current git master
txz:	force
	./targit.pl
targit: txz

clean: force
	rm -f testaa-regex logo.png *~

distclean: clean
	rm -f $(BIN)

# empty the (unneeded) default suffix list with an empty .SUFFIXES target
.SUFFIXES:

# Local variables:
# mode: makefile
# End:
