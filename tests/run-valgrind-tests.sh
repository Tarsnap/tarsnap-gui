#!/bin/sh

set -e

# Command-line
VALGRINDS="cli consolelog persistent task taskmanager"

# Gui
VALGRINDS="${VALGRINDS} customfilesystemmodel small-widgets setupwizard"
VALGRINDS="${VALGRINDS} backuptabwidget settingswidget jobstabwidget"
VALGRINDS="${VALGRINDS} archivestabwidget"

# Run tests
for D in $VALGRINDS; do
	cd $D
	make test_valgrind
	cd ..
done

# Extra check for "still reachable", which doesn't produce an error code
printf "\n"
for D in $VALGRINDS; do
	reachable=$(grep "still reachable:" $D/valgrind-full.log)
	set +e
	reachable_prob=$(echo "${reachable}" | grep -v "0 blocks")
	set -e
	if [ -n "${reachable_prob}" ]; then
		reachable_short=$(echo "${reachable_prob}" | cut -d " " -f 2-)
		printf "$D:\t${reachable_short}\n"
	fi
done
