#!/bin/sh

set -e

# Command-line
DIRS_C=""
DIRS_C="${DIRS_C} core consolelog"
DIRS_C="${DIRS_C} task taskmanager persistent"
DIRS_C="${DIRS_C} app-cmdline cli"

# Gui
DIRS_G="${DIRS_G} lib-widgets"
DIRS_G="${DIRS_G} customfilesystemmodel small-widgets setupwizard"
DIRS_G="${DIRS_G} backuptabwidget settingswidget"
DIRS_G="${DIRS_G} archivestabwidget app-setup"
DIRS_G="${DIRS_G} translations helpwidget"
DIRS_G="${DIRS_G} jobstabwidget mainwindow"

if [ "${VALGRIND_ONLY_CONSOLE:-0}" -gt "0" ] ; then
	DIRS="${DIRS_C}"
else
	DIRS="${DIRS_C} ${DIRS_G}"
fi

check_dir() {
	dirname=$1

	cd "$D"
	make -j3 test_valgrind

	# Extra check for "still reachable", which doesn't produce an error code
	printf "\n"
	reachable=$(grep "still reachable:" valgrind-full.log)
	set +e
	reachable_prob=$(echo "${reachable}" | grep -v "0 blocks")
	set -e
	if [ -n "${reachable_prob}" ]; then
		reachable_short=$(echo "${reachable_prob}" | cut -d " " -f 2-)
		printf "%s:\t%s\n" "${dirname}" "${reachable_short}"
		exit 1
	fi
	cd ..
}

# Run tests
for D in $DIRS; do
	check_dir "$D"
done
