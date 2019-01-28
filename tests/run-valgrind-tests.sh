#!/bin/sh

set -e

# Command-line
VALGRINDS="cli consolelog persistent task taskmanager"

# Gui
VALGRINDS="${VALGRINDS} small-widgets customfilesystemmodel setupwizard"
VALGRINDS="${VALGRINDS} backuptabwidget"

for D in $VALGRINDS; do
	cd $D
	qmake > /dev/null
	nice make -j3 > /dev/null
	make test_valgrind
	cd ..
done
