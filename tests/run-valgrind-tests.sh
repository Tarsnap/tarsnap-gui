#!/bin/sh

set -e

# Command-line
VALGRINDS=""
VALGRINDS="app-cmdline core cli consolelog persistent task taskmanager"

# Gui
VALGRINDS="${VALGRINDS} lib-widgets"
VALGRINDS="${VALGRINDS} customfilesystemmodel small-widgets setupwizard"
VALGRINDS="${VALGRINDS} backuptabwidget settingswidget"
VALGRINDS="${VALGRINDS} archivestabwidget app-setup"
VALGRINDS="${VALGRINDS} translations helpwidget"
VALGRINDS="${VALGRINDS} jobstabwidget mainwindow"

check_dir() {
	dirname=$1

	cd $D
	make -j3 test_valgrind

	# Extra check for "still reachable", which doesn't produce an error code
	printf "\n"
	reachable=$(grep "still reachable:" valgrind-full.log)
	set +e
	reachable_prob=$(echo "${reachable}" | grep -v "0 blocks")
	set -e
	if [ -n "${reachable_prob}" ]; then
		reachable_short=$(echo "${reachable_prob}" | cut -d " " -f 2-)
		printf "${dirname}:\t${reachable_short}\n"
		exit 1
	fi
	cd ..
}

# Run tests
for D in $VALGRINDS; do
	check_dir $D
done
